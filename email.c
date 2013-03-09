#include "email.h"

#include "string_helpers.h"

#include <stdlib.h>
#include <string.h>

struct email* new_email()
{
  struct email* email = malloc(sizeof(struct email));
  email->ehlo = 0;
  email->mode = HEADERS;
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

int email_set_sender(struct email* email, char* from)
{
  if (email->mode != HEADERS)
    return 0;
  email->from = stripOutEmailAddress(from);
  return 1;
}

int email_add_recipient(struct email* email, char* to)
{
  if (email->mode != HEADERS)
    return 0;
  for (int i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i] == NULL) {
      email->to[i] = stripOutEmailAddress(to);
      return 1;
    }
  }
  return 0;
}

int email_has_recipients(struct email* email)
{
  return (email->from && email->to[0]) ? 1 : 0;
}

int email_append_data(struct email* email, char* data)
{
  if (email->mode != DATA)
    return 0;
  if (email->data) {
    size_t length = strlen(email->data) + strlen(data) + 2; /* One for \n and one for \0 */
    email->data = realloc(email->data, length);
    strncat(email->data, "\n", length);
    strncat(email->data, data, length);
  } else {
    email->data = malloc(strlen(data+1));
    strcpy(email->data, data);
  }
  return 0;
}

#ifdef DEV

#include <stdio.h>

void print_emails(struct email* email)
{
  printf("From: %s\n", email->from);
  printf("To: ");
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
  printf("Data: ");
  if (email->data)
    printf("%s\n", email->data);
  else
    printf("NULL\n");
}
#endif