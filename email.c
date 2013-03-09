#include "email.h"

#include <stdlib.h>
#include <string.h>

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

void email_set_sender(struct email* email, char* from)
{
  email->from = malloc(strlen(from+1));
  strcpy(email->from, from);
}

int email_add_recipient(struct email* email, char* to)
{
  for (int i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i] == NULL) {
      char* copy = malloc(strlen(to+1));
      strcpy(copy, to);
      email->to[i] = copy;
      return 1;
    }
  }
  return 0;
}