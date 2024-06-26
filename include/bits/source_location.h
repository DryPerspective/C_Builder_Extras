#ifndef DP_SOURCE_LOC
#define DP_SOURCE_LOC

/*
*  C++20's std::source_location relies on compiler magic to work; so unfortunately we can't fully emulate it
*  However, we can kind of sort of emulate it with a bit of macro magic.
*  And it's a shame that we have to use macros, but needs must. 
*/

#include "bits/borland_compat_typedefs.h"
#include "bits/macros.h"

namespace dp {

	struct source_location {
		const char* function;
		const char* file;
		int line;
	};

}

#define DP_SOURCE_LOC_HERE dp::source_location{DP_FUNC, __FILE__, __LINE__}







#endif