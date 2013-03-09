#ifndef _EMAIL_H
#define _EMAIL_H

#define MAX_RECIPIENTS 100

struct email {
  int ehlo;
  char* from;
  char* to[MAX_RECIPIENTS];
};

/** Create a new email structure and return this
 */
struct email* new_email();

/** Clean up this email structure
 */
void delete_email(struct email* email);

/** Set our sender
 */
void email_set_sender(struct email* email, char* from);

/** Add a recipient to our email structure
 * @return 1 if added correctly, 0 if not added
 * this will only occur when there are more than
 * MAX_RECIPIENTS recipients
 */
int email_add_recipient(struct email* email, char* to);

#endif //_EMAIL_H