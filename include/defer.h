#ifndef DP_DEFER_INTERFACE
#define DP_DEFER_INTERFACE

#ifdef __BORLANDC__
#include "bits/borland_version_defs.h"
#endif

/*
*  The changes to the mechanics of DEFER are significant between versions. As such we separate the implementations into separate headers.
*/
//C++17
#if defined(DP_CBUILDER11) || __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#include "bits/defer_cpp17.h"
//C++98
#else
#include "bits/defer_cpp98.h"
#endif

#endif