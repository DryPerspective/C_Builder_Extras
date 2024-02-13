#ifndef DP_BORLANDDEFS
#define DP_BORLANDDEFS


//C++Builder does not support __cplusplus to determine language standard
//Instead we have to do a hacky workaround based on the version number of the *IDE*
//I don't make the rules.
#ifdef __BORLANDC__
#if __BORLANDC__ < 0x552 //Original value is = 0x551 for C++Builder 5
#define DP_CBUILDER5
#elif __BORLANDC__ < 0x0740
#define DP_CBUILDER5
#define DP_CBUILDER10
#elif __BORLANDC__ < 0x770
#define DP_CBUILDER5
#define DP_CBUILDER10
#define DP_CBUILDER11
#endif
#endif

#endif