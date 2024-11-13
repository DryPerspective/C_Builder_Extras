#ifndef DP_BORLANDDEFS
#define DP_BORLANDDEFS


//C++Builder does not support __cplusplus to determine language standard
//Instead we have to do a hacky workaround based on the version number of the *IDE*
//I don't make the rules.
#ifdef __BORLANDC__
#if __BORLANDC__ >= 0x551 
#define DP_CBUILDER5
#endif
#if __BORLANDC__ >= 0x730
#define DP_CBUILDER10
#endif
#if __BORLANDC__ >= 0x760
#define DP_CBUILDER11
#endif
#endif


#endif