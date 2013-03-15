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

#include "smtp.h"
#include "postgres.h"

#include <event.h>
#include <signal.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

static const struct option g_LongOpts[] = {
  { "help",     no_argument,       0, 'h' },
  { "debug",    no_argument,       0, 'D' },
  { "port",     required_argument, 0, 'p' },
  { 0, 0, 0, 0 }
};

struct event_base* event_base = NULL;

void onSignal(int signal)
{
  closeMailListener();
  event_base_free(event_base);
  exit(0);
}

void usage()
{
  printf("USAGE: ventistipes [options]\n");
  printf("-h, --help\tShow this help.\n");
  printf("-p, --port\tListen on this port, defaults to 2525.\n");
  printf("-D, --debug\tKeep open for debugging.\n");
}

int main(int argc, char **argv)
{
  int iArg, iOptIndex, tmp = -1;
  unsigned short listen_port = 2525;
  char debug = 0;
#ifdef DEV
  debug = 1;
#endif //DEV
  while ((iArg = getopt_long(argc, argv, "hDp:", g_LongOpts, &iOptIndex)) != -1) {
    switch (iArg) {
      case 'D':
        debug = 1;
        break;
      case 'p':
        tmp = strtol(optarg, NULL, 10);
        if ((errno == ERANGE || (tmp == LONG_MAX || tmp == LONG_MIN)) || (errno != 0 && tmp == 0) || tmp < 0 || tmp > 65535) {
          fprintf(stderr, "--port requires a valid port.\n");
          return 1;
        }
        listen_port = (unsigned short) tmp;
        break;
      default:
      case 'h':
        usage();
        return 0;
    }
  }
  if ((debug == 0 && fork() == 0) || debug) {
    event_base = event_base_new();
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    initMailListener(event_base, listen_port);
    initDatabasePool(event_base);
    signal(SIGTERM, onSignal);
    signal(SIGSTOP, onSignal);
    event_base_dispatch(event_base); /* We probably won't go further than this line.. */
    closeMailListener();
    event_base_free(event_base);
  }
  return 0;
}