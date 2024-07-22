#ifndef DP_CPP17_CONVERT_TO
#define DP_CPP17_CONVERT_TO

#if defined(DP_CPP98_CONVERT_TO) || defined(DP_CPP98_CONVERT_BORLAND)
#error "Using multiple different versions of convert_to is not supported"
#endif

#include <charconv>
#include <type_traits>
#include <string_view>
#include <cstdlib>
#include <cwchar>
#include <cmath>
#include <limits>
#include <array>

#include "bits/borland_version_defs.h"
#include "bits/tmp_tags.h"

#include "contracts.h"

/*
*  A general purpose type conversion tool. To allow for easy conversion between common types.
*  We do a bit of TMP to get it working. In particular, we use tag dispatch as it lends itself well to being able to
*  manipulate overload sets on entire families of types in a much more user-friendly way than most ways of manipulating function templates via SFINAE
*  or endless chains of if-constexpr
*/

/*
* While this library is intended to work alongside the Borland types and remove some of the clunkiness therein,
* at present it can only work in a C++ environment running C++17.
*/
#if defined(CBUILDER5) && !defined(CBUILDER11)
#error "This header requires C++17 to function"
#endif

#ifdef __cpp_lib_constexpr_charconv
#define CHARCONV_CONSTEXPR constexpr
#else
#define CHARCONV_CONSTEXPR
#endif

namespace dp {

	//Forward dec
	template<typename To, typename From>
	constexpr To convert_to(const From&);


	//Implementation tools. Do not touch these in user code.
	namespace conv_impl {

		// TMP to determine if from_chars exists for floating point overloads, as this was (and seemingly still is) a common
		// implementation quirk for C++17 compilers.
		template<typename T, typename = void>
		struct from_chars_exists_impl : std::false_type {};

		template<typename T>
		struct from_chars_exists_impl <T, std::void_t<decltype(std::from_chars(std::declval<const char*>(), std::declval<const char*>(), std::declval<T&>()))>> : std::true_type {};

		template<typename T>
		static constexpr inline bool from_chars_exists = from_chars_exists_impl<T>::value;


		template<typename>
		constexpr inline bool dependent_false{ false };

		template<typename T, typename = void>
		struct conv_helper {
			static_assert(dependent_false<T>, "Attempting to convert to invalid type");
		};

		//INTEGER
		template<typename T>
		struct conv_helper<T, std::enable_if_t<std::is_integral_v<T>>> {

			//INT
			template<typename IntT>
			constexpr static inline T get(IntT in, tag_any_int) {
				//Will warn on narrowing.
				//Not my fault if you want a narrower type. I can't invent new data
				return static_cast<IntT>(in);
			}
			//FLOAT
			template<typename FloatT>
			constexpr static inline T get(FloatT in, tag_floating_point) {
				return static_cast<FloatT>(in);
			}
			//BOOL
			constexpr static inline T get(bool in, instance_of<bool>) {
				return in ? 1 : 0;
			}
			//STRINGS
			CHARCONV_CONSTEXPR static inline T get(std::string_view in, tag_narrow_string) {
				//Fortunately integral from_chars is supported on all major platforms
				//Check for hex formatting
				int base{ 10 };
				if (in.length() >= 2 && in[0] == '0' && in[1] == 'x') {
					base = 16;
					in.remove_prefix(2);
				}
				T out{};
				auto result{ std::from_chars(in.data(), in.data() + in.length(), out, base) };
				CONTRACT_ASSERT(result.ec == std::errc{}, std::string{ "Error converting to integer type: " } + (result.ec == std::errc::invalid_argument ? " invalid input" : " out of range"));
				return out;
				
			}
			//WIDESTRINGS
			static inline T get(std::wstring_view in, tag_wide_string) {
				int base{ 10 };
				if (in.length() >= 2 && in[0] == L'0' && in[1] == L'x') {
					base = 16;
					in.remove_prefix(2);
				}

				T result{};
				wchar_t* end{};
				if constexpr (std::is_unsigned_v<T>) {
					if constexpr (std::is_same_v<T, unsigned long long>) {
						result = std::wcstoull(in.data(), &end, base);
					}
					else {
						result = std::wcstoul(in.data(), &end, base);
					}
				}
				else {
					if constexpr (std::is_same_v<T, long long>) {
						result = std::wcstoll(in.data(), &end, base);
					}
					else {
						result = std::wcstol(in.data(), &end, base);
					}
				}
				CONTRACT_ASSERT(result != HUGE_VALF && result != HUGE_VAL && result != HUGE_VALL, "Error converting to integer type: Out of range");
				CONTRACT_ASSERT(result != 0 || end != in.data(), "Error converting to integer type: Invalid input");
				return result;
			}
		};

		//FLOATING POINT
		template<typename T>
		struct conv_helper<T, std::enable_if_t<std::is_floating_point_v<T>>> {

			//INT
			template<typename IntT>
			constexpr static inline T get(IntT in, tag_any_int) {
				return static_cast<IntT>(in);
			}
			//FLOAT
			template<typename FloatT>
			constexpr static inline T get(FloatT in, tag_floating_point) {
				return static_cast<FloatT>(in);
			}
			//BOOL
			constexpr static inline T get(bool in, instance_of<bool>) {
				return in ? 1 : 0;
			}

			//STRING TYPES
			CHARCONV_CONSTEXPR static inline T get(std::string_view in, tag_narrow_string) {
				//Not every compiler supports floating point from_chars (looking at you Embarcadero)
				//But where we can have it we want it.
				if constexpr (from_chars_exists<T>) {
					T out;
					auto result{ std::from_chars(in.data(), in.data() + in.length(), out) };
					CONTRACT_ASSERT(result.ec == std::errc{}, std::string{ "Error converting to floating point: " } + (result.ec == std::errc::invalid_argument ? " invalid input" : " out of range"));
					return out;
				}
				else {
					char* end{};
					T result{};
					if constexpr (std::is_same_v<T, float>) {
						result = std::strtof(in.data(), &end);
					}
					else if constexpr (std::is_same_v<T, double>) {
						result = std::strtod(in.data(), &end);
					}
					else {
						result = strtold(in.data(), &end);
					}
					CONTRACT_ASSERT(result != HUGE_VALF && result != HUGE_VAL && result != HUGE_VALL, "Error converting to floating point: out of range");
					CONTRACT_ASSERT(result != 0 || end != in.data(), "Error converting to floating point: invalid input");
					return result;
				}
			}

			//WIDESTRING TYPES
			static inline T get(std::wstring_view in, tag_wide_string) {
				//No from chars support here
				wchar_t* end{};
				T result{};
				if constexpr (std::is_same_v<T, float>) {
					result = std::wcstof(in.data(), &end);
				}
				else if constexpr (std::is_same_v<T, double>) {
					result = std::wcstod(in.data(), &end);
				}
				else {
					result = std::wcstold(in.data(), &end);
				}
				CONTRACT_ASSERT(result != HUGE_VALF && result != HUGE_VAL && result != HUGE_VALL, "Error converting to floating point: out of range");
				CONTRACT_ASSERT(result != 0 || end != in.data(), "Error converting to floating point: invalid input");
				return result;
			}
		};

		template<typename T>
		static constexpr inline bool is_owning_narrow_string = std::is_same_v<T, std::string>
#ifdef DP_CBUILDER11
			|| std::is_same_v<T, AnsiString>
#endif
			;

		//STD::STRING OR BORLAND ANSISTRING
		template<typename T>
		struct conv_helper<T, std::enable_if_t<is_owning_narrow_string<T>>> {
			//Other narrow string types
			constexpr static inline T get(std::string_view in, tag_narrow_string) {
				//static cast rather than T{in} to avoid initializer list constructors
				return static_cast<T>(in);
			}
			//INTEGER TYPES
			template<typename IntT>
			CHARCONV_CONSTEXPR static inline T get(IntT in, tag_any_int) {
				std::array<char, std::numeric_limits<IntT>::digits + 1> arr;
				auto result{ std::to_chars(arr.data(), arr.data() + arr.size(), in) };
				if (result.ec == std::errc::value_too_large) CONTRACT_ASSERT(false, "Error converting to std::string: input too large");
				return T(arr.data(), result.ptr - arr.data());
			}
			//FLOATING POINT TYPES
			template<typename FloatT>
			CHARCONV_CONSTEXPR static inline T get(FloatT in, tag_floating_point) {
				//Premake our buffer
				std::array<char, std::numeric_limits<FloatT>::max_digits10 + 5> arr;
				auto result{ std::to_chars(arr.data(), arr.data() + arr.size(), in) };
				if (result.ec == std::errc::value_too_large) CONTRACT_ASSERT(false, "Error converting to std::string: input too large");
				return T(arr.data(), result.ptr - arr.data());
			}
			//BOOL
			constexpr static inline T get(bool in, instance_of<bool>) {
				return in ? "true" : "false";
			}
		};

		template<typename T>
		static constexpr inline bool is_owning_wide_string = std::is_same_v<T, std::wstring>
#ifdef DP_CBUILDER11
			|| std::is_same_v<T, UnicodeString>
#endif
			;

		//WIDESTRING TYPES
		//This will have more Borland preprocessor awkwardness than the others
		template<typename T>
		struct conv_helper<T, std::enable_if_t<is_owning_wide_string<T>>> {
			//Other wide string types
			constexpr static inline T get(std::wstring_view in, tag_wide_string) {
				return static_cast<T>(in);
			}
#ifdef DP_CBUILDER11
			//C++Builder supports this of all things. Don't try at home
			static inline T get(std::string_view in, tag_narrow_string) {
				if constexpr (std::is_same_v<T, std::wstring>) {
					return T(in.begin(), in.end());
				}
				else {
					return T(in.data(), in.size());
				}
			}
#endif
			//INT TYPES
			template<typename IntT>
			static inline T get(IntT in, tag_any_int) {
				if constexpr (std::is_same_v <T, std::wstring>) {
					return std::to_wstring(in);
				}
				else {
#ifdef DP_CBUILDER11
					if constexpr (std::is_signed_v<IntT>) {
						return IntToStr(in);
					}
					else {
						return UIntToStr(in);
					}
#else
					static_assert(dependent_false<IntT>, "Conversion to unsupported widestring type");
#endif
				}
			}

			//FLOATING POINT
			template<typename FloatT>
			static inline T get(FloatT in, tag_floating_point) {
				if constexpr (std::is_same_v<T, std::wstring>) {
					return std::to_wstring(in);
				}
				else {
#ifdef DP_CBUILDER11
					return FloatToStr(in);
#else
					static_assert(dependent_false<FloatT>, "Conversion to unsupported widestring type");
#endif
				}
			}

			//BOOL
			constexpr static inline T get(bool in, instance_of<bool>) {
				return in ? L"true" : L"false";
			}

		};


#ifdef DP_CBUILDER5
		//C++BUILDER CURRENCY TYPE
		//Looking at the constructors for Currency this might seem redundant
		//However, I can assure you that thanks in part to built-in type conversions, this is not.
		template<typename T>
		struct conv_helper<T, std::enable_if_t<std::is_same_v<T, Currency>>> {
			//Trivial conversion to the same type
			static inline T get(const Currency& in, instance_of<Currency>) {
				return in;
			}
			//Integer types
			template<typename IntT>
			static inline T get(IntT in, tag_any_int) {
				return Currency{ static_cast<int>(in) };
			}
			//Floating point
			template<typename FloatT>
			static inline T get(FloatT in, tag_floating_point) {
				return Currency{ static_cast<double>(in) };
			}
			//String types
			static inline T get(std::string_view in, tag_narrow_string) {
				auto fp_val{ convert_to<double>(in) };
				return Currency{ fp_val };
			}
			//WString types
			static inline T get(std::wstring_view in, tag_wide_string) {
				auto fp_val{ convert_to<double>(in) };
				return Currency{ fp_val };
			}
		};
#endif



	}




	//Literals and C-strings don't play nicely with tag matching. Got to instantiate too many templates
	//Note we could std::remove_cv_t to support convert_to<const T>; however the function returns by value
	//so the returned value would not have any particular cv qualification. 
	//It seems misleading on the user end to have convert_to<const T> return something which isn't practically const
	template<typename To>
	constexpr To convert_to(const char* in) {
		return conv_impl::conv_helper<To>::get(std::string_view{ in }, instance_of<std::string_view>{});
	}

	template<typename To>
	constexpr To convert_to(const wchar_t* in) {
		return conv_impl::conv_helper<To>::get(std::wstring_view{ in }, instance_of<std::wstring_view>{});
	}

	//You may wonder why a const & rather than a forwarding reference.
	//The only nontrivial-to-copy types we deal with (string types) end up as string_views in the impl functions
	template<typename To, typename From>
	constexpr To convert_to(const From& in) {
		return conv_impl::conv_helper<To>::get(in, instance_of<From>{});
	}

}

#undef CHARCONV_CONSTEXPR

#endif