#ifndef _ANDROID_PUSH_H
#define _ANDROID_PUSH_H

#include <event2/event.h>

void android_push(void* context, char* push_id, struct event_base* event_base);

#endif //_ANDROID_PUSH_H