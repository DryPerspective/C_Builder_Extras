#ifndef DP_CPP98_CONTRACTS
#define DP_CPP98_CONTRACTS

#ifdef DP_CPP17_CONTRACTS
#error "Detected both C++98 and C++17 contracts in the same TU"
#endif

#include <fstream>
#include <stdexcept>

#include "bits/borland_compat_typedefs.h"
#include "bits/source_location.h"

#ifdef __BORLANDC__
#include "bits/borland_version_defs.h"
#endif


namespace dp {
	namespace contract {
		//Makeshift scoped enum for our policy
		struct policy {
			enum type {
				enforce,
				observe,
				ignore
			};
		};

		//Our representation of a violation
		class violation {

			dp::source_location loc;
			dp::compat::string msg;

		public:

			violation(const dp::source_location in_loc, const dp::compat::string in_msg) : loc(in_loc), msg(in_msg) {}

			dp::compat::string function() const {
				return loc.function;
			}
			
			dp::compat::string file() const {
				return loc.file;
			}

			dp::compat::string message() const {
				return msg;
			}

			int line() const {
				return loc.line;
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

		dp::compat::string default_message(const violation& in) {
			return "Contract violation in function " + in.function() + " : " + in.message();
		}

		struct default_handler {

			void enforce(bool condition) {
				if (!condition) throw violation_exception("Contract violation");
			}
			void enforce(bool condition, const dp::contract::violation& viol) {
				if (!condition) throw violation_exception(default_message(viol));
			}

			void observe(bool condition) {
				if (!condition) {
					std::ofstream out("Contract violations.log", std::ios_base::out | std::ios_base::app);
					out << "Contract violation\n";
				}
			}
			void observe(bool condition, const dp::contract::violation& viol) {
				if (!condition) {
					std::ofstream out("Contract violations.log", std::ios_base::out | std::ios_base::app);
					out << default_message(viol) << '\n';
				}
			}

			void ignore(bool) {}
			void ignore(bool, const dp::contract::violation&) {}

		};

		//And our primary function. We suffer from the lack of C++98's support for default template arguments on template functions
		template<policy::type pol, typename handler>
		void contract_assert(bool condition, handler hand, dp::contract::violation viol) {
			if (pol == policy::enforce) {
				hand.enforce(condition, viol);
			}
			else if (pol == policy::observe) {
				hand.observe(condition, viol);
			}
			else {
				hand.ignore(condition, viol);
			}
		}

	}
}

/*
*  I don't like macros, but given the very limited C++98 language features, they make this library much easier to use
*  And given this is a safety tool, we want it to be as easy to use as possible
*/

//Hook to define the handler used by the predefined macros.
#ifndef DP_DEFAULT_HANDLER
#define DP_DEFAULT_HANDLER dp::contract::default_handler
#endif

#ifndef DP_DEFAULT_POLICY
#define DP_DEFAULT_POLICY dp::contract::policy::enforce
#endif

#define CONTRACT_ASSERT3(cond, message, handler)	dp::contract::contract_assert<dp::contract::policy:: DP_DEFAULT_POLICY>(cond, handler(), dp::contract::violation(DP_SOURCE_LOC_HERE, message))
#define CONTRACT_ASSERT2(cond, message)				CONTRACT_ASSERT3(cond, message, DP_DEFAULT_HANDLER)
#define CONTRACT_ASSERT1(cond)						CONTRACT_ASSERT2(cond, #cond)

// If we're in C++98 we don't have __VA_ARGS__. However, C++Builder 10 is a strange exception to this rule. Go figure.
#if defined(DP_CBUILDER10) || __cplusplus >= 201103L || defined(_MSC_VER)
#define CONTRACT_ASSERT(...)	DP_MACRO_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)
#else
#define CONTRACT_ASSERT(arg)	CONTRACT_ASSERT1(arg)
#endif




#endif