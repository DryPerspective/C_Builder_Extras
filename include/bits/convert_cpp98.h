#ifndef DP_CPP98_CONVERT_TO
#define DP_CPP98_CONVERT_TO

#if defined(DP_CPP17_CONVERT_TO) || defined(DP_CPP98_CONVERT_BORLAND)
#error "Using multiple versions of the convert_to header is not supported";
#endif

#include <string>
#include <cstdlib>
#include <climits>
#include <sstream>


#include "tmp_tags.h"
#include "support_tools.h"
#include "contracts.h"

/*
*	The C++98 equivalent for convert_to
*	Note that this does not use any Borland types as the techniques used here, though valid C++98, will not compile on Borland's compiler
*	A Borland header is provided separately.
*/


namespace dp {

	//Forward dec
	template<typename To, typename From>
	To convert_to(const From&);

	namespace conv_impl {

		template<typename T, typename = void>
		struct conv_helper {};

		//SIGNED INTEGRAL TYPES
		template<typename T>
		struct conv_helper<T, typename dp::support::enable_if<dp::support::is_integral<T>::value&& dp::support::is_signed<T>::value>::type> {

			//INT
			template<typename IntT>
			static inline T get(IntT in, tag_any_int) {
				return static_cast<T>(in);
			}
			//FLOAT
			template<typename FloatT>
			static inline T get(FloatT in, tag_floating_point) {
				return static_cast<T>(in);
			}
			//BOOL
			static inline T get(bool in, instance_of<bool>) {
				return in ? 1 : 0;
			}
			//C-STRING
			//Can't be DRY with std::string :(
			template<std::size_t N>
			static inline T get(const char(&in)[N], instance_of<char[N]>) {
				char* end = NULL;
				long result = std::strtol(in, &end, 0);

				CONTRACT_ASSERT(result != LONG_MAX && result != LONG_MIN, "Error converting a C-string to integer type via convert_to");
				return static_cast<T>(result);
			}
			//STD::STRING
			static inline T get(const std::string& in, instance_of<std::string>) {
				char* end = NULL;
				long result = std::strtol(in.c_str(), &end, 0);

				CONTRACT_ASSERT(result != LONG_MAX && result != LONG_MIN, "Error converting a std::string to integer type via convert_to");
				return static_cast<T>(result);
			}
		};


		//UNSIGNED INTEGRAL TYPES
		template<typename T>
		struct conv_helper<T, typename dp::support::enable_if<dp::support::is_integral<T>::value && !dp::support::is_signed<T>::value>::type> {
			//INT
			template<typename IntT>
			static inline T get(IntT in, tag_any_int) {
				return static_cast<T>(in);
			}
			//FLOAT
			template<typename FloatT>
			static inline T get(FloatT in, tag_floating_point) {
				return static_cast<T>(in);
			}
			//BOOL
			static inline T get(bool in, instance_of<bool>) {
				return in ? 1 : 0;
			}
			//C-String
			template<std::size_t N>
			static inline T get(const char(&in)[N], instance_of<char[N]>) {
				char* end = NULL;
				unsigned long result = std::strtoul(in, &end, 0);

				CONTRACT_ASSERT(result != ULONG_MAX, "Error converting a C-string to integer type via convert_to");
				return static_cast<T>(result);
			}
			//STD::STRING
			static inline T get(const std::string& in, instance_of<std::string>) {
				char* end = NULL;
				unsigned long result = std::strtoul(in.c_str(), &end, 0);

				CONTRACT_ASSERT(result != ULONG_MAX, "Error converting a std::string to integer type via convert_to");
				return static_cast<T>(result);
			}
		};

		//FLOATING POINT TYPES
		template<typename T>
		struct conv_helper<T, typename dp::support::enable_if<dp::support::is_floating_point<T>::value>::type> {
			//INT
			template<typename IntT>
			static inline T get(IntT in, tag_any_int) {
				return static_cast<T>(in);
			}
			//FLOAT
			template<typename FloatT>
			static inline T get(FloatT in, tag_floating_point) {
				return static_cast<T>(in);
			}
			//BOOL
			static inline T get(bool in, instance_of<bool>) {
				return in ? 1 : 0;
			}
			//C-String
			template<std::size_t N>
			static inline T get(const char(&in)[N], instance_of<char[N]>) {
				T result = 0;
				char* end = NULL;
				//No comptime branching. Ugh.
				if (dp::support::is_same<T, float>::value) {
					//Because this must be a runtime branch, the compiler will warn about potentially narrowing conversions
					//even though we will never go down the paths that create them
					//So we use a redundant cast to suppress the warning.
					result = static_cast<T>(strtof(in, &end));
				}
				else if (dp::support::is_same<T, double>::value) {
					result = static_cast<T>(strtod(in, &end));
				}
				else {
					result = static_cast<T>(strtold(in, &end));
				}
				CONTRACT_ASSERT(result != HUGE_VAL && result != HUGE_VALF && result != HUGE_VALL && end != in, "Error converting a C-string to floating point via convert_to");
				return result;
			}
			//STD::STRING
			static inline T get(const std::string& in, instance_of<std::string>) {
				char* end = NULL;
				T result = static_cast<T>(strtod(in.c_str(), &end));
				CONTRACT_ASSERT(result != HUGE_VAL && result != HUGE_VALF && result != HUGE_VALL && end != in, "Error converting a std::string to floating point via convert_to");
				return result;
			}
		};

		//STD::STRING
		template<typename T>
		struct conv_helper<T, typename dp::support::enable_if<dp::support::is_same<std::string, T>::value>::type> {
			//INT
			//Suboptimal that tag dispatch means we have two identicl function bodies, but needs must
			template<typename IntT>
			static inline T get(IntT in, tag_any_int) {
				std::stringstream ss;
				ss << in;
				return ss.str();
			}
			//FLOAT
			template<typename FloatT>
			static inline T get(FloatT in, tag_floating_point) {
				std::stringstream ss;
				ss << in;
				return ss.str();
			}
			//BOOL
			static inline T get(bool in, instance_of<bool>) {
				return in ? "true" : "false";
			}
			//C-String
			template<std::size_t N>
			static inline T get(const char(&in)[N], instance_of<char[N]>) {
				return std::string(in);
			}
			//STD::STRING
			static inline T get(const std::string& in, instance_of<std::string>) {
				return in;
			}
		};

		//STD::WSTRING
		template<typename T>
		struct conv_helper<T, typename dp::support::enable_if<dp::support::is_same<T, std::wstring>::value>::type> {
			//INT
			template<typename IntT>
			static inline T get(IntT in, tag_any_int) {
				std::wstringstream ss;
				ss << in;
				return ss.str();
			}
			//FLOAT
			template<typename FloatT>
			static inline T get(FloatT in, tag_floating_point) {
				std::wstringstream ss;
				ss << in;
				return ss.str();
			}
			//BOOL
			static inline T get(bool in, instance_of<bool>) {
				return in ? L"true" : L"false";
			}
			//C-String
			template<std::size_t N>
			static inline T get(const wchar_t(&in)[N], instance_of<wchar_t[N]>) {
				return std::wstring(in);
			}
			//STD::WSTRING
			static inline T get(const std::wstring& in, instance_of<std::wstring>) {
				return in;
			}

		};


	}


	template<typename To, typename From>
	To convert_to(const From& in) {
		return conv_impl::conv_helper<To>::get(in, instance_of<From>());
	}


}



#endif