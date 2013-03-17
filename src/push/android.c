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

#include "push/android.h"

#include "config/gcm_key.h"

#ifndef API_KEY
#  error "Please define your API_KEY for gcm, you can register for one over here: https://code.google.com/apis/console"
#  error "If you decide to not use gcm just define it to something empty please."
#endif

#include "safefree.h"

#define BUFFER_SIZE 1024*4

#include <openssl/ssl.h>

#include <event2/dns.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

static void android_readcb(struct bufferevent *bev, void* args)
{
  struct evbuffer* buffer = bufferevent_get_input(bev);
  char* line = evbuffer_readln(buffer, NULL, EVBUFFER_EOL_CRLF);
  while (line) {
#ifdef DEV
    fprintf(stderr, "gcm: %s\n", line);
#endif //DEV
    free(line);
    line = evbuffer_readln(buffer, NULL, EVBUFFER_EOL_CRLF);
  } /* Don't bother with this output, somehow we can read more than the http headers anyway */
}

static void android_eventcb(struct bufferevent *bev, short events, void* args)
{
  if (events & BEV_EVENT_CONNECTED)
    return;
#ifdef DEV
  if (events & BEV_EVENT_EOF)
    printf("Connection closed.\n");
#endif
  bufferevent_free(bev);
}

char* stripData(char* data, size_t to_length)
{
  size_t len = strlen(data);
  size_t output_len = (len < to_length) ? len : to_length;
  char* output = malloc(output_len);
  int i;
  for (i = 0; i < output_len; i++)
    output[i] = data[i];
  output[output_len] = '\0';
  return output;
}

static char* _POST = "POST /gcm/send HTTP/1.0\r\n";
static char* _HOST = "Host: android.googleapis.com\r\n";
static char* _USER_AGENT = "User-Agent: Ventistipes\r\n";
static char* _CONTENT_TYPE = "Content-Type: application/json\r\n";
static char* _AUTH = "Authorization:key=";
static char* _CRLF = "\r\n";
static char* _CONTENT_LENGTH = "Content-Length: ";

void android_push(struct push_info* push_info, char* push_id, struct event_base* event_base)
{
  SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv3_method());
  SSL *ssl = SSL_new(ssl_ctx);
  struct bufferevent *bev = bufferevent_openssl_socket_new(event_base, -1, ssl
                                                          ,BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, android_readcb, NULL, android_eventcb, NULL);
  bufferevent_socket_connect_hostname(bev, dns, AF_INET, "android.googleapis.com", 443);
  bufferevent_enable(bev, EV_READ|EV_WRITE);
  bufferevent_write(bev, _POST, strlen(_POST));
  bufferevent_write(bev, _HOST, strlen(_HOST));
  bufferevent_write(bev, _USER_AGENT, strlen(_USER_AGENT));
  bufferevent_write(bev, _CONTENT_TYPE, strlen(_CONTENT_TYPE));
  bufferevent_write(bev, _AUTH, strlen(_AUTH));
  bufferevent_write(bev, API_KEY, strlen(API_KEY));
  bufferevent_write(bev, _CRLF, strlen(_CRLF));
  bufferevent_write(bev, _CONTENT_LENGTH, strlen(_CONTENT_LENGTH));
  char data[BUFFER_SIZE];
  sprintf(data, "");
  if (push_info->subject) {
    char* tmp = stripData(push_info->subject, BUFFER_SIZE-12);
    sprintf(data, "%s\"subject\":\"%s\"", data, tmp);
    SAFEFREE(tmp);
  }
  if (push_info->sender) {
    char* tmp = stripData(push_info->sender, BUFFER_SIZE-strlen(data)-11-((push_info->subject) ? 1 : 0));
    sprintf(data, "%s%s\"sender\":\"%s\"", data, ((push_info->subject) ? "," : ""), tmp);
    SAFEFREE(tmp);
  }
  if (push_info->data) {
    char* tmp = stripData(push_info->data, BUFFER_SIZE-strlen(data)-9-((push_info->subject || push_info->sender) ? 1 : 0));
    sprintf(data, "%s%s\"data\":\"%s\"", data, ((push_info->subject || push_info->sender) ? "," : ""), tmp);
    SAFEFREE(tmp);
  }
  char buffer[1024*6];
  sprintf(buffer, "{\"registration_ids\":[\"%s\"],\"data\":{%s}}", push_id, data);
#ifdef DEV
  printf("\n\nJSON: %s\n\n", buffer);
#endif //DEV
  char length_buffer[4];
  snprintf(length_buffer, sizeof(length_buffer), "%zd", strlen(buffer));
  bufferevent_write(bev, length_buffer, strlen(length_buffer));
  bufferevent_write(bev, _CRLF, strlen(_CRLF));
  bufferevent_write(bev, _CRLF, strlen(_CRLF));
  bufferevent_write(bev, buffer, strlen(buffer));
  bufferevent_write(bev, _CRLF, strlen(_CRLF));
}