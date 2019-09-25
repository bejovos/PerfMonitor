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
#undef TIMER_OBJ
#undef TIMERMEMORY_OBJ
#undef TIMER
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
#define TIMER_OBJ(...) 0
#define TIMERMEMORY_OBJ(...) 0
#define TIMER(...)
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
#include "Callbacks.h"

// TIMER
#if OPTIONS_TIMERMEMORY
#undef TIMER_OBJ
#undef TIMERMEMORY_OBJ
#undef TIMER
#undef TIMERMEMORY

// #define TIMER_OBJ(...) (                                                  \
//   [&](){                                                                  \
//   using namespace PerfMonitor;                                            \
//   PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);  \
//   }(),                                                                    \
//   PerfMonitor::TimeAndMemoryWatcher<true, false>(nullptr))
// #define TIMERMEMORY_OBJ(...) (                                            \
//   [&](){                                                                  \
//   using namespace PerfMonitor;                                            \
//   PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);  \
//   }(),                                                                    \
//   PerfMonitor::TimeAndMemoryWatcher<true, true>(nullptr))

/**
* @brief Timer. Prints elapsed time to cout in its destructor. Nested scope is indented.
* Usage examples:
* @code
* TIMER("Calculation")
*   {
*   DoSomething();
*   } @endcode
*/
#define TIMER(...) if (PerfMonitor::TimeAndMemoryWatcher<true, false> indendent_info = (\
  [&](){                                                                                \
  using namespace PerfMonitor;                                                          \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                \
  }(),                                                                                  \
  nullptr) ){}else
#define TIMERMEMORY(...) if (PerfMonitor::TimeAndMemoryWatcher<true, true> indendent_info = ( \
  [&](){                                                                                      \
  using namespace PerfMonitor;                                                                \
  PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                      \
  }(),                                                                                        \
  nullptr) ){}else
#endif

// STATICCOUNTER
#if OPTIONS_COUNTERS
#undef STATICCOUNTER
#undef STATICCOUNTER_ADD
#undef STATICCOUNTER_GET
#undef STATICCOUNTER_SET
#undef STATICCOUNTER_RESET

#define PM_STRING_TO_CLASS(string, class_name)                                                    \
  struct class_name {                                                                             \
    static constexpr const char* Str() {                                                          \
    return PerfMonitor::internal::strlen(string) == 0 ? __FUNCSIG__ : "cdecl "string "::<"; }     \
  }                                                                                      
#define PM_STRING_TO_LAMBDA(string)                                                               \
  [&](){                                                                                          \
    PM_STRING_TO_CLASS(string, temporary);                                                        \
    return temporary{};                                                                           \
    }()

#define STATICCOUNTER(...)                                                                        \
  if (const auto& indention_info = PerfMonitor::internal::MakeFromFancyString<PerfMonitor::StaticCounter>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__))) {}else

// #define STATICCOUNTER_ADD(counter_name, value)                                                   \
//   if ([&](){                                                                                     \
//     PM_MAKE_STRING_IN_CLASS("" ## counter_name, string_in_class);                                \
//     ASM_IncrementCounter2(PerfMonitor::CounterInitialization<2, string_in_class>::id.GetOffset(), value);\
//     return false;                                                                                \
//     }()){}else
// Not thread-safe
// #define STATICCOUNTER_GET(counter_name)                                                      \
//   PerfMonitor::StaticCounter::GetTotalValue([&](){                                           \
//     PM_MAKE_STRING_IN_CLASS(counter_name, string_in_class);                                          \
//     return PerfMonitor::CounterInitialization<2, string_in_class>::id.GetId();               \
//   }())
// Not thread-safe
// #define STATICCOUNTER_SET(counter_name, value)                                               \
//   PerfMonitor::StaticCounter::SetTotalValue([&](){                                           \
//     PM_MAKE_STRING_IN_CLASS(counter_name, string_in_class);                                          \
//     return PerfMonitor::CounterInitialization<2, string_in_class>::id.GetId();               \
//   }(), value)
// Not thread-safe
// #define STATICCOUNTER_RESET(...)                          \
//   if ([&](){                                              \
//     INFO(__VA_ARGS__, STATICCOUNTER_GET(__VA_ARGS__));    \
//     STATICCOUNTER_SET(__VA_ARGS__, 0);                    \
//     return false;                                         \
//     }()){}else

#endif

// TIMERSUM
#if OPTIONS_TIMERMEMORY && OPTIONS_COUNTERS
#undef TIMERSUM_SCOPED
#undef TIMERSUM
#undef TIMERSUM_GET
#undef TIMERSUM_SET

#define TIMERSUM_SCOPED(...)                                                   \
  const auto& indention_info = PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__));
#define TIMERSUM(...)                                                          \
  if (const auto& indention_info = PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__ ))) {}else
#define TIMERSUM_INIT(...)                                                     \
  PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__ )) ? PerfMonitor::internal::convertible_to_any{} : 

// Not thread-safe
// #define TIMERSUM_GET(counter_name)                                              \
// PerfMonitor::TimerSum::GetTotalValue([]() -> size_t {                           \
//   PM_MAKE_STRING_IN_CLASS(counter_name, string_in_class);                               \
//   return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetId();    \
// }())
// Not thread-safe
// #define TIMERSUM_SET(counter_name, microseconds)                                \
// PerfMonitor::TimerSum::SetTotalValue([]() -> size_t {                           \
//   PM_MAKE_STRING_IN_CLASS(counter_name, string_in_class);                               \
//   return PerfMonitor::CounterInitialization<0, string_in_class>::id.GetId();    \
// }(), microseconds)
// Not thread-safe
// #define TIMERSUM_RESET(counter_name)                                            \
//   if ([&](){                                                                    \
//     INFO(counter_name, TIMERSUM_GET(counter_name));                             \
//     TIMERSUM_SET(counter_name, std::chrono::microseconds{0});                   \
//     return false;                                                               \
//     }()){}else

#endif 

// INFO
#if OPTIONS_INFO
#undef COLOR
#undef INFO
#undef INFO_SCOPED

#define COLOR(color, ...) PerfMonitor::MakeColoredValue(color, __VA_ARGS__)
#define INFO(...)                                                          \
if (PerfMonitor::Indention::Indent indendent_info = ([&]()                 \
  {                                                                        \
  using namespace PerfMonitor;                                             \
  PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);    \
  }(), nullptr)){}else                                                               
#define INFO_SCOPED(...)                                                   \
PerfMonitor::Indention::Indent temporary_indendent_info_1 = ([&]()         \
  {                                                                        \
  using namespace PerfMonitor;                                             \
  PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);    \
  }(), nullptr);

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

