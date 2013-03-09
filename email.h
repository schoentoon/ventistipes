#ifndef _EMAIL_H
#define _EMAIL_H

#define MAX_RECIPIENTS 100

struct email {
  int ehlo;
  char* from;
  char* to[MAX_RECIPIENTS];
  char* subject;
  char* data;
  enum email_fillin_mode {
    HEADERS = 0,
    DATA_HEADERS = 1,
    DATA = 2
  } mode;
};

/** Create a new email structure and return this
 */
struct email* new_email();

/** Clean up this email structure
 */
void delete_email(struct email* email);

/** Set our sender
 */
int email_set_sender(struct email* email, char* from);

/** Add a recipient to our email structure
 * @return 1 if added correctly, 0 if not added
 * this will only occur when there are more than
 * MAX_RECIPIENTS recipients
 */
int email_add_recipient(struct email* email, char* to);

/** Simple check if the email structure has the
 * from and to field filled in.
 * @return If it has both the from and at least one
 * recipient it'll return 1, else it'll return 0
 */
int email_has_recipients(struct email* email);

int email_set_subject(struct email* email, char* line);

int email_append_data(struct email* email, char* data);

#ifdef DEV
void print_emails(struct email* email);
#endif

#endif //_EMAIL_H