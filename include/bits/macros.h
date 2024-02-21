#ifndef DP_BITS_MACROS
#define DP_BITS_MACROS

#define DP_CONCAT_IMPL(x,y) x##y
#define DP_CONCAT( x, y ) DP_CONCAT_IMPL( x, y )

#ifdef __COUNTER__
#define DP_COUNT __COUNTER__
#else
#define DP_COUNT __LINE__
#endif

#define DP_UNIQUE_NAME(arg) DP_CONCAT(arg, DP_COUNT)

//Try to get a cross platform call to __FUNCTION__
//In Borland land it's a macro so we have to do a bit of preprocessor for that
#ifdef __BORLANDC__
#define DP_FUNC __FUNC__
#elif defined (_MSC_VER)
#define DP_FUNC __func__
#elif defined(__GNUC__)
#define DP_FUNC __FUNCTION__
#else
#error "No function handle found"
#endif

//MSVC has a bug in expanding variadic macros
#ifdef _MSC_VER
#define DP_GLUE(x, y) x y
#define DP_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, count, ...) count
#define DP_EXPAND_ARGS(args) DP_NUM_ARGS_IMPL args
#define DP_NUM_ARGS(...) DP_EXPAND_ARGS((__VA_ARGS__, 6, 5, 4, 3, 2, 1, 0))
#define DP_MACRO_OVERLOAD_IMPL2(name, count) name##count
#define DP_MACRO_OVERLOAD_IMPL1(name, count) DP_MACRO_OVERLOAD_IMPL2(name, count)
#define DP_MACRO_OVERLOAD_IMPL(name, count) DP_MACRO_OVERLOAD_IMPL1(name, count)
#define DP_MACRO_OVERLOAD(name, ...) DP_GLUE(DP_MACRO_OVERLOAD_IMPL(name, DP_NUM_ARGS(__VA_ARGS__)), (__VA_ARGS__))
#elif defined(DP_CBUILDER10) || __cplusplus >= 201103L
//But other compilers do not
#define DP_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, count, ...) count
#define DP_NUM_ARGS(...) DP_NUM_ARGS_IMPL(__VA_ARGS__, 6, 5, 4, 3, 2, 1)
#define DP_MACRO_OVERLOAD(MACRO, ...) DP_CONCAT(MACRO, DP_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)
#endif



#endif