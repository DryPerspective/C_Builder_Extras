#ifndef DP_SOURCE_LOC
#define DP_SOURCE_LOC

/*
*  C++20's std::source_location relies on compiler magic to work; so unfortunately we can't fully emulate it
*  However, we can kind of sort of emulate it with a bit of macro magic.
*  And it's a shame that we have to use macros, but needs must. 
*/

#include "bits/macros.h"

#if defined(DB_CBUILDER11) || __cplusplus >= 201103L || defined(_MSC_VER)
#define DP_CONSTEXPR constexpr
#else
#define DP_CONSTEXPR
#endif

namespace dp {

	struct source_location {
		const char* function;
		const char* file;
		int line;

		//Even though this is essentially an aggregate we need a constructor
		//To allow certain compiler builtins to behave themselves
		DP_CONSTEXPR source_location(const char* in_func, const char* in_file, int in_line) : function(in_func), file(in_file), line(in_line) {}
	};

}

#define DP_SOURCE_LOCATION_THIS_FUNCTION dp::source_location(DP_FUNC, __FILE__, __LINE__)

/*
*  There is no easy way to mimic the functionality of std::source_location::current properly
*  Even "modern" C++Builder is built on a version of CLang which cannot support it
*  As such, we need to do some ugly preprocessor hackery
*/

#ifdef _MSC_VER
#define DP_SOURCE_LOCATION_CURRENT dp::source_location{__builtin_FUNCTION(), __FILE__, __LINE__}
#elif defined(__clang__) && __clang_major__ >= 9
#define DP_SOURCE_LOCATION_CURRENT dp::source_location{__builtin_FUNCTION(), __FILE__, __LINE__}
//Regrettably before CLang 9 there was no way to get a function in this context which would do the right thing
#elif defined(__clang__)
#define DP_SOURCE_LOCATION_CURRENT dp::source_location{"", __FILE__, __LINE__}
#elif defined(__GNUC__)
#define DP_SOURCE_LOCATION_CURRENT dp::source_location{__builtin_FUNCTION(), __FILE__, __LINE__}
//If on old Borland (C++98)
#elif defined(__BORLANDC__)
#define DP_SOURCE_LOCATION_CURRENT DP_SOURCE_LOCATION_THIS_FUNCTION
#endif







#undef DP_CONSTEXPR

#endif