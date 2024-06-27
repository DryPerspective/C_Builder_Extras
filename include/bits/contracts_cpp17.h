#ifndef DP_CPP17_CONTRACTS
#define DP_CPP17_CONTRACTS

#ifdef DP_CPP98_CONTRACTS
#error "Detected both C++98 and C++17 contracts in the same TU"
#endif

#include <iostream>
#include <string_view>
#include <type_traits>
#include <fstream>


#include "source_location.h"
#include "bits/borland_compat_typedefs.h"

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

		class violation {
			dp::source_location loc;
			std::string_view msg;

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

			constexpr void append_message(std::string_view in_msg) {
				msg = in_msg;
			}
		};

		class violation_exception : public std::runtime_error {
		public:
			violation_exception(const std::string& msg) : std::runtime_error{ msg } {}
		};

		dp::compat::string default_message(const violation& in) {
			return dp::compat::string{ "Contract violation in function " } + in.function() + " : " + dp::compat::string{ in.message() };
		}


		/*
		*	Our "default handler" to determine the default behaviour of our contract violation handling
		*/
		struct default_handler {


			[[noreturn]] inline void enforce(const dp::contract::violation& viol) {
				throw violation_exception{ default_message(viol)};
			}

			inline void observe(const dp::contract::violation& viol) {
				std::ofstream out("Contract violations.log", std::ios_base::out | std::ios_base::app);
				out << default_message(viol) << '\n';
			}

			template<typename... Args>
			constexpr inline void ignore(Args&&...) noexcept {}
		};



		/*
		*  And, our primary assertion function
		*/
		template<policy pol = policy::enforce, typename handler = default_handler, typename... Args>
		constexpr inline  
		std::enable_if_t<!std::is_constructible_v<std::string_view, handler>, void> //Returns void, but we need to SFINAE to ensure that a message on the other overload doesn't try to resolve as handler
					contract_assert(bool condition, handler hand = default_handler{}, violation viol = violation{ DP_SOURCE_LOCATION_CURRENT, "" }, Args&&... args) {
			if (condition) return;

			//If you get an error that this did not evaluate to constant or could not be used in a constant expression, then odds are your assertion failed.
			if constexpr (pol == policy::enforce) {
				hand.enforce(viol, std::forward<Args>(args)...);
			}
			else if constexpr (pol == policy::observe) {
				hand.observe(viol, std::forward<Args>(args)...);
			}
			else {
				hand.ignore(viol, std::forward<Args>(args)...);
			}
		}

		template<policy pol = policy::enforce, typename handler = default_handler, typename... Args>
		constexpr inline void contract_assert(bool condition, std::string_view message, handler hand = default_handler{}, violation viol = violation{ DP_SOURCE_LOCATION_CURRENT, ""}, Args&&... args) {
			
			if (condition) return;

			//If you get an error that this did not evaluate to constant or could not be used in a constant expression, then odds are your assertion failed.
			viol.append_message(message);
			if constexpr (pol == policy::enforce) {
				hand.enforce(viol, std::forward<Args>(args)...);
			}
			else if constexpr (pol == policy::observe) {
				hand.observe(viol, std::forward<Args>(args)...);
			}
			else {
				hand.ignore(viol, std::forward<Args>(args)...);
			}
		}

		template<typename handler = default_handler, typename... Args>
		constexpr inline 
			std::enable_if_t<!std::is_constructible_v<std::string_view, handler>, void> //Returns void, but we need to SFINAE to ensure that a message on the other overload doesn't try to resolve as handler			
				contract_assert(dp::contract::policy pol, bool condition, handler hand = default_handler{}, violation viol = violation{ DP_SOURCE_LOCATION_CURRENT, "" }, Args&&... args) {
			if (condition) return;

			if (pol == policy::enforce) {
				hand.enforce(viol, std::forward<Args>(args)...);
			}
			else if (pol == policy::observe) {
				hand.observe(viol, std::forward<Args>(args)...);
			}
			else {
				hand.ignore(viol, std::forward<Args>(args)...);
			}
		}

		template<typename handler = default_handler, typename... Args>
		constexpr inline void contract_assert(dp::contract::policy pol, bool condition, std::string_view message, handler hand = default_handler{}, violation viol = violation{ DP_SOURCE_LOCATION_CURRENT, "" }, Args&&... args) {
			if (condition) return;

			viol.append_message(message);

			if (pol == policy::enforce) {
				hand.enforce(viol, std::forward<Args>(args)...);
			}
			else if (pol == policy::observe) {
				hand.observe(viol, std::forward<Args>(args)...);
			}
			else {
				hand.ignore(viol, std::forward<Args>(args)...);
			}
		}


	}

	using contract::contract_assert;
}

/*
* For backwards compatibility reasons we continue to support the previous macros
*/


#ifndef DP_NO_CONTRACT_MACROS
//Hook to define the handler used by the predefined macros.
#ifndef DP_DEFAULT_HANDLER
#define DP_DEFAULT_HANDLER dp::contract::default_handler
#endif

#ifndef DP_DEFAULT_POLICY
#define DP_DEFAULT_POLICY enforce
#endif


#define CONTRACT_ASSERT3(cond, message, handler)	dp::contract::contract_assert<dp::contract::policy:: DP_DEFAULT_POLICY>(cond, handler{}, dp::contract::violation{DP_SOURCE_LOC_HERE, message})
#define CONTRACT_ASSERT2(cond, message)				CONTRACT_ASSERT3(cond, message, DP_DEFAULT_HANDLER)
#define CONTRACT_ASSERT1(cond)						CONTRACT_ASSERT2(cond, #cond)


#define CONTRACT_ASSERT(...)						DP_MACRO_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)
#endif


#endif