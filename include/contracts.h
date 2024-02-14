#ifndef DP_CONTRACTS
#define DP_CONTRACTS


#include <fstream>

#include "bits/borland_compat_typedefs.h"
#ifdef __BORLANDC__
#include "bits/borland_version_defs.h"
#endif

#if defined(DB_CBUILDER11) || __cplusplus >= 201103L || defined(_MSC_VER)
#define DP_NOEXCEPT noexcept
#else
#define DP_NOEXCEPT
#endif



/*
*	A contracts interface, allowing for standard error handling which encapsulates the nature of an error.
*	We borrow from the current P2900 design a little.
*/

namespace dp {
	namespace contract {

		//We support three contact kinds, though postconditions are a bit clunky in C++98
		enum kind {
			precondition,
			postcondition,
			assertion
		};

		inline const char* to_string(kind in) DP_NOEXCEPT {
			switch(in){
			case precondition:
				return "precondition";
			case postcondition:
				return "postcondition";
			case assertion:
				return "assertion";	
			default:
				return "[Unknown Type]";
			}
		}

		//And the three main violation handling policies
		enum policy {
			ignore,
			observe,
			enforce
		};

		//Our violation class contains and provides the nature of a contract violation
		class violation {

			kind m_kind;
			dp::compat::string	m_func;
			dp::compat::string	m_message;


		public:

			violation(kind in_kind, const char* in_func, const char* in_message) : m_kind(in_kind), m_func(in_func), m_message(in_message) {}
			violation(kind in_kind, const char* in_func, const dp::compat::string& in_message) : m_kind(in_kind), m_func(in_func), m_message(in_message) {}
#ifdef DP_CBUILDER10
			violation(kind in_kind, const char* in_func, const UnicodeString& in_message) : m_kind(in_kind), m_func(in_func), m_message(in_message) {}
#endif

			kind kind() const DP_NOEXCEPT {
				return m_kind;
			}
			const dp::compat::string& function() const DP_NOEXCEPT {
				return m_func;
			}
			const dp::compat::string& message() const DP_NOEXCEPT {
				return m_message;
			}

		};

		//And our violation exception
#ifdef DP_CBUILDER5
		class violation_exception : public Exception {
		public:
			violation_exception(const AnsiString& in) : Exception(in) {}
#ifdef DP_CBUILDER10
			violation_exception(const UnicodeString& in) : Exception(in) {}
#endif
		};
#else
		class violation_exception : public std::runtime_error {
		public:
			violation_exception(const std::string& in) : std::runtime_error(in.c_str()) {}
		};
#endif

		inline dp::compat::string default_message(const violation& in) {
			return dp::compat::string("Contract ") + to_string(in.kind()) + " violation in function " + in.function() + ": " + in.message();
		}

		//Our "violation handler" typedef
		typedef void(*handler_t)(const violation&);

		inline handler_t get_handler() DP_NOEXCEPT;
		inline policy get_policy() DP_NOEXCEPT;

		inline void default_handler(const violation& in) {
			policy current_policy = get_policy();
			if (current_policy == ignore) return;
			else if(current_policy == enforce){
				throw violation_exception(default_message(in));
			}
			else {
				//Not the most elegant solution but without any language support we need *some* kind of observe-and-continue metric
				std::ofstream out("Contract_Violations.log", std::ios_base::out | std::ios_base::app);
				//c_str because AnsiString operator<< cannot be guaranteed to be available.
				out << default_message(in).c_str() << '\n';
				return;
			}
		}

		namespace detail {
			//I *hate* this ugly hack but without inline variables it's where we are.
			//We know that both of these functions will always be called before a violation
			//is created so there's no chance of awkward sequencing issues.
			inline handler_t& current_handler() DP_NOEXCEPT {
				static handler_t current_handler = default_handler;
				return current_handler;
				
			}
			inline policy& current_policy() DP_NOEXCEPT {
				static policy current_policy = enforce;
				return current_policy;
			}
		}


		inline void set_handler(handler_t in) DP_NOEXCEPT {
			detail::current_handler() = in;
		}
		inline handler_t get_handler() DP_NOEXCEPT {
			return detail::current_handler();
		}

		inline void set_policy(policy in) DP_NOEXCEPT {
			detail::current_policy() = in;
		}
		inline policy get_policy() DP_NOEXCEPT {
			return detail::current_policy();
		}
	}
}

/*
*	And now we move on to the ugly preprocessor magic which makes it all work.
*/

//Try to get a cross platform call to __FUNCTION__
#ifdef __BORLANDC__
#define DP_FUNC __FUNC__
#elif defined (_MSC_VER)
#define DP_FUNC __func__
#elif defined(__GNUC__)
#define DP_FUNC __FUNCTION__
#else
#error "No function handle found"
#endif

//We allow 3 "overloads" of the contract annotations - condition, label, and single-use handler.
#define CONTRACT_ASSERT1(cond)						if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) dp::contract::get_handler()(dp::contract::violation(dp::contract::assertion, DP_FUNC, #cond));
#define CONTRACT_ASSERT2(cond, message)				if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) dp::contract::get_handler()(dp::contract::violation(dp::contract::assertion, DP_FUNC, message));
#define CONTRACT_ASSERT3(cond, message, handler)	if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) handler(dp::contract::violation(dp::contract::assertion, DP_FUNC, message));

#define PRE1(cond)						if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) dp::contract::get_handler()(dp::contract::violation(dp::contract::precondition, DP_FUNC, #cond));
#define PRE2(cond, message)				if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) dp::contract::get_handler()(dp::contract::violation(dp::contract::precondition, DP_FUNC, message));
#define PRE3(cond, message, handler)	if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) handler(dp::contract::violation(dp::contract::precondition, DP_FUNC, message));

#define POST1(cond)						if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) dp::contract::get_handler()(dp::contract::violation(dp::contract::postcondition, DP_FUNC, #cond));
#define POST2(cond, message)			if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) dp::contract::get_handler()(dp::contract::violation(dp::contract::postcondition, DP_FUNC, message));
#define POST3(cond, message, handler)	if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) handler(dp::contract::violation(dp::contract::postcondition, DP_FUNC, message));


//In C++11 and up (and in C++Builder10 for some reason) we have __VA_ARGS__ to "overload" macros
//MSVC has a bug in expanding variadic macros
#ifdef _MSC_VER
#define GLUE(x, y) x y
#define RETURN_ARG_COUNT(_1_, _2_, _3_, count, ...) count
#define EXPAND_ARGS(args) RETURN_ARG_COUNT args
#define NUM_ARGS(...) EXPAND_ARGS((__VA_ARGS__, 3, 2, 1, 0))
#define OVERLOAD_MACRO2(name, count) name##count
#define OVERLOAD_MACRO1(name, count) OVERLOAD_MACRO2(name, count)
#define OVERLOAD_MACRO(name, count) OVERLOAD_MACRO1(name, count)
#define CALL_OVERLOAD(name, ...) GLUE(OVERLOAD_MACRO(name, NUM_ARGS(__VA_ARGS__)), (__VA_ARGS__))

#define CONTRACT_ASSERT(...) CALL_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)
#define PRE(...) CALL_OVERLOAD(PRE, __VA_ARGS__)
#define POST(...) CALL_OVERLOAD(POST, __VA_ARGS__)

#elif defined(DP_CBUILDER10) || __cplusplus >= 201103L 
//But other compilers do not
#define NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, TOTAL, ...) TOTAL
#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__, 6, 5, 4, 3, 2, 1)
#define CONCAT_IMPL(X, Y) X##Y  
#define CONCAT(MACRO, NUMBER) CONCAT_IMPL(MACRO, NUMBER)
#define CALL_OVERLOAD(MACRO, ...) CONCAT(MACRO, NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define CONTRACT_ASSERT( ... ) CALL_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)
#define PRE(...) CALL_OVERLOAD(PRE, __VA_ARGS__)
#define POST(...) CALL_OVERLOAD(POST, __VA_ARGS__)
#else
//And we're on a C++98 compiler we can't overload anyway
#define CONTRACT_ASSERT(cond) CONTRACT_ASSERT1(cond)
#define PRE(cond) PRE1(cond)
#define POST(cond) POST1(cond)
#endif





#undef DP_NOEXCEPT

#endif
