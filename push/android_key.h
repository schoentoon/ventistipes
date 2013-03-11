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