#ifndef DP_CPP98_SUPPORT_TOOLS
#define DP_CPP98_SUPPORT_TOOLS

//Rather than reinvent the type_traits wheel in every header that requires it
//Or add my cpp98 lib as a hard requirement, we provide a quick example of what we need.
namespace dp {

    namespace support {


        template<typename T, T v>
        struct integral_constant {
            static const T value = v;
            typedef T value_type;
            typedef integral_constant type;
            operator value_type() const { return value; }
            value_type operator()() const { return value; }

        };

        typedef integral_constant<bool, true> true_type;
        typedef integral_constant<bool, false> false_type;


        template<bool b>
        struct bool_constant : true_type {};

        template<>
        struct bool_constant<false> : false_type {};


        template<typename T>
        struct remove_cv {
            typedef T type;
        };
        template<typename T>
        struct remove_cv<const T> {
            typedef T type;
        };
        template<typename T>
        struct remove_cv<volatile T> {
            typedef T type;
        };
        template<typename T>
        struct remove_cv<const volatile T> {
            typedef T type;
        };

        template<typename T, typename U>
        struct is_same : false_type {};
        template<typename T>
        struct is_same<T, T> : true_type {};


        template<typename T>
        struct is_integral : integral_constant<bool,
            is_same<typename remove_cv<T>::type, bool>::value ||
            is_same<typename remove_cv<T>::type, signed char>::value ||
            is_same<typename remove_cv<T>::type, unsigned char>::value ||
            is_same<typename remove_cv<T>::type, signed short>::value ||
            is_same<typename remove_cv<T>::type, unsigned short>::value ||
            is_same<typename remove_cv<T>::type, signed int>::value ||
            is_same<typename remove_cv<T>::type, unsigned int>::value ||
            is_same<typename remove_cv<T>::type, signed long>::value ||
            is_same<typename remove_cv<T>::type, unsigned long>::value ||
            is_same<typename remove_cv<T>::type, wchar_t>::value> {};

        template<typename T>
        struct is_floating_point : integral_constant<bool,
            is_same<typename remove_cv<T>::type, float>::value ||
            is_same<typename remove_cv<T>::type, double>::value ||
            is_same<typename remove_cv<T>::type, long double>::value> {};


        template<bool b, typename T = void>
        struct enable_if {};

        template<typename T>
        struct enable_if<true, T> {
            typedef T type;
        };



        template<typename T>
        struct is_arithmetic : integral_constant<bool,
            is_integral<T>::value ||
            is_floating_point<T>::value> {};

        namespace detail
        {
            template<typename T, bool = is_arithmetic<T>::value>
            struct is_signed : integral_constant<bool, T(-1) < T(0) > {};
            template<typename T>
            struct is_signed<T, false> : false_type {};
        }
        template<typename T>
        struct is_signed : detail::is_signed<T>::type {};
    }

}




#endif