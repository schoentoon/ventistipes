#include "email.h"

#include <stdlib.h>

struct email* new_email()
{
  struct email* email = malloc(sizeof(struct email));
  email->ehlo = 0;
  return email;
}

void delete_email(struct email* email)
{
  if (email->from)
    free(email->from);
  for (int i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i])
      free(email->to[i]);
    else
      break;
  }
}

int add_recipient(struct email* email, char* to)
{
  for (int i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i] == NULL) {
      email->to[i] = to;
      return 1;
    }
  }
  return 0;
}