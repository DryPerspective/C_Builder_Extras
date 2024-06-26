#ifndef DP_CONTRACTS_INTERFACE
#define DP_CONTRACTS_INTERFACE

#include "bits/borland_version_defs.h"

#if defined(DP_CBUILDER11) || __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#include "bits/contracts_cpp17.h"
#else
#include "bits/contracts_cpp98.h"
#endif

#endif