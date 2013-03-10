#include "email.h"

#include "safefree.h"
#include "string_helpers.h"

#include <stdlib.h>
#include <string.h>

struct email* new_email()
{
  struct email* email = malloc(sizeof(struct email));
  email->ehlo = 0;
  email->from = NULL;
  int i;
  for (i = 0; i < MAX_RECIPIENTS; i++)
    email->to[i] = NULL;
  email->subject = NULL;
  email->data= NULL;
  email->mode = HEADERS;
  return email;
}

void delete_email(struct email* email)
{
  if (email) {
    if (email->from)
      SAFEFREE(email->from);
    int i;
    for (i = 0; i < MAX_RECIPIENTS; i++) {
      if (email->to[i])
        SAFEFREE(email->to[i]);
      else
        break;
    }
    if (email->subject)
      SAFEFREE(email->subject);
    if (email->data)
      SAFEFREE(email->data);
    SAFEFREE(email);
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
  int i;
  for (i = 0; i < MAX_RECIPIENTS; i++) {
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

int email_set_subject(struct email* email, char* line)
{
  size_t line_length = strlen(line);
  if (line_length < 8)
    return 0;
  email->subject = malloc(line_length - 8); /* Subject: is 9 characters (with the space) but we need \0 too you know */
  int i;
  for (i = 0; i < line_length - 8; i++)
    email->subject[i] = line[i+9];
  return 1;
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
    int i;
    for (i = 0; i < MAX_RECIPIENTS; i++) {
      if (email->to[i]) {
        if (i == 0)
          printf("%s", email->to[i]);
        else
          printf(", %s", email->to[i]);
      } else
        break;
    }
    printf("\n");
  }
  printf("Subject: ");
  if (email->subject)
    printf("%s\n", email->subject);
  else
    printf("No subject\n");
  printf("Data: ");
  if (email->data)
    printf("%s\n", email->data);
  else
    printf("NULL\n");
}
#endif