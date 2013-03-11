#ifndef _PUSH_H
#define _PUSH_H

#include "email.h"

#include <event2/dns.h>

struct evdns_base* dns;

struct push_info {
  char* subject;
  char* data;
  struct event_base* event_base;
};

void launch_push_queries(char* address, void* context, struct email* email);

#endif //_PUSH_H