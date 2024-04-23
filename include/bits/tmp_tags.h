#ifndef TMP_TAGS
#define TMP_TAGS

/*
*  A collection of tags for template tag dispatch.
*  Note that this header is intended to be used in several environments and across several C++ standards so there's some preprocessor hackery going on
*/

#include <string>
#include <climits>

#ifdef __BORLANDC__
#include "bits/borland_version_defs.h"
#endif

//If C++11. MSVC define __cplusplus to its 1998 value by default, and Borland define it to 1 because Borland are special snowflakes
#if defined(DP_CBUILDER11) || __cplusplus >= 201103L || defined(_MSC_VER) 
#define DP_CONSTEXPR constexpr
#else
#define DP_CONSTEXPR
#endif

#ifdef DP_CBUILDER5
#include <vcl.h>
#endif

//Embarcadero don't define the C++ library versioning macros.
//Don't ask me why
//So we can't just #ifdef __cpp_lib_string_view. We have to hack our own way
#if defined(__cpp_lib_string_view) || defined(DP_CBUILDER11)
#define DP_STRING_VIEW_AVAILABLE
#endif

#ifdef DP_STRING_VIEW_AVAILABLE
#include <string_view>
#endif



namespace dp {

	/*
	* The core of our TMP bag of tricks. This lightweight struct allows us to represent a concrete type without needing to construct it.
	*/
	template<typename T>
	struct instance_of {
		typedef T type;
	};

	/*
	* Tag classes. When the same behaviour is required in the same "family" of types, these allow us to categorise a generic type
	* e.g. a function with an overload which accepts a tag_signed_int, and a call which uses instance_of<int> will match
	* Don't allow overlap. E.g. one overload which accepts a tag_std_string and one which accepts tag_any_string will be ambiguous
	*/
	struct tag_signed_int {
		DP_CONSTEXPR tag_signed_int() {}
		DP_CONSTEXPR tag_signed_int(instance_of<short>) {}
		DP_CONSTEXPR tag_signed_int(instance_of<int>) {}
		DP_CONSTEXPR tag_signed_int(instance_of<long>) {}
#ifdef LLONG_MAX
		DP_CONSTEXPR tag_signed_int(instance_of<long long>) {}
#endif
	};

	struct tag_unsigned_int {
		DP_CONSTEXPR tag_unsigned_int() {}
		DP_CONSTEXPR tag_unsigned_int(instance_of<unsigned short>) {}
		DP_CONSTEXPR tag_unsigned_int(instance_of<unsigned int>) {}
		DP_CONSTEXPR tag_unsigned_int(instance_of<unsigned long>) {}
#ifdef ULLONG_MAX
		DP_CONSTEXPR tag_unsigned_int(instance_of<unsigned long long>) {}
#endif
	};

	struct tag_any_int {
		DP_CONSTEXPR tag_any_int() {}
		DP_CONSTEXPR tag_any_int(instance_of<short>) {}
		DP_CONSTEXPR tag_any_int(instance_of<int>) {}
		DP_CONSTEXPR tag_any_int(instance_of<long>) {}
		DP_CONSTEXPR tag_any_int(instance_of<unsigned short>) {}
		DP_CONSTEXPR tag_any_int(instance_of<unsigned int>) {}
		DP_CONSTEXPR tag_any_int(instance_of<unsigned long>) {}
#ifdef LLONG_MAX
		DP_CONSTEXPR tag_any_int(instance_of<long long>) {}
		DP_CONSTEXPR tag_any_int(instance_of<unsigned long long>) {}
#endif

	};

	struct tag_floating_point {
		DP_CONSTEXPR tag_floating_point() {}
		DP_CONSTEXPR tag_floating_point(instance_of<float>) {}
		DP_CONSTEXPR tag_floating_point(instance_of<double>) {}
		DP_CONSTEXPR tag_floating_point(instance_of<long double>) {}
	};

	struct tag_std_string {
		DP_CONSTEXPR tag_std_string() {}
		DP_CONSTEXPR tag_std_string(instance_of<std::string>) {}
		DP_CONSTEXPR tag_std_string(instance_of<std::wstring>) {}
#ifdef DP_STRING_VIEW_AVAILABLE
		DP_CONSTEXPR tag_std_string(instance_of<std::string_view>) {}
		DP_CONSTEXPR tag_std_string(instance_of<std::wstring_view>) {}
#endif
	};

#ifdef DP_CBUILDER5
	struct tag_delphi_string {
		DP_CONSTEXPR tag_delphi_string() {}
		DP_CONSTEXPR tag_delphi_string(instance_of<AnsiString>) {}
#ifdef DP_CBUILDER10
		DP_CONSTEXPR tag_delphi_string(instance_of<UnicodeString>) {}
#endif
	};
#endif

	struct tag_any_string {
		DP_CONSTEXPR tag_any_string() {}
		DP_CONSTEXPR tag_any_string(instance_of<std::string>) {}
		DP_CONSTEXPR tag_any_string(instance_of<std::wstring>) {}
#ifdef DP_STRING_VIEW_AVAILABLE
		DP_CONSTEXPR tag_any_string(instance_of<std::string_view>) {}
		DP_CONSTEXPR tag_any_string(instance_of<std::wstring_view>) {}
#endif
#ifdef DP_CBUILDER5
		DP_CONSTEXPR tag_any_string(instance_of<AnsiString>) {}
#ifdef DP_CBUILDER10
		DP_CONSTEXPR tag_any_string(instance_of<UnicodeString>) {}
#endif
#endif
	};

	struct tag_char {
		DP_CONSTEXPR tag_char() {}
		DP_CONSTEXPR tag_char(instance_of<char>) {}
		DP_CONSTEXPR tag_char(instance_of<unsigned char>) {}
		DP_CONSTEXPR tag_char(instance_of<signed char>) {}
		DP_CONSTEXPR tag_char(instance_of<wchar_t>) {}
	};

	struct tag_narrow_string {
		DP_CONSTEXPR tag_narrow_string() {}
		DP_CONSTEXPR tag_narrow_string(instance_of<std::string>) {}
#ifdef DP_STRING_VIEW_AVAILABLE
		DP_CONSTEXPR tag_narrow_string(instance_of<std::string_view>) {}
#endif
#ifdef DP_CBUILDER5
		DP_CONSTEXPR tag_narrow_string(instance_of<AnsiString>) {}
#endif
	};

	struct tag_wide_string {
		DP_CONSTEXPR tag_wide_string() {}
		DP_CONSTEXPR tag_wide_string(instance_of<std::wstring>) {}
#ifdef DP_STRING_VIEW_AVAILABLE
		DP_CONSTEXPR tag_wide_string(instance_of<std::wstring_view>) {}
#endif
#ifdef DP_CBUILDER10
		DP_CONSTEXPR tag_wide_string(instance_of<UnicodeString>) {}
#endif
	};


}

#undef DP_CONSTEXPR
#undef DP_STRING_VIEW_AVAILABLE


#endif
