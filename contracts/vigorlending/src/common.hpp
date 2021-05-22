#pragma once
#if __INTELLISENSE__
#pragma diag_suppress 2486
#endif
// #define USE_SSTREAM
// #define USE_DEBUG_TIMERS

// This is somehow changing affecting the tests...need investigation
//#define DEBUG_TRACE
// #define DEBUG_OUTPUT_STACKCALLS
//#define TEST_MODE
#define LOCAL_MODE
//#define DEBUG

//  Lazy instanciation of false
template <typename>
inline constexpr bool invalid_type {false};

#define throw_static_error(error, message) static_assert (error, message);
#define throw_invalid_type(type) throw_static_error (invalid_type<type>, "Invalid type")