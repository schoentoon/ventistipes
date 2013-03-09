#include "smtp.h"

#include "email.h"
#include "macros.h"
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

static char* _220_HELLO         = "220 Hello\n";
static char* _250_OK            = "250 Ok\n";
static char* _354_GO_AHEAD      = "354 Go ahead\n";
static char* _221_BYE           = "221 Bye\n";
static char* _502_NOT_SUPPORTED = "502 Command not implemented\n";
static char* _503_BAD_SEQUENCE  = "503 Bad sequence of commands\n";

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
          bufferevent_write(bev, _250_OK, strlen(_250_OK));
        } else if (string_startsWith(line, "RCPT TO:<")) {
          email_add_recipient(email, line);
          bufferevent_write(bev, _250_OK, strlen(_250_OK));
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
  free(args);
}

static void smtp_listener_cb(struct evconnlistener *listener, evutil_socket_t fd
                            ,struct sockaddr *sa, int socklen, void *user_data)
{
  struct event_base* base = user_data;
  struct bufferevent* bev;
  bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    fprintf(stderr, "Error constructing bufferevent!");
    event_base_loopbreak(base);
    return;
  }
  struct email* email = new_email();
  bufferevent_setcb(bev, smtp_conn_readcb, NULL, smtp_conn_eventcb, email);
  bufferevent_enable(bev, EV_WRITE|EV_READ);
  bufferevent_write(bev, _220_HELLO, strlen(_220_HELLO));
}