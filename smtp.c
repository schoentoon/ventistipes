#include "smtp.h"

#include "email.h"
#include "macros.h"
#include "safefree.h"
#include "push/push.h"
#include "string_helpers.h"

#include <event2/listener.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct evconnlistener *listener;

static void smtp_listener_cb(struct evconnlistener *listener
                            ,evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data);

int initMailListener(struct event_base* event_base)
{
  struct sockaddr_in sin;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(SERVER_PORT);

  listener = evconnlistener_new_bind(event_base, smtp_listener_cb, (void*) event_base
                                    ,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1
                                    ,(struct sockaddr*) &sin, sizeof(sin));

  if (!listener) {
    fprintf(stderr, "Could not create a listener!\n");
    return 1;
  }
  return 0;
}

void closeMailListener()
{
  evconnlistener_free(listener);
}

/** All the internal smtp stuff is below here, hidden from the outside world really.
 */

#include "postgres.h"

static char* _220_HELLO         = "220 Hello\n";
static char* _250_OK            = "250 Ok\n";
static char* _354_GO_AHEAD      = "354 Go ahead\n";
static char* _221_BYE           = "221 Bye\n";
static char* _502_NOT_SUPPORTED = "502 Command not implemented\n";
static char* _503_BAD_SEQUENCE  = "503 Bad sequence of commands\n";
static char* _550_NOT_ALLOWED   = "550 Not allowed\n";

static void check_email_from_callback(PGresult* res, void* email, char* query);
static void check_email_to_callback(PGresult* res, void* email, char* query);
char* create_check_email_from_query(char* email);
char* create_check_email_to_query(char* email);

static void smtp_conn_readcb(struct bufferevent *bev, void* args)
{
  struct evbuffer* buffer = bufferevent_get_input(bev);
  struct email* email = (struct email*) args;
  size_t len;
  char* line = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF);
  while (line) {
    if (email->mode != DATA && email->mode != DATA_LAST_LINE_EMPTY) {
      if (string_equals(line, "QUIT"))
        bufferevent_write(bev, _221_BYE, strlen(_221_BYE));
      else if (string_equals(line, "RSET"))
        bufferevent_write(bev, _502_NOT_SUPPORTED, strlen(_502_NOT_SUPPORTED));
    }
    switch (email->mode) {
    case HEADERS:
      if (len >= 4 && !email->ehlo) { /* Could be an EHLO or HELO */
        if (string_startsWith(line, "EHLO") || string_startsWith(line, "HELO")) {
          email->ehlo = 1;
          bufferevent_write(bev, _250_OK, strlen(_250_OK));
        }
      } else if (email->ehlo) {
        if (string_startsWith(line, "MAIL FROM:<")) {
          email_set_sender(email, line);
          if (email->from)
            databaseQuery(create_check_email_from_query(email->from), check_email_from_callback, email);
        } else if (string_startsWith(line, "RCPT TO:<")) {
          if (email_add_recipient(email, line))
            databaseQuery(create_check_email_to_query(email_get_last_recipient(email)), check_email_to_callback, email);
        } else if (string_equals(line, "DATA")) {
          if (email_has_recipients(email)) {
            bufferevent_write(bev, _354_GO_AHEAD, strlen(_354_GO_AHEAD));
            email->mode = DATA_HEADERS;
          } else
            bufferevent_write(bev, _503_BAD_SEQUENCE, strlen(_503_BAD_SEQUENCE));
        }
      }
      break;
    case DATA_HEADERS:
      if (strlen(line) == 0)
        email->mode = DATA;
      else if (string_startsWith(line, "Subject: "))
        email_set_subject(email, line);
      break;
    case DATA:
      if (strlen(line) > 0)
        email_append_data(email, line);
      else
        email->mode = DATA_LAST_LINE_EMPTY;
      break;
    case DATA_LAST_LINE_EMPTY:
      if (string_equals(line, ".")) {
        bufferevent_write(bev, _250_OK, strlen(_250_OK));
        email->mode = DATA_DONE;
        email_for_each_recipient(email, bufferevent_get_base(bev), launch_push_queries);
      } else
        email->mode = DATA;
      break;
    default:
      break;
    }
#ifdef DEV
    printf("I got the following line: %s\n", line);
#endif
    free(line);
    line = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF);
  }
}

static void smtp_conn_eventcb(struct bufferevent *bev, short events, void* args)
{
#ifdef DEV
  if (events & BEV_EVENT_EOF)
    printf("Connection closed.\n");
#endif
  bufferevent_free(bev);
  struct email* email = (struct email*) args;
#ifdef DEV
  print_emails(email);
#endif
  delete_email(email);
}

static void smtp_listener_cb(struct evconnlistener *listener, evutil_socket_t fd
                            ,struct sockaddr *sa, int socklen, void *user_data)
{
  struct event_base* base = user_data;
  struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    fprintf(stderr, "Error constructing bufferevent!");
    event_base_loopbreak(base);
    return;
  }
  struct email* email = new_email();
  email->bev = bev;
  bufferevent_setcb(bev, smtp_conn_readcb, NULL, smtp_conn_eventcb, email);
  bufferevent_enable(bev, EV_WRITE|EV_READ);
  bufferevent_write(bev, _220_HELLO, strlen(_220_HELLO));
}

static void check_email_from_callback(PGresult* res, void* context, char* query)
{
  struct email* email = (struct email*) context;
  struct bufferevent* bev = email->bev;
#ifdef DEV
  printf("Query %s returned with %d rows.\n", query, PQntuples(res));
#endif
  if (PQntuples(res) > 0)
    bufferevent_write(bev, _250_OK, strlen(_250_OK));
  else {
    bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
    SAFEFREE(email->from);
  }
  SAFEFREE(query);
}

static void check_email_to_callback(PGresult* res, void* context, char* query)
{
  struct email* email = (struct email*) context;
  struct bufferevent* bev = email->bev;
#ifdef DEV
  printf("Query %s returned with %d rows.\n", query, PQntuples(res));
#endif
  if (PQntuples(res) > 0)
    bufferevent_write(bev, _250_OK, strlen(_250_OK));
  else {
    bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
    size_t len = strlen(query);
    size_t email_len = len - 38 -2;
    char addr[email_len];
    int i;
    for (i = 0; i < email_len; i++)
      addr[i] = query[i + 38];
    addr[email_len] = '\0';
    char* addr_p = malloc(email_len);
    email_remove_email_from_recipients(email, strcpy(addr_p, addr));
    SAFEFREE(addr_p);
  }
  SAFEFREE(query);
}

char* create_check_email_from_query(char* email)
{ /* SELECT 1 FROM allowed_in_mail WHERE email = 'email@addre.ss'; */
  size_t email_len = strlen(email); /* I am aware that I should escape this right here.. */
  size_t output_len = email_len + 45 + 2 + 1; /* Sadly that requires a PGconn* object, which I don't have here. */
  char buffer[output_len]; //TODO escape the query parameters properly
  snprintf(buffer, sizeof(buffer), "SELECT 1 FROM allowed_in_mail WHERE email = '%s';", email);
  char* output = malloc(output_len);
  return strcpy(output, buffer);
}

char* create_check_email_to_query(char* email)
{ /* SELECT 1 FROM push_ids WHERE email = 'email@addre.ss'; */
  size_t email_len = strlen(email);
  size_t output_len = email_len + 38 + 2 + 1;
  char buffer[output_len];
  snprintf(buffer, sizeof(buffer), "SELECT 1 FROM push_ids WHERE email = '%s';", email);
  char* output = malloc(output_len);
  return strcpy(output, buffer);
}