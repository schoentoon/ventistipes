#include "email.h"

#include "string_helpers.h"

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
  email->from = stripOutEmailAddress(from);
}

int email_add_recipient(struct email* email, char* to)
{
  for (int i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i] == NULL) {
      email->to[i] = stripOutEmailAddress(to);
      return 1;
    }
  }
  return 0;
}

#ifdef DEV

#include <stdio.h>

void print_emails(struct email* email)
{
  printf("From: %s\n", email->from);
  printf("To ");
  if (!email->to[0])
    printf("nobody.\n");
  else {
    for (int i = 0; i < MAX_RECIPIENTS; i++) {
      if (email->to[i]) {
        if (i == 0)
          printf("%s", email->to[i]);
        else
          printf(", %s", email->to[i]);
      }
    }
    printf("\n");
  }
}
#endif