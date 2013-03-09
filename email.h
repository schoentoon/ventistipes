#ifndef _EMAIL_H
#define _EMAIL_H

#define MAX_RECIPIENTS 100

struct email {
  int ehlo;
  char* from;
  char* to[MAX_RECIPIENTS];
};

struct email* new_email();

void delete_email(struct email* email);

int add_recipient(struct email* email, char* to);

#endif //_EMAIL_H