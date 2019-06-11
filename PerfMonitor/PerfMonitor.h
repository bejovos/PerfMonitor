//#pragma once // intentially missing guard

#ifndef OPTIONS_INFO
  #define OPTIONS_INFO 1
#endif
#ifndef OPTIONS_TIMERMEMORY
  #define OPTIONS_TIMERMEMORY 1
#endif
#ifndef OPTIONS_COUNTERS
  #define OPTIONS_COUNTERS 1
#endif
#ifndef OPTIONS_DEBUG
  #define OPTIONS_DEBUG 1
#endif

#undef COLOR
#undef INFO
#undef INFO_SCOPED
#undef TIMER_START
#undef TIMER_STOP
#undef TIMERMEMORY_START
#undef TIMERMEMORY_STOP
#undef TIMER
#undef MEMORY
#undef TIMERMEMORY
#undef TIMERSUM_SCOPED
#undef TIMERSUM
#undef TIMERSUM_GET
#undef TIMERSUM_SET
#undef STATICCOUNTER
#undef STATICCOUNTER_GET
#undef STATICCOUNTER_SET
#undef STATICCOUNTER_RESET
#undef PASSERT

#define COLOR(color, ...) __VA_ARGS__
#define INFO(...)
#define INFO_SCOPED(...)
#define TIMER_START(...)
#define TIMER_STOP()
#define TIMERMEMORY_START(...)
#define TIMERMEMORY_STOP()
#define TIMER(...)
#define MEMORY(...)
#define TIMERMEMORY(...)
#define TIMERSUM_SCOPED(...)
#define TIMERSUM(...)
#define TIMERSUM_GET(...) std::chrono::microseconds{0}
#define TIMERSUM_SET(...) 
#define STATICCOUNTER(...)
#define STATICCOUNTER_GET(...) size_t{0}
#define STATICCOUNTER_SET(...)
#define STATICCOUNTER_RESET(...)
#define PASSERT(...) if(true){}else


//////////////////////////////////////////////////////////////////////////

#include "WriteToStream.h"
#include "StringToClass.h"
#include "BreakInDestructor.h"
#include "Indention.h"
#include "Counters.h"
#include "TimeAndMemoryWatcher.h"

// TIMER
#if OPTIONS_TIMERMEMORY
#undef TIMERMEMORY_START
#undef TIMER_START
#undef TIMER_STOP
#undef TIMERMEMORY_STOP
#undef TIMER
#undef MEMORY
#undef TIMERMEMORY


#define TIMERMEMORY_START(...) auto temporary_timer_1 = (                 \
  [&](){                                                                  \
  using namespace PerfMonitor;                                            \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);  \
  }(),                                                                    \
  PerfMonitor::TimeAndMemoryWatcher<true, true>())
#define TIMER_START(...) auto temporary_timer_1 = (                       \
  [&](){                                                                  \
  using namespace PerfMonitor;                                            \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);  \
  }(),                                                                    \
  PerfMonitor::TimeAndMemoryWatcher<true, false>())
#define TIMER_STOP() temporary_timer_1.~TimeAndMemoryWatcher()
#define TIMERMEMORY_STOP() temporary_timer_1.~TimeAndMemoryWatcher()

/**
* @brief Timer. Prints elapsed time to cout in its destructor. Nested scope is indented.
* Usage examples:
* @code
* TIMER("Calculation")
*   {
*   DoSomething();
*   } @endcode
*/
#define TIMER(...) if (auto indendent_info = (                            \
  [&](){                                                                  \
  using namespace PerfMonitor;                                            \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);  \
  }(),                                                                    \
  PerfMonitor::TimeAndMemoryWatcher<true, false>()) ){}else
#define MEMORY(...) if (auto indendent_info = (                           \
  [&](){                                                                  \
  using namespace PerfMonitor;                                            \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);  \
  }(),                                                                    \
  PerfMonitor::TimeAndMemoryWatcher<false, true>()) ){}else
#define TIMERMEMORY(...) if (auto indendent_info = (                      \
  [&](){                                                                  \
  using namespace PerfMonitor;                                            \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);  \
  }(),                                                                    \
  PerfMonitor::TimeAndMemoryWatcher<true, true>()) ){}else
#endif

// STATICCOUNTER
#if OPTIONS_COUNTERS
#undef STATICCOUNTER
#undef STATICCOUNTER_ADD
#undef STATICCOUNTER_GET
#undef STATICCOUNTER_SET
#undef STATICCOUNTER_RESET

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
  if ([&](){                                                                                     \
    STRING_TO_CLASS(PM_ANONYMOUS_STRING("" ## __VA_ARGS__), string_in_class);                    \
    ASM_IncrementCounter(PerfMonitor::CounterInitialization<2, string_in_class>::id.GetOffset());\
    return false;                                                                                \
    }()){}else
/**
 * @brief Thread-safe static counter. Each call adds given number to the internal counter.
 * Usage examples:
 * @code
 * STATICCOUNTER_ADD("Name", 1); @endcode
 */
#define STATICCOUNTER_ADD(counter_name, value)                                                   \
  if ([&](){                                                                                     \
    STRING_TO_CLASS(PM_ANONYMOUS_STRING("" ## counter_name), string_in_class);                   \
    ASM_IncrementCounter2(PerfMonitor::CounterInitialization<2, string_in_class>::id.GetOffset(), value);\
    return false;                                                                                \
    }()){}else
// Not thread-safe
#define STATICCOUNTER_GET(counter_name)                                                      \
  PerfMonitor::StaticCounter::GetTotalValue([&](){                                           \
    STRING_TO_CLASS(counter_name, string_in_class);                                          \
    return PerfMonitor::CounterInitialization<2, string_in_class>::id.GetId();               \
  }())
// Not thread-safe
#define STATICCOUNTER_SET(counter_name, value)                                               \
  PerfMonitor::StaticCounter::SetTotalValue([&](){                                           \
    STRING_TO_CLASS(counter_name, string_in_class);                                          \
    return PerfMonitor::CounterInitialization<2, string_in_class>::id.GetId();               \
  }(), value)
// Not thread-safe
#define STATICCOUNTER_RESET(...)                          \
  if ([&](){                                              \
    INFO(__VA_ARGS__, STATICCOUNTER_GET(__VA_ARGS__));    \
    STATICCOUNTER_SET(__VA_ARGS__, 0);                    \
    return false;                                         \
    }()){}else

#endif

// TIMERSUM
#if OPTIONS_TIMERMEMORY && OPTIONS_COUNTERS
#undef TIMERSUM_SCOPED
#undef TIMERSUM
#undef TIMERSUM_GET
#undef TIMERSUM_SET

#define TIMERSUM_SCOPED(...)                                                         \
PerfMonitor::TimerSum temporary_timer_1([]() -> size_t {                             \
  STRING_TO_CLASS(PM_ANONYMOUS_STRING("" ## __VA_ARGS__), string_in_class);          \
  return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetOffset();     \
}())


#define TIMERSUM(...)                                                                \
if (auto indendent_info = PerfMonitor::TimerSum( []() -> size_t {                    \
  STRING_TO_CLASS(PM_ANONYMOUS_STRING("" ## __VA_ARGS__), string_in_class);          \
  return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetOffset();     \
}() ) ){} else

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
#undef COLOR
#undef INFO
#undef INFO_SCOPED

#define COLOR(color, ...) PerfMonitor::MakeColoredValue(color, __VA_ARGS__)
#define INFO(...)                                                          \
if (auto indendent_info = [&]()                                            \
  {                                                                        \
  using namespace PerfMonitor;                                             \
  PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);    \
  return PerfMonitor::Indention::Indent();                                 \
  }()){}else                                                               
#define INFO_SCOPED(...)                                                   \
auto temporary_indendent_info_1 = [&]()                                    \
  {                                                                        \
  using namespace PerfMonitor;                                             \
  PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);    \
  return PerfMonitor::Indention::Indent();                                 \
  }();

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

