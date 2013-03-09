#ifndef _STRING_HELPERS_H
#define _STRING_HELPERS_H

/** Simple check if str1 starts with str2
 * @param str1 Input string
 * @param str2 String line has to start with
 * @return It'll return 1 in case line does start
 * with start, else it'll return 0
 */
int string_startsWith(char* str1, char* str2);

/** Simple check if str1 is equal to str2
 * @param str1 Input string 1
 * @param str2 Input string 2
 * @return It'll return 1 in case they're equal,
 * else it'll return 0
 */
int string_equals(char* str1, char* str2);

/** Return just the email address
 * @param line Raw line from the SMTP client
 * @return A new string with just the email address
 * @note line is not freed.
 */
char* stripOutEmailAddress(char* line);

#endif //_STRING_HELPERS_H