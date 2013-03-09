#ifndef _SMTP_H
#define _SMTP_H

#include <event.h>

#define SERVER_PORT 2525

int initMailListener(struct event_base* event_base);

void closeMailListener();

#endif //_SMTP_H