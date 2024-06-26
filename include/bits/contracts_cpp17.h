#ifndef DP_CPP17_CONTRACTS
#define DP_CPP17_CONTRACTS

#ifdef DP_CPP98_CONTRACTS
#error "Detected both C++98 and C++17 contracts in the same TU"
#endif

#include <iostream>
#include <string_view>
#include <type_traits>

#include "bits/source_location.h"
#include "bits/borland_compat_typedefs.h"

namespace dp {
	namespace contract {
		enum class policy {
			enforce,
			observe,
			ignore
		};

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

		/*
		*	Our "default handler" to determine the default behaviour of our contract violation handling
		*	We have to do some silly dances of if(!condition) to meet the constexpr specification 
		*/
		struct default_handler {

			constexpr inline void enforce(bool condition) {
				if (!condition) {
					throw violation_exception{ "Contract violation" };
				}
			}

			constexpr inline void enforce(bool condition, const dp::contract::violation& viol) {
				if (!condition) {
					throw violation_exception{ dp::compat::string{"Contract violation in function "} + viol.function() + " : " + dp::compat::string{viol.message()} };
				}
			}

			constexpr inline void observe(bool condition) {
				if (!condition) {
					std::cerr << "Contract violation\n";
				}
			}

			constexpr inline void observe(bool condition, const dp::contract::violation& viol) {
				if (!condition) {
					std::cerr << "Contract violation in function " << viol.function() << " : " << viol.message() << '\n';
				}
			}

			constexpr inline void ignore(bool) noexcept {}
			template<typename... Args>
			constexpr inline void ignore(Args&&...) noexcept {}
		};



		/*
		*  And, our primary assertion function
		*/
		template<policy pol = policy::enforce, typename handler = default_handler>
		constexpr inline  
		std::enable_if_t<!std::is_constructible_v<std::string_view, handler>, void> //Returns void, but we need to SFINAE to ensure that a message on the other overload doesn't try to resolve as handler
					contract_assert(bool condition, handler hand = default_handler{}, violation viol = violation{ DP_SOURCE_LOCATION_CURRENT, "" }) {
			if constexpr (pol == policy::enforce) {
				hand.enforce(condition, viol);
			}
			else if constexpr (pol == policy::observe) {
				hand.observe(condition, viol);
			}
			else {
				hand.ignore(condition, viol);
			}
		}

		template<policy pol = policy::enforce, typename handler = default_handler>
		constexpr inline void contract_assert(bool condition, std::string_view message, handler hand = default_handler{}, violation viol = violation{ DP_SOURCE_LOCATION_CURRENT, ""}) {
			viol.append_message(message);
			if constexpr (pol == policy::enforce) {
				hand.enforce(condition, viol);
			}
			else if constexpr (pol == policy::observe) {
				hand.observe(condition, viol);
			}
			else {
				hand.ignore(condition, viol);
			}
		}


	}
}

/*
* For backwards compatibility reasons we continue to support the previous macros
*/


//Hook to define the handler used by the predefined macros.
#ifndef DP_DEFAULT_HANDLER
#define DP_DEFAULT_HANDLER dp::contract::default_handler
#endif

#ifndef DP_DEFAULT_POLICY
#define DP_DEFAULT_POLICY dp::contract::policy::enforce
#endif


#define CONTRACT_ASSERT3(cond, message, handler)	dp::contract::contract_assert<dp::contract::policy:: DP_DEFAULT_POLICY>(cond, handler{}, dp::contract::violation{DP_SOURCE_LOC_HERE, message})
#define CONTRACT_ASSERT2(cond, message)				CONTRACT_ASSERT3(cond, message, DP_DEFAULT_HANDLER)
#define CONTRACT_ASSERT1(cond)						CONTRACT_ASSERT2(cond, #cond)


#define CONTRACT_ASSERT(...)						DP_MACRO_OVERLOAD(CONTRACT_ASSERT, __VA_ARGS__)



#endif