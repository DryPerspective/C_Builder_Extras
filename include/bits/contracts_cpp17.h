#ifndef DP_CPP17_CONTRACTS
#define DP_CPP17_CONTRACTS

/*
*  Documentation is here: https://github.com/DryPerspective/C_Builder_Extras/wiki/Contracts
*/

#ifdef DP_CPP98_CONTRACTS
#error "Detected both C++98 and C++17 contracts in the same TU"
#endif

#include <iostream>
#include <string_view>
#include <fstream>
#include <atomic>


#include "source_location.h"


namespace dp {
	namespace contract {
		enum class policy {
			enforce,
			observe,
			ignore
		};

		//For backwards compatibility and ease of use, we make the names available in this scope
		constexpr inline auto enforce = policy::enforce;
		constexpr inline auto observe = policy::observe;
		constexpr inline auto ignore = policy::ignore;

		class violation;

		using handler_t = void(*)(violation);


		class violation {
			dp::source_location loc;
			std::string_view msg;

			constexpr void append_message(std::string_view in_msg) {
				msg = in_msg;
			}

			friend inline void assert_impl(std::string_view, handler_t, violation);

		public:

			constexpr violation(const dp::source_location in_loc, std::string_view in_msg) : loc{ in_loc }, msg{ in_msg } {}

			constexpr const char* function() const {
				return loc.function;
			}

			constexpr const char* file() const {
				return loc.file;
			}

			constexpr std::string_view message() const {
				return msg;
			}

			constexpr int line() const {
				return loc.line;
			}


		};
#ifdef __BORLANDC__
		class violation_exception : public Exception {
		public:
			violation_exception(std::string_view in) : Exception(in) {};
		};
#else
		class violation_exception : public std::runtime_error {
		public:
			violation_exception(const std::string& msg) : std::runtime_error{ msg } {}
		};
#endif


		std::string default_message(const violation& in) {
			return std::string{ "Contract violation in function " } + in.function() + ": " + std::string{ in.message() };
		}

#ifdef __BORLANDC__
		//Because all things Borland want UnicodeString as the base unit for all string processing
		//And because there's no simple and safe way to convert an existing std::string to UnicodeString
		//We offer an alternative.
		UnicodeString default_message_us(const violation& in) {
			return UnicodeString{ L"Contract violation in function " } + in.function() + L": " + UnicodeString{ in.message() };
		}
#endif

		[[noreturn]] void default_enforce(violation viol) {
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
			inline std::atomic<handler_t>& handle_impl() {
				static std::atomic<handler_t> handler{ default_handler };
				return handler;
			}
			inline std::atomic<policy>& pol_impl() {
				static std::atomic<policy> pol{ enforce };
				return pol;
			}
		}

		inline handler_t set_handler(handler_t new_handler) {
			if (new_handler == nullptr) throw dp::contract::violation_exception("Attempt to set null handler");
			return detail::handle_impl().exchange(new_handler, std::memory_order_acq_rel);
		}

		inline handler_t get_handler() {
			return detail::handle_impl().load(std::memory_order_acquire);
		}

		inline policy set_policy(policy new_policy) {
			return detail::pol_impl().exchange(new_policy, std::memory_order_acq_rel);
		}

		inline policy get_policy() {
			return detail::pol_impl().load(std::memory_order_acquire);
		}

		inline void assert_impl(std::string_view message, handler_t handler, dp::contract::violation viol) {

			if(viol.message() == "") viol.append_message(message);
			handler(viol);
		}

		/*
		*  And, our primary assertion function
		*/
		constexpr inline void contract_assert(bool condition, std::string_view message, handler_t hand, dp::contract::violation viol = dp::contract::violation(DP_SOURCE_LOCATION_CURRENT, "")) {
			if (condition) return;

			//If you get errors here that it can't be a constant expression, odds are your assertion failed
			assert_impl(message, hand, viol);
		}

		constexpr inline void contract_assert(bool condition, std::string_view message, dp::contract::violation viol = dp::contract::violation(DP_SOURCE_LOCATION_CURRENT, "")) {
			
			if (condition) return;

			assert_impl(message, get_handler(), viol);
		}

		constexpr inline void contract_assert(bool condition, dp::contract::violation viol = dp::contract::violation(DP_SOURCE_LOCATION_CURRENT, "")) {
			if (condition) return;

			assert_impl("", get_handler(), viol);
		}

	}

	using contract::contract_assert;
}

/*
* For backwards compatibility reasons we continue to support the previous macros
*/


#ifndef DP_NO_CONTRACT_MACROS

#define CONTRACT_ASSERT3(cond, message, handler)	dp::contract::contract_assert(cond, message, handler, dp::contract::violation(DP_SOURCE_LOCATION_THIS_FUNCTION, message))
#define CONTRACT_ASSERT2(cond, message)				CONTRACT_ASSERT3(cond, message, dp::contract::get_handler())
#define CONTRACT_ASSERT1(cond)						CONTRACT_ASSERT2(cond, #cond)


#define CONTRACT_ASSERT(...)						DP_MACRO_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)
#endif


#endif