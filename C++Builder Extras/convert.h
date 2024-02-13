#ifndef DP_CONVERT_INTERFACE
#define DP_CONVERT_INTERFACE

/*
* Because the template mechanics we do with convert_to differs so widely between compiler versions, it is cleaner to separate these implementations
* Into their own headers rather than have one monster header with multiple preprocessor blocks.
* This header serves as an "interface", including the right one based on compiler versions and standards
*/

#ifdef __BORLANDC__
#include "bits/borland_version_defs.h"
#endif

//C++17
#if defined(DP_CBUILDER11) || __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#include "bits/convert_cpp17.h"
//Borland
#elif defined(DP_CBUILDER5)
#include "bits/convert_cpp98_borland.h"
//Everyone else
#else
#include "bits/convert_cpp98.h"
#endif


#endif
