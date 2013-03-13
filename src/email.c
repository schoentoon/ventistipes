/*  Ventistipes
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
  email->bev = NULL;
  email->mode = HEADERS;
  return email;
}

void delete_email(struct email* email)
{
  if (email) {
    SAFEFREE(email->from);
    int i;
    for (i = 0; i < MAX_RECIPIENTS; i++) {
      if (email->to[i])
        SAFEFREE(email->to[i]);
      else
        break;
    }
    SAFEFREE(email->subject);
    SAFEFREE(email->data);
    SAFEFREE(email);
  }
}

int email_set_sender(struct email* email, char* from)
{
  if (email->mode != HEADERS)
    return 0;
  if (valididateEmailAddress(from)) {
    email->from = from;
    return 1;
  }
  return 0;
}

int email_add_recipient(struct email* email, char* to)
{
  if (email->mode != HEADERS)
    return 0;
  if (valididateEmailAddress(to)) {
    int i;
    for (i = 0; i < MAX_RECIPIENTS; i++) {
      if (email->to[i] == NULL) {
        email->to[i] = to;
        return 1;
      }
    }
  }
  return 0;
}

int email_has_recipients(struct email* email)
{
  return (email->from && email->to[0]) ? 1 : 0;
}

char *email_get_last_recipient(struct email* email)
{
  int i;
  char* output = NULL;
  for (i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i])
      output = email->to[i];
    else
      break;
  }
  return output;
}

int email_remove_email_from_recipients(struct email* email, char* addr)
{
  int i;
  int output = 0;
  for (i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i] && string_equals(email->to[i], addr)) {
      SAFEFREE(email->to[i]);
      int j;
      for (j = i; j < MAX_RECIPIENTS; j++)
        email->to[j] = email->to[j+1];
      output = 1;
    }
  }
  return output;
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

void email_for_each_recipient(struct email* email, void* context, void (*execute)(char* address, void* context, struct email* email))
{
  int i;
  for (i = 0; i < MAX_RECIPIENTS; i++) {
    if (email->to[i])
      execute(email->to[i], context, email);
  }
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
  return 1;
}

#ifdef DEV

#include <stdio.h>

void print_emails(struct email* email)
{
  printf("From: %s\n", (email->from ? email->from : "nobody."));
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