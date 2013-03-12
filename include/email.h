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

#ifndef _EMAIL_H
#define _EMAIL_H

#define MAX_RECIPIENTS 100

struct email {
  int ehlo;
  char* from;
  char* to[MAX_RECIPIENTS];
  char* subject;
  char* data;
  struct bufferevent *bev;
  enum email_fillin_mode {
    HEADERS = 0,
    DATA_HEADERS = 1,
    DATA = 2,
    DATA_DONE = 3
  } mode;
};

/** Create a new email structure and return this
 * @return a new email structure
 */
struct email* new_email();

/** Clean up this email structure
 * @param email The email structure we want to clean up
 */
void delete_email(struct email* email);

/** Set our sender
 * @param email The email structure we want to have this sender
 * @param from The raw line we want to parse the email address out of
 * @return 1 in case we set it correctly, else we'll return 0
 */
int email_set_sender(struct email* email, char* from);

/** Add a recipient to our email structure
 * @param email The email structure we want to have this recipient
 * @param to The raw line we want to parse the email address out of
 * @return 1 if added correctly, 0 if not added
 * this will only occur when there are more than
 * MAX_RECIPIENTS recipients or if the email isn't valid
 */
int email_add_recipient(struct email* email, char* to);

/** Simple check if the email structure has the
 * from and to field filled in.
 * @param email The email structure we want to check for recipients
 * @return If it has both the from and at least one
 * recipient it'll return 1, else it'll return 0
 */
int email_has_recipients(struct email* email);

/** Get the last recipient
 * @return Returns the last recipient in the to array
 * returns NULL if there are no recipients
 */
char *email_get_last_recipient(struct email* email);

/** Remove an email address from the mail structure
 * @param email The structure to remove an address from
 * @param addr THe address to remove
 * @return 1 in case there was something deleted, else it'll return 0
 */
int email_remove_email_from_recipients(struct email* email, char* addr);

/** Parse the subject and set it to the email structure
 * @param email The email structure we want to have the subject
 * @param line The raw smtp line to parse for the subject
 * @return 1 in case there is a subject set, else it'll return 0
 */
int email_set_subject(struct email* email, char* line);

/** Execute this function pointer for each recipient
 * @param email The email structure to use for the recipients
 * @param context Void pointer to pass to the function pointer
 * @param execute The function pointer to execute for each recipient
 */
void email_for_each_recipient(struct email* email, void* context, void (*execute)(char* address, void* context, struct email* email));

/** Parse a raw line to 'append' it to the data field in email
 * @param email The email structure we want to have this data
 * @param data The data we want to append
 * @return 1 in case we're in the right mode, else it'll return 0
 */
int email_append_data(struct email* email, char* data);

#ifdef DEV
void print_emails(struct email* email);
#endif

#endif //_EMAIL_H