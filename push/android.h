#ifndef _ANDROID_PUSH_H
#define _ANDROID_PUSH_H

#include "push.h"

#include <event2/event.h>

void android_push(struct push_info* push_info, char* push_id, struct event_base* event_base);

#endif //_ANDROID_PUSH_H