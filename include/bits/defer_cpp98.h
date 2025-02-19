#ifndef DP_CPP98_DEFER
#define DP_CPP98_DEFER

#include <exception>

#include "bits/macros.h"

/*
*  A DEFER tool, also known as a SCOPE_EXIT tool. If a function should contain the expression DEFER(foo) then foo will automatically be executed when the scope exits.
*  Note that due to the limitations of C++98, this tool is not aware of any function-local data and can only call stateless code or code which manipulates some global state (ugh).
*  Still, good for when using what is essentially a C library as you can automate cleanup.
*
*  Full documentation here: https://github.com/DryPerspective/C_Builder_Extras/wiki/Defer
*/

#define DEFER(ARGS) struct { \
                        struct Defer_Impl{ \
                            Defer_Impl() {}; \
                            ~Defer_Impl() { ARGS ;} \
                        }  impl; \
                    } DP_UNIQUE_NAME(Defer_Struct);




#endif