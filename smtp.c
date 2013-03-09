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

static char* _220_HELLO = "220 Hello\n";
static char* _250_OK    = "250 Ok\n";

static void smtp_conn_readcb(struct bufferevent *bev, void* user_data)
{
  struct evbuffer* buffer = bufferevent_get_input(bev);
  struct email* email = (struct email*) user_data;
  size_t len;
  char* line = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF);
  if (line) {
    if (len >= 4 && !email->ehlo) { /* Could be an EHLO or HELO */
      if (startsWith(line, "EHLO") || startsWith(line, "HELO")) {
        email->ehlo = 1;
        bufferevent_write(bev, _250_OK, strlen(_250_OK));
      }
    } else if (email->ehlo) {
      if (startsWith(line, "MAIL FROM:<")) {
        email->from = malloc(strlen (line)+1);
        strcpy(email->from, line);
        bufferevent_write(bev, _250_OK, strlen(_250_OK));
      } else if (startsWith(line, "RCPT TO:<")) {
        char* copy = malloc(strlen(line+1));
        strcpy(copy, line);
        add_recipient(email, copy);
        bufferevent_write(bev, _250_OK, strlen(_250_OK));
      }
    }
    printf("I got the following line: %s\n", line);
    free(line);
  }
}

static void smtp_conn_writecb(struct bufferevent *bev, void *user_data)
{
}

static void smtp_conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
  if (events & BEV_EVENT_EOF)
    printf("Connection closed.\n");
  bufferevent_free(bev);
  struct email* email = (struct email*) user_data;
  delete_email(email);
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
  bufferevent_setcb(bev, smtp_conn_readcb, smtp_conn_writecb, smtp_conn_eventcb, email);
  bufferevent_enable(bev, EV_WRITE|EV_READ);
  bufferevent_write(bev, _220_HELLO, strlen(_220_HELLO));
}