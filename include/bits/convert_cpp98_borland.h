#ifndef DP_CPP98_CONVERT_BORLAND
#define DP_CPP98_CONVERT_BORLAND

#if defined(DP_CPP17_CONVERT_TO) || defined(DP_CPP98_CONVERT_TO)
#error "Using multiple versions of the convert_to header is not supported";
#endif

#ifndef __BORLANDC__
#error "This is a header for the C++Builder Classic Borland compiler. It will not work on other compilers"
#endif

#include <string>
#include <cstdlib>
#include <climits>
#include <sstream>

#include "bits/support_tools.h"
#include "bits/borland_version_defs.h"
#include "bits/tmp_tags.h"

/*
*	The template engine on the Borland C++Builder compiler is weak and noncompliant.
*	Consequently, the practice of using template specialisations and enable_if does not work.
*
*	As such, we need to do something much uglier and much more unholy in order to get a functional borland tool.
*	On the plus side, let's us remove a bunch of preprocessor garbage from the the standard C++98 conversion header
*/

namespace dp {

	//Forward dec
	template<typename To, typename From>
	To convert_to(const From&);



	namespace conv_impl {

		//Side note: is_constructible is also not implementable in the Borland compiler so...
		template<typename T>
		struct is_delphi_string : dp::support::false_type {};

		template<>
		struct is_delphi_string<AnsiString> : dp::support::true_type {};
#ifdef DP_CBUILDER10
		template<>
		struct is_delphi_string<UnicodeString> : dp::support::true_type {};
#endif


		//INT TO INT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_integral<To>::value, To>::type fun(From in, tag_any_int) {
			return static_cast<To>(in);
		}

		//INT TO FLOAT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_floating_point<To>::value, To>::type fun(From in, tag_any_int) {
			return static_cast<To>(in);
		}

		//INT TO BOOL
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, bool>::value, To>::type fun(From in, tag_any_int) {
			return in == 0 ? false : true;
		}

		//INT TO STD::STRING
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, std::string>::value, To>::type fun(From in, tag_any_int) {
			std::stringstream ss;
			ss << in;
			return ss.str();
		}

		//SIGNED INT TO DELPHI STRING
		template<typename To, typename From>
		typename dp::support::enable_if<is_delphi_string<To>::value, To>::type fun(From in, tag_signed_int) {
			return IntToStr(in);
		}

		//UNSIGNED INT TO DELPHI STRING
		template<typename To, typename From>
		typename dp::support::enable_if<is_delphi_string<To>::value, To>::type fun(From in, tag_unsigned_int) {
			return UIntToStr(in);
		}

		//INT TO CURRENCY
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, Currency>::value, To>::type fun(From in, tag_any_int) {
			return Currency(static_cast<int>(in));
		}




		//FLOAT TO INT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_integral<To>::value, To>::type fun(From in, tag_floating_point) {
			return static_cast<To>(in);
		}

		//FLOAT TO FLOAT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_floating_point<To>::value, To>::type fun(From in, tag_floating_point) {
			return static_cast<To>(in);
		}

		//FLOAT TO BOOL
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, bool>::value, To>::type fun(From in, tag_floating_point) {
			return in == 0 ? false : true;
		}

		//FLOAT TO STD::STRING
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, std::string>::value, To>::type fun(From in, tag_floating_point) {
			std::stringstream ss;
			ss << in;
			return ss.str();
		}

		//FLOAT TO DELPHI STRING
		template<typename To, typename From>
		typename dp::support::enable_if<is_delphi_string<To>::value, To>::type fun(From in, tag_floating_point) {
			return FloatToStr(in);
		}

		//FLOAT TO CURRENCY
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, Currency>::value, To>::type fun(From in, tag_floating_point) {
			return Currency(static_cast<int>(in));
		}

		//BOOL TO NARROW STRING
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, std::string>::value || dp::support::is_same<To, AnsiString>::value, To>::type fun(From in, instance_of<bool>) {
			return in ? "true" : "false";
		}

		//BOOL TO WIDE STRING
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, std::wstring>::value || dp::support::is_same<To, UnicodeString>::value, To>::type fun(From in, instance_of<bool>) {
			return in ? L"true" : L"false";
		}



		//STD::STRING TO SIGNED INT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_integral<To>::value&& dp::support::is_signed<To>::value, To>::type fun(const From& in, instance_of<std::string>) {
			char* end = NULL;
			long result = std::strtol(in.c_str(), &end, 0);

			if (result == LONG_MAX || result == LONG_MIN) throw Exception("Error converting a std::string to integer type via convert_to");
			return static_cast<To>(result);
		}

		//STD::STRING TO UNSIGNED INT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_integral<To>::value && !dp::support::is_signed<To>::value, To>::type fun(const From& in, instance_of<std::string>) {
			char* end = NULL;
			unsigned long result = std::strtoul(in.c_str(), &end, 0);

			if (result == ULONG_MAX) throw Exception("Error converting a std::string to integer type via convert_to");
			return static_cast<To>(result);
		}

		//STD::STRING TO FLOAT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_floating_point<To>::value, To>::type fun(const From& in, instance_of<std::string>) {
			char* end = NULL;
			To result = static_cast<To>(strtod(in.c_str(), &end));

			if (result == HUGE_VAL || end == in) throw Exception("Error converting a std::string to floating point via convert_to");
			return result;
		}

		//STD::STRING TO STD::STRING
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, std::string>::value, To>::type fun(const From& in, instance_of<std::string>) {
			return in;
		}

		//STD::STRING TO DELPHI STRING
		template<typename To, typename From>
		typename dp::support::enable_if<is_delphi_string<To>::value, To>::type fun(const From& in, instance_of<std::string>) {
			return in.c_str();
		}

		//STD::STRING TO CURRENCY
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, Currency>::value, To>::type fun(const From& in, tag_std_string) {
			return Currency(convert_to<double>(in));
		}

		//DELPHI STRING TO SIGNED INT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_integral<To>::value&& dp::support::is_signed<To>::value, To>::type fun(const From& in, tag_delphi_string) {
			return static_cast<To>(StrToInt(in));
		}

		//DELPHI STRING TO UNSIGNED INT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_integral<To>::value && !dp::support::is_signed<To>::value, To>::type fun(const From& in, tag_delphi_string) {
			return static_cast<To>(StrToUInt(in));
		}

		//DELPHI STRING TO FLOAT
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_floating_point<To>::value, To>::type fun(const From& in, tag_delphi_string) {
			return static_cast<To>(StrToFloat(in));
		}

		//DELPHI STRING TO STD::STRING
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, std::string>::value, To>::type fun(const From& in, instance_of<AnsiString>) {
			return in.c_str();
		}
#ifdef DP_CBUILDER10
		//DELPHI STRING TO STD::STRING
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, std::wstring>::value, To>::type fun(const From& in, instance_of<UnicodeString>) {
			return in.c_str();
		}
#endif
		//DELPHI STRING TO DELPHI STRING
		template<typename To, typename From>
		typename dp::support::enable_if<is_delphi_string<To>::value, To>::type fun(const From& in, tag_delphi_string) {
			return in;
		}

		//DELPHI STRING TO CURRENCY
		template<typename To, typename From>
		typename dp::support::enable_if<dp::support::is_same<To, Currency>::value, To>::type fun(const From& in, tag_delphi_string) {
			return Currency(convert_to<double>(in));
		}

		//C-STRING TO TYPE
		//Yes this uses unecessary allocation but getting the Borland compiler to recognise literals was not easy.
		template<typename To, typename From>
		To fun(const From& in, instance_of<char*>) {
			return convert_to<To>(std::string(in));
		}
		template<typename To, typename From>
		To fun(const From& in, instance_of<const char*>) {
			return convert_to<To>(std::string(in));
		}
		template<typename To, typename From>
		To fun(const From& in, instance_of<wchar_t*>) {
			return convert_to<To>(std::wstring(in));
		}
		template<typename To, typename From>
		To fun(const From& in, instance_of<const wchar_t*>) {
			return convert_to<To>(std::wstring(in));
		}
	}


	template<typename To, typename From>
	To convert_to(const From& in) {
		//Ugly decay to account for string literals
		return conv_impl::fun<To, From>(in, instance_of<typename dp::support::decay_if_array<From>::type>());
	}




}



#endif