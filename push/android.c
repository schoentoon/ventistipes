#include "android.h"

void android_push(void* context, char* push_id, struct event_base* event_base)
{
  printf("Push_id: %s\n", push_id);
}