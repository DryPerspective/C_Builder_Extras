#ifndef DP_BORLAND_COMPAT_TYPES
#define DP_BORLAND_COMPAT_TYPES

/*
*  The C++Builder library does not play nicely with C++ standard library types. For easy portability, we define some typedefs
*  for common types such that on Borland you get VCL types and everywhere else you get normal human types.
*  This helps to remove some preprocessor hackery from the main library files.
* 
*  This obviously has the downside of very slightly different semantics when this library is compiled in C++Builder.
*  This may appear to be a compatibility concern, however I balance it like this:
*	* If you are in C++Builder, you will most likely be using the VCL as STL types don't work with it.
*   * C++Builder offers almost no ABI compatility to non-Embarcadero code, and itself breaks ABI on every update, so it is unlikely
*	  that you would be able to get code compiled anywhere else to work on C++Builder in the first place
*   * C++Builder already breaks cross-compatibility by removing, tweaking, and renaming common functionality.
*   * C++Builder fails to define standard versioning macros, so semantics will already be subtly different regardless.
*   * The paricular audience for this library does things a particular way and it is better to match their existing style 
*     in C++Builder than it is to create a mismatch of style and force it upon them.
*/


#ifdef __BORLANDC__
//BORLAND includes
#include <vcl.h>

#else
//STL includes
#include <string>
#include <stdexcept>

#endif

//We use typedefs over aliases for C++98 compatibility
namespace dp {
namespace compat {

#ifdef __BORLANDC__
	typedef AnsiString			string;
	typedef Exception			runtime_error;

#else
	typedef std::string			string;
	typedef std::runtime_error	runtime_error;

#endif

}
}



#endif