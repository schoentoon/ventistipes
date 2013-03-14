/*  Ventistipes
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smtp.h"

#include "email.h"
#include "macros.h"
#include "safefree.h"
#include "push/push.h"
#include "string_helpers.h"

#include <event2/listener.h>

#include <ctype.h>
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

int isEmailCharacters(char c) {
  return (!(isalnum(c) || c == '>' || c == '<' || c == '.' || c == '_' || c == ' ' || c == ':' || c == '@' || c == '-'));
}

static void smtp_conn_readcb(struct bufferevent *bev, void* args)
{
  struct evbuffer* buffer = bufferevent_get_input(bev);
  struct email* email = (struct email*) args;
  size_t len;
  char* line = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF);
  while (line) {
    if (email->mode != DATA) {
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
        if (!forEachCharacter(line, isEmailCharacters)) {
          if (string_startsWith(line, "MAIL FROM:<")) {
            char* addr = stripOutEmailAddress(line);
            if (addr) {
              char* query = create_check_email_from_query(addr);
              if (query)
                databaseQuery(query, check_email_from_callback, email);
              else
                bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
              SAFEFREE(addr);
            } else
              bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
          } else if (string_startsWith(line, "RCPT TO:<")) {
            char* addr = stripOutEmailAddress(line);
            if (addr) {
              char* query = create_check_email_to_query(addr);
              if (query)
                databaseQuery(query, check_email_to_callback, email);
              else
                bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
              SAFEFREE(addr);
            } else
              bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
          } else if (string_equals(line, "DATA")) {
            if (email_has_recipients(email)) {
              bufferevent_write(bev, _354_GO_AHEAD, strlen(_354_GO_AHEAD));
              email->mode = DATA_HEADERS;
            } else
              bufferevent_write(bev, _503_BAD_SEQUENCE, strlen(_503_BAD_SEQUENCE));
          } else
            bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
        } else
          bufferevent_write(bev, _502_NOT_SUPPORTED, strlen(_502_NOT_SUPPORTED));
      }
      break;
    case DATA_HEADERS:
      if (strlen(line) == 0)
        email->mode = DATA;
      else if (!string_contains(line, ':')) {
        email->mode = DATA;
        email_append_data(email, line);
      } else if (string_startsWith(line, "Subject: "))
        email_set_subject(email, line);
      break;
    case DATA:
      if (strlen(line) > 0) {
        if (string_equals(line, ".")) {
          bufferevent_write(bev, _250_OK, strlen(_250_OK));
          email->mode = DATA_DONE;
          email_for_each_recipient(email, bufferevent_get_base(bev), launch_push_queries);
        } else
          email_append_data(email, line);
      }
      break;
    default:
      break;
    }
#ifdef DEV
    printf("I got the following line: %s\n", line);
#endif
    SAFEFREE(line);
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

#define FROM_QUERY_START 45
#define FROM_QUERY_END 10

static void check_email_from_callback(PGresult* res, void* context, char* query)
{
  struct email* email = (struct email*) context;
  struct bufferevent* bev = email->bev;
#ifdef DEV
  printf("Query %s returned with %d rows.\n", query, PQntuples(res));
#endif
  if (PQntuples(res) == 1) {
    size_t len = strlen(query);
    size_t email_len = len - FROM_QUERY_START - FROM_QUERY_END;
    char*  addr = malloc(email_len);
    int i;
    for (i = 0; i <= email_len; i++)
      addr[i] = query[i + FROM_QUERY_START];
    addr[email_len] = '\0';
    if (email_set_sender(email, addr))
      bufferevent_write(bev, _250_OK, strlen(_250_OK));
    else {
      SAFEFREE(addr);
      bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
    }
  } else
    bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
  SAFEFREE(query);
}

#define TO_QUERY_START 38
#define TO_QUERY_END 10

static void check_email_to_callback(PGresult* res, void* context, char* query)
{
  struct email* email = (struct email*) context;
  struct bufferevent* bev = email->bev;
#ifdef DEV
  printf("Query %s returned with %d rows.\n", query, PQntuples(res));
#endif
  if (PQntuples(res) == 1) {
    size_t len = strlen(query);
    size_t email_len = len - TO_QUERY_START - TO_QUERY_END;
    char* addr = malloc(email_len);
    int i;
    for (i = 0; i <= email_len; i++)
      addr[i] = query[i + TO_QUERY_START];
    addr[email_len] = '\0';
    if (email_add_recipient(email, addr))
      bufferevent_write(bev, _250_OK, strlen(_250_OK));
    else {
      SAFEFREE(addr);
      bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
    }
  } else
    bufferevent_write(bev, _550_NOT_ALLOWED, strlen(_550_NOT_ALLOWED));
  SAFEFREE(query);
}

char* create_check_email_from_query(char* email)
{ /* SELECT 1 FROM allowed_in_mail WHERE email = 'email@addre.ss'; */
  if (!email || !valididateEmailAddress(email))
    return NULL;
  size_t email_len = strlen(email);
  size_t output_len = email_len + FROM_QUERY_START + FROM_QUERY_END + 1;
  char buffer[output_len];
  snprintf(buffer, sizeof(buffer), "SELECT 1 FROM allowed_in_mail WHERE email = '%s' LIMIT 1;", email);
  char* output = malloc(output_len);
  return strcpy(output, buffer);
}

char* create_check_email_to_query(char* email)
{ /* SELECT 1 FROM push_ids WHERE email = 'email@addre.ss'; */
  if (!email || !valididateEmailAddress(email))
    return NULL;
  size_t email_len = strlen(email);
  size_t output_len = email_len + TO_QUERY_START + TO_QUERY_END + 1;
  char buffer[output_len];
  snprintf(buffer, sizeof(buffer), "SELECT 1 FROM push_ids WHERE email = '%s' LIMIT 1;", email);
  char* output = malloc(output_len);
  return strcpy(output, buffer);
}
