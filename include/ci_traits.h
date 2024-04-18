#ifndef DP_CI_TRAITS
#define DP_CI_TRAITS

/*
*	The classic example of a case-insensitive traits class for string, string_view, and others.
*	Has been done before, but I needed one and it's better to have a common impl than reinvent the wheel
*/

#include <string>
#include <cctype>
#include <cwctype>

//Embarcadero don't define the standard testing macros...
#if defined(__cpp_lib_string_view) || defined(DP_CBUILDER11)
#include <string_view>
#endif

#if defined(DB_CBUILDER11) || __cplusplus >= 201103L || defined(_MSC_VER)
#define DP_CONSTEXPR constexpr
#else
#define DP_CONSTEXPR
#endif



namespace dp {

	//At present we only support char and wchar_t; as unicode is its own kettle of fish.
	template<typename CharT>
	class ci_traits : public std::char_traits<CharT> {

		static char upper(char in) {
			return std::toupper(static_cast<unsigned char>(in));
		}
		static wchar_t upper(wchar_t in) {
			return std::towupper(in);
		}

	public:
		static bool eq(CharT lhs, CharT rhs) {
			return upper(lhs) == upper(rhs);
		}
		static bool lt(CharT lhs, CharT rhs) {
			return upper(lhs) < upper(rhs);
		}
		static int compare(const CharT* lhs, const CharT* rhs, std::size_t n) {
			while (n-- != 0) {
				if (upper(*lhs) < upper(*rhs)) return -1;
				if (upper(*lhs) > upper(*rhs)) return 1;
				++lhs;
				++rhs;
			}
			return 0;
		}
		static const CharT* find(const CharT* str, std::size_t n, CharT ch) {
			const CharT upper_ch = upper(ch);
			while (n-- != 0) {
				if (upper(*str) == upper_ch) return str;
				++str;
			}
			return NULL;
		}
	};

	//Arguably unnecessary, but there to give us an easier time when dealing with traits_cast.
	//As we no longer have to remember the C++98 > > space rule with templates
	typedef ci_traits<char>		ci_char_traits;
	typedef ci_traits<wchar_t>	ci_wchar_traits;

	//Because we're in C++98 we don't have std::string_view as a standardised tool for converting string types around.
	//As such, we need to template the class this cast function uses as its stringlike.
	//This way it'll work with std::string, std::string_view, and a custom string_view which follows the pattern.
	template<typename Dest_Traits, typename CharT, typename Src_Traits, template<typename, typename> class StrT>
	DP_CONSTEXPR StrT<CharT, Dest_Traits> traits_cast(StrT<CharT, Src_Traits> in) {
		return StrT<CharT, Dest_Traits>(in.data(), in.size());
	}

	//We include the usual convenience typedefs
	typedef std::basic_string<char, ci_traits<char> >		ci_string;
	typedef std::basic_string<wchar_t, ci_traits<wchar_t> >	ci_wstring;


	//If we have std::string_view
#if defined(__cpp_lib_string_view) || defined(DP_CBUILDER11)
	typedef std::basic_string_view<char, ci_traits<char> >			ci_string_view;
	typedef std::basic_string_view<wchar_t, ci_traits<wchar_t> >	ci_wstring_view;
#else

	//And because I have written a C++98 string_view (but don't want to make it a hard dependency) we allow for typedefs of that too
	//Yes this may cause some confusion, but this is a C++Builder lib so ABI across updates is a non-issue
	template<typename CharT, typename Traits>
	class basic_string_view;

	typedef basic_string_view<char, ci_traits<char> >		ci_string_view;
	typedef basic_string_view<wchar_t, ci_traits<wchar_t> >	ci_wstring_view;

#endif


}


#undef DP_CONSTEXPR

#endif