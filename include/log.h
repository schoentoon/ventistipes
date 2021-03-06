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

#ifndef _LOG_H
#define _LOG_H

/** Print to stderr and write to log
 */
#define ERROR(...) \
        write_to_log(__VA_ARGS__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");

/** Write stuff to our log in case logging is enabled
 * usage is the same as printf(const char*, ...);
 */
void write_to_log(const char* format, ...);

/** Configure the log file
 * @param filename Write a log to this file, file will be created if it doesn't exist.
 */
void set_logfile(char* filename);

#endif //_LOG_H