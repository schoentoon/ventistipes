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
#include <openssl/ssl.h>
#include <openssl/err.h>

int main(int argc, char **argv)
{
  struct event_base* event_base = event_base_new();
  SSL_library_init();
  ERR_load_crypto_strings();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  initMailListener(event_base);
  initDatabasePool(event_base);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  closeMailListener();
  event_base_free(event_base);
  return 0;
}