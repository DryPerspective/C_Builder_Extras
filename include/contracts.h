#ifndef DP_CONTRACTS
#define DP_CONTRACTS


#include <fstream>
#include <exception>

#include "defer.h"

#include "bits/macros.h"

#include "bits/borland_compat_typedefs.h"
#ifdef __BORLANDC__
#include "bits/borland_version_defs.h"
#endif

#if defined(DB_CBUILDER11) || __cplusplus >= 201103L || defined(_MSC_VER)
#define DP_NOEXCEPT noexcept
#else
#define DP_NOEXCEPT
#endif

#if defined(DB_CBUILDER11) || __cplusplus >= 201103L || defined(_MSC_VER)
#define DP_NORETURN [[noreturn]]
#else
#define DP_NORETURN
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
		switch (in) {
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

	//And our violation exception.
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


	/*
	*	It's possible for the default handler to be called during stack unwinding if a deferred postcondition is evaluated as a part of stack unwinding.
	*	If we have an exception active, then this is bad.
	*	As such the default handler will not throw an additional exception if there is already an active current one.
	*	We need another macro to ensure we call the right function
	*	Borland don't define the versioning macros
	*/
#if defined(DP_CBUILDER11) || __cpp_lib_uncaught_exceptions
#define DP_UNCAUGHT_EXCEPTION std::uncaught_exceptions()
#else
#define DP_UNCAUGHT_EXCEPTION std::uncaught_exception()
#endif


	inline void default_observe(const violation& in) {
		//Not the most elegant solution but without any language support we need *some* kind of observe-and-continue metric
		std::ofstream out("Contract_Violations.log", std::ios_base::out | std::ios_base::app);
		//c_str because AnsiString operator<< cannot be guaranteed to be available.
		out << default_message(in).c_str() << '\n';
		return;
	}

	DP_NORETURN inline void default_enforce(const violation& in) {
		throw violation_exception(default_message(in));
	}
	

	inline void default_handler(const violation& in) {
		policy current_policy = get_policy();
		if (current_policy == ignore) return;
		else if (current_policy == enforce) {
			if (DP_UNCAUGHT_EXCEPTION) {
#ifdef DP_CBUILDER5
				ShowMessage("Contract violation occurred during active exception: " + default_message(in));
#else
				default_observe(in);
#endif
			}
			else {
				default_enforce(in);
			}
		}
		//If observe
		else {
			default_observe(in);
		}
	}

#undef DP_UNCAUGHT_EXCEPTION

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


	inline handler_t set_handler(handler_t in) DP_NOEXCEPT {
		handler_t prev = detail::current_handler();
		detail::current_handler() = in;
		return prev;

	}
	inline handler_t get_handler() DP_NOEXCEPT {
		return detail::current_handler();
	}

	inline policy set_policy(policy in) DP_NOEXCEPT {
		policy prev = detail::current_policy();
		detail::current_policy() = in;
		return prev;
	}
	inline policy get_policy() DP_NOEXCEPT {
		return detail::current_policy();
	}
}
}


/*
*	And now we move on to the ugly preprocessor magic which makes it all work.
*/
//We allow 3 "overloads" of the contract annotations - condition, label, and single-use handler.
#define CONTRACT_ASSERT3(cond, message, handler)	if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) handler(dp::contract::violation(dp::contract::assertion, DP_FUNC, message))
#define CONTRACT_ASSERT2(cond, message)				CONTRACT_ASSERT3(cond, message, (dp::contract::get_handler() ? dp::contract::get_handler() : dp::contract::default_handler))
#define CONTRACT_ASSERT1(cond)						CONTRACT_ASSERT2(cond, #cond)

#define PRE3(cond, message, handler)	if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) handler(dp::contract::violation(dp::contract::precondition, DP_FUNC, message))
#define PRE2(cond, message)				PRE3(cond, message, (dp::contract::get_handler() ? dp::contract::get_handler() : dp::contract::default_handler))
#define PRE1(cond)						PRE2(cond, #cond)

//If on C++17 we can defer evaluation of postconditions to the end of the scope
#if defined(DP_CBUILDER11) || __cplusplus >= 201703L || _MSVC_LANG >= 201703L
//We have to construct the violation object as a separate expression so that the call to __func__ isn't deferred to inside operator() of the defer class
//We also have to make it on the same line so the names can be generated without collision.
#define POST3(cond, message, handler)	dp::contract::violation DP_CONCAT(violation,__LINE__){dp::contract::postcondition, DP_FUNC, message}; DEFER(if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) handler(DP_CONCAT(violation,__LINE__)))
#else
#define POST3(cond, message, handler)	if(dp::contract::get_policy() != dp::contract::ignore && ! ( cond )) handler(dp::contract::violation(dp::contract::postcondition, DP_FUNC, message))
#endif
#define POST2(cond, message)			POST3(cond, message, (dp::contract::get_handler() ? dp::contract::get_handler() : dp::contract::default_handler))
#define POST1(cond)						POST2(cond, #cond)




//We can "overload" our macros to call a selected one based on the number of args
#if defined(DP_CBUILDER10) || __cplusplus >= 201103L || defined(_MSC_VER)
#define CONTRACT_ASSERT( ... )	DP_MACRO_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)
#define PRE(...)				DP_MACRO_OVERLOAD(PRE, __VA_ARGS__)
#define POST(...)				DP_MACRO_OVERLOAD(POST, __VA_ARGS__)
//But C++Builder10 does not enjoy proper DEFER functionality, despite having __VA_ARGS__
//C++Builder11 does, as does every other compiler which supports __VA_ARGS__.
#if defined(DP_CBUILDER10) && !defined(DP_CBUILDER11)
#define DEFER_POST(...)			DEFER(POST(__VA_ARGS__))
#else
#define DEFER_POST(...)			POST(__VA_ARGS__)
#endif
#else
//And we're on a C++98 compiler we can't overload anyway
//Believe it or not, with the limitations on DEFER in C++98, it's easier to spin a new version just for deferred postconditions.
#define DP_DEFER_POST_STRUCT(cond, violation_name) struct DP_CONCAT(struct_, violation_name){ \
														dp::contract::violation m_viol; \
														DP_CONCAT(struct_, violation_name)(const dp::contract::violation& in) : m_viol(in) {} \
														~DP_CONCAT(struct_, violation_name)() { \
															if (dp::contract::get_policy() != dp::contract::ignore && !(cond)) {   \
																	(dp::contract::get_handler() ? dp::contract::get_handler() : dp::contract::default_handler)(m_viol); \
															} \
														} \
													} DP_CONCAT(instance_, violation_name)(violation_name); 

#define CONTRACT_ASSERT(cond)	CONTRACT_ASSERT1(cond)
#define	PRE(cond)				PRE1(cond)
#define	POST(cond)				POST1(cond)
#define DEFER_POST(cond)		dp::contract::violation DP_CONCAT(violation,__LINE__)(dp::contract::postcondition, DP_FUNC, #cond); DP_DEFER_POST_STRUCT(cond, DP_CONCAT(violation,__LINE__))
#endif





#undef DP_NOEXCEPT

#endif
