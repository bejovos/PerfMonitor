#pragma once

#ifndef OPTIONS_INFO
  #define OPTIONS_INFO 1
#endif
#ifndef OPTIONS_WATCHERS
  #define OPTIONS_WATCHERS 1
#endif
#ifndef OPTIONS_COUNTERS
  #define OPTIONS_COUNTERS 1
#endif
#ifndef OPTIONS_DEBUG
  #define OPTIONS_DEBUG 1
#endif
#ifndef OPTIONS_PRECISEMEMORY
  #define OPTIONS_PRECISEMEMORY 1
#endif

#define CHECKE(value, expected) value
#define COLOR(color, ...) __VA_ARGS__
#define INFO(...)
#define TIMER_START(...)
#define TIMER_STOP()
#define TIMER(...)
#define MEMORY(...)
#define TIMERMEMORY
#define TIMERSUM_SCOPED(...)
#define TIMERSUM(...)
#define TIMERSUM_GET(...) std::chrono::microseconds{0}
#define TIMERSUM_SET(...) 
#define STATICCOUNTER(...)
#define STATICCOUNTER_GET(...) size_t{0}
#define STATICCOUNTER_SET(...)
#define PASSERT(...) if(true){}else


//////////////////////////////////////////////////////////////////////////

#include "WriteToStream.h"
#include "StringToClass.h"
#include "BreakInDestructor.h"
#include "Indention.h"
#include "Counters.h"

// TIMER
#if OPTIONS_WATCHERS
#undef TIMER_START
#undef TIMER_STOP
#undef TIMER
#undef MEMORY
#undef TIMERMEMORY

#define TIMER_START(...) auto temporary_timer_1 = (                 \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), \
  PerfMonitor::TimeAndMemoryWatcher<true, false, false>())
#define TIMER_STOP() temporary_timer_1.~TimeAndMemoryWatcher()

/**
* @rief Timer. Prints elapsed time to cout in its destructor. Nested scope is indented.
* Usage examples:
* @code
* TIMER("Calculation")
*   {
*   DoSomething();
*   } @endcode
*/
#define TIMER(...) if (auto indendent_info = (                     \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), \
  PerfMonitor::TimeAndMemoryWatcher<true, false, false>()) ){}else
#define MEMORY(...) if (auto indendent_info = (                    \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), \
  PerfMonitor::TimeAndMemoryWatcher<false, true, OPTIONS_PRECISEMEMORY>()) ){}else
#define TIMERMEMORY(...) if (auto indendent_info = (               \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), \
  PerfMonitor::TimeAndMemoryWatcher<true, true, OPTIONS_PRECISEMEMORY>()) ){}else
#endif

// STATICCOUNTER
#if OPTIONS_COUNTERS
#undef STATICCOUNTER
#undef STATICCOUNTER_GET
#undef STATICCOUNTER_SET

#define PM_CONCATENATE(x, y) x ## y
#define PM_QUOTE(x) PM_QUOTE2(x)
#define PM_QUOTE2(x) # x
#define PM_FILE_LINE PM_CONCATENATE(__FILE__, ":") ## PM_QUOTE(__LINE__)
#define PM_ANONYMOUS_STRING(string) ((PerfMonitor::internal::c_strlen(string) == 0) ? PM_FILE_LINE : string)
/**
 * @brief Thread-safe static counter. By default each call adds 1 to the internal counter.
 * Usage examples:
 * @code
 * STATICCOUNTER("Name"); @endcode
 */
#define STATICCOUNTER(...)                                                                       \
  ([&](){                                                                                        \
    STRING_TO_CLASS(PM_ANONYMOUS_STRING("" ## __VA_ARGS__), string_in_class);                    \
    ASM_IncrementCounter(PerfMonitor::CounterInitialization<2, string_in_class>::id.GetOffset());\
  })()

#define STATICCOUNTER_GET(counter_name)                                                      \
  PerfMonitor::StaticCounter::GetTotalValue([&](){                                           \
    STRING_TO_CLASS(counter_name, string_in_class);                                          \
    return PerfMonitor::CounterInitialization<2, string_in_class>::id.GetId();               \
  }())

#define STATICCOUNTER_SET(counter_name, value)                                               \
  PerfMonitor::StaticCounter::SetTotalValue([&](){                                           \
    STRING_TO_CLASS(counter_name, string_in_class);                                          \
    return PerfMonitor::CounterInitialization<2, string_in_class>::id.GetId();               \
  }(), value)

#endif

// TIMERSUM
#if OPTIONS_WATCHERS && OPTIONS_COUNTERS
#undef TIMERSUM
#undef TIMERSUM_SCOPED
#undef TIMERSUM_GET
#undef TIMERSUM_SET

#define TIMERSUM_SCOPED(string)                                                      \
PerfMonitor::TimerSum temporary_timer_1([]() -> size_t {                             \
  STRING_TO_CLASS(string, string_in_class);                                          \
  return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetOffset();     \
}())


#define TIMERSUM(string)                                                             \
if (auto indendent_info = std::move(PerfMonitor::TimerSum( []() -> size_t {          \
  STRING_TO_CLASS(string, string_in_class);                                          \
  return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetOffset();     \
}() )) ){} else

#define TIMERSUM_GET(counter_name)                                              \
PerfMonitor::TimerSum::GetTotalValue([]() -> size_t {                           \
  STRING_TO_CLASS(counter_name, string_in_class);                               \
  return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetId();    \
}())

#define TIMERSUM_SET(counter_name, microseconds)                                \
PerfMonitor::TimerSum::SetTotalValue([]() -> size_t {                           \
  STRING_TO_CLASS(counter_name, string_in_class);                               \
  return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetId();    \
}(), microseconds)


#endif 

// CHECKE INFO
#if OPTIONS_INFO
#undef CHECKE
#undef COLOR
#undef INFO

#define CHECKE(value, expected) PerfMonitor::CheckEqual(value, expected)
#define COLOR(color, ...) PerfMonitor::MakeColoredValue(color, __VA_ARGS__)
#define INFO(...)                                                          \
if (auto indendent_info = [&]()                                            \
  {                                                                        \
  using namespace PerfMonitor;                                             \
  PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);    \
  return PerfMonitor::Indention::Indent();                                 \
  }()){}else                                                               
#endif

// _ASSERT
#if OPTIONS_DEBUG
#undef PASSERT

/**
* @brief Simple assert. Condition should be always true. Nested scope is executed BEFORE triggering debug break.
* Usage examples:
* @code
* PASSERT(i > 0) std::cout << i; @endcode
*/
#define PASSERT(...) if (auto indendent_info = PerfMonitor::BreakInDestructor(__VA_ARGS__)){}else 
#endif

/**
* @brief Simple break. Breaks when condition is true. Nested scope is executed BEFORE triggering debug break.
* Usage examples:
* @code
* PBREAK(i > 0) std::cout << i; @endcode
*/
#define PBREAK(...) PASSERT(!(__VA_ARGS__))

//#pragma warning (disable:4189)
//#pragma warning (disable: 4390)
#pragma warning (disable:4100) // unreferenced formal parameter
