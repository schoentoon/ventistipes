//#define CONNINFO "<Your postgres connect info here>"

#ifdef DEV // This preprocessor is just here so you can develop with at least some sort of default auth
#  ifndef CONNINFO
#    define CONNINFO "user=ventstipes dbname=ventstipes"
#  endif //CONNINFO
#endif //_DEV