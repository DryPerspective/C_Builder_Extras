#ifndef DP_CPP98_CONTRACTS
#define DP_CPP98_CONTRACTS

#ifdef DP_CPP17_CONTRACTS
#error "Detected both C++98 and C++17 contracts in the same TU"
#endif

#include <fstream>
#include <stdexcept>
#include <string>

#include "bits/borland_compat_typedefs.h"
#include "source_location.h"

#ifdef __BORLANDC__
#include "bits/borland_version_defs.h"
#endif


namespace dp {
	namespace contract {
		//Policy enum
		enum policy {
			enforce,
			observe,
			ignore
		};

		class violation;

		//Our handler function type
		typedef void(*handler_t)(violation);

		//Our representation of a violation
		class violation {

			dp::source_location loc;
			std::string msg;

			void append_message(const char* in) {
				msg = in;
			}

			friend inline void contract_assert(bool, dp::compat::string, handler_t, violation);

		public:

			violation(const dp::source_location& in_loc, const std::string in_msg) : loc(in_loc), msg(in_msg) {}

			const char* function() const {
				return loc.function;
			}
			
			const char* file() const {
				return loc.file;
			}

			const char* message() const {
				return msg.c_str();
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
			violation_exception(const std::string& in) : Exception(in.c_str()) {}
		};
#else
		class violation_exception : public std::runtime_error {
		public:
			violation_exception(const std::string& in) : std::runtime_error(in.c_str()) {}
		};
#endif

		std::string default_message(const violation& in) {
			return std::string("Contract violation in function ") + in.function() + ": " + in.message();
		}

		void default_enforce(violation viol) {
			throw violation_exception(default_message(viol));
		}


		void default_observe(violation viol) {
			std::ofstream out("Contract violations.log", std::ios_base::out | std::ios_base::app);
			out << default_message(viol) << '\n';
		}

		//Forward dec as we need to be aware of this func.
		policy get_policy();

		void default_handler(violation viol) {
			const policy pol = get_policy();
			if (pol == enforce) {
				default_enforce(viol);
			}
			else if (pol == observe) {
				default_observe(viol);
			}
			//Ignore does nothing.
		}

		


		//Our currently set handler/policy
		namespace detail {
			inline handler_t& handle_impl() {
				static handler_t handler = default_handler;
				return handler;
			}
			inline policy& pol_impl() {
				static policy pol = enforce ;
				return pol;
			}
		}

		inline handler_t set_handler(handler_t new_handler) {
			if (new_handler == NULL) throw dp::contract::violation_exception("Attempt to set null handler");
			handler_t old = detail::handle_impl();
			detail::handle_impl() = new_handler;
			return old;
		}

		inline handler_t get_handler() {
			return detail::handle_impl();
		}

		inline policy set_policy(policy new_policy) {
			policy old = detail::pol_impl();
			detail::pol_impl() = new_policy;
			return old;
		}

		inline policy get_policy() {
			return detail::pol_impl();
		}





		inline void contract_assert(bool condition, dp::compat::string message, handler_t hand, dp::contract::violation viol = dp::contract::violation(DP_SOURCE_LOCATION_CURRENT, "")) {
			if (condition) return;

			viol.append_message(message.c_str());
			hand(viol);
		}

		inline void contract_assert(bool condition, dp::compat::string message, dp::contract::violation viol = dp::contract::violation(DP_SOURCE_LOCATION_CURRENT, "")) {
			contract_assert(condition, message, get_handler(), viol);
		}

		inline void contract_assert(bool condition, dp::contract::violation viol = dp::contract::violation(DP_SOURCE_LOCATION_CURRENT, "")) {
			if (condition) return;

			get_handler()(viol);
		}
	}

	using contract::contract_assert;
}

/*
*  I don't like macros, but given the very limited C++98 language features, they make this library much easier to use
*  And given this is a safety tool, we want it to be as easy to use as possible
*/

#ifndef DP_NO_CONTRACT_MACROS

#define CONTRACT_ASSERT3(cond, message, handler)	dp::contract::contract_assert(cond, message, handler, dp::contract::violation(DP_SOURCE_LOCATION_THIS_FUNCTION, message))
#define CONTRACT_ASSERT2(cond, message)				CONTRACT_ASSERT3(cond, message, dp::contract::get_handler())
#define CONTRACT_ASSERT1(cond)						CONTRACT_ASSERT2(cond, #cond)

// If we're in C++98 we don't have __VA_ARGS__. However, C++Builder 10 is a strange exception to this rule. Go figure.
#if defined(DP_CBUILDER10) || __cplusplus >= 201103L || defined(_MSC_VER)
#define CONTRACT_ASSERT(...)	DP_MACRO_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)
#else
#define CONTRACT_ASSERT(arg)	CONTRACT_ASSERT1(arg)
#endif
#endif



#endif