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

//#define API_KEY "<Your API key here>"

#ifdef DEV // This preprocessor is just here so you can develop without a gcm key
#  ifndef API_KEY
#    define API_KEY ""
#  endif //API_KEY
#endif //_DEV

/* Please fill in your gcm API key and uncomment the #define
 * you can register for a gcm API key over here https://code.google.com/apis/console
 * in services toggle Google Cloud Messaging for Android.
 * Then over at API Access generate a new Server key (be smart and restrict it to your ip)
 * and copy the API key into the define
 */