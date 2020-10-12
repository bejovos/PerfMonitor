//#pragma once // intentially missing guard
// ReSharper disable IdentifierTypo

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

//////////////////////////////////////////////////////////////////////////

#include "WriteToStream.h"
#include "StringToClass.h"
#include "BreakInDestructor.h"
#include "Indention.h"
#include "Counters.h"
#include "TimeAndMemoryWatcher.h"
#include "Callbacks.h"

/// @todo: add __LINE__ to funcsig, usecase: several anonimous staticcounter in the same function 
#define PM_STRING_TO_CLASS(string, class_name)                                                    \
  struct class_name {                                                                             \
    static constexpr const char* Str() {                                                          \
    return PerfMonitor::internal::strlen(string) == 0 ? __FUNCTION__ : string"::<" ; }     \
  }                                                                                      
#define PM_STRING_TO_LAMBDA(string)                                                               \
  [](){                                                                                           \
    PM_STRING_TO_CLASS(string, temporary);                                                        \
    return temporary{};                                                                           \
    }()
#define PM_CONCATENATE_IMPL(x, y) x ## y 
#define PM_CONCATENATE(x, y) PM_CONCATENATE_IMPL(x, y)
#define PM_INDENTION_NAME PM_CONCATENATE(indention_info, __COUNTER__)
#define PM_END_WITH_ELSE_IMPL(counter)                                                            \
  { goto PM_CONCATENATE(the_very_unique_label_name, counter); }                                   \
  else PM_CONCATENATE(the_very_unique_label_name, counter):                                        
#define PM_END_WITH_ELSE PM_END_WITH_ELSE_IMPL(__COUNTER__)                                                                 

// TIMER
#undef TIMER
#undef TIMER_INIT
#undef TIMER_SCOPED
#undef TIMERMEMORY
#undef TIMERMEMORY_INIT
#undef TIMERMEMORY_SCOPED
#if OPTIONS_TIMERMEMORY

/**
* @brief Timer. Prints elapsed time to cout in its destructor. Nested scope is indented.
* Usage examples:
* @code
* TIMER("Calculation")
*   {
*   DoSomething();
*   } @endcode
*/
#define TIMER(...) \
  if (const auto&& indention_info = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, false>>((  \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr))                                                                                                            \
  ) PM_END_WITH_ELSE

#define TIMER_INIT(...) \
  (PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), PerfMonitor::TimeAndMemoryWatcher<true, false>()) ? \
    PerfMonitor::internal::convertible_to_any{} :                                                                             \

#define TIMER_SCOPED(...) \
  const auto&& PM_INDENTION_NAME = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, false>>((       \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr))                                                                                                            

#define TIMERMEMORY(...) \
  if (const auto&& indention_info = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, true>>((   \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr))                                                                                                            \
  ) PM_END_WITH_ELSE

#define TIMERMEMORY_INIT(...) \
  (PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), PerfMonitor::TimeAndMemoryWatcher<true, true>()) ?  \
    PerfMonitor::internal::convertible_to_any{} :                                                                             \

#define TIMERMEMORY_SCOPED(...) \
  const auto&& PM_INDENTION_NAME = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, true>>((       \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr))                                                                                                            

#else
  #define TIMER(...) 
  #define TIMER_INIT(...) 
  #define TIMER_SCOPED(...) 
  #define TIMERMEMORY(...) 
  #define TIMERMEMORY_INIT(...) 
  #define TIMERMEMORY_SCOPED(...) 
#endif

// STATICCOUNTER
#undef STATICCOUNTER
#undef STATICCOUNTER_ADD
#undef STATICCOUNTER_GET
#undef STATICCOUNTER_SET
#if OPTIONS_COUNTERS

#define STATICCOUNTER(...)                                                                       \
  if ([](){                                                                                      \
    PerfMonitor::internal::MakeFromFancyString<PerfMonitor::StaticCounter>(                      \
    PM_STRING_TO_LAMBDA("" ## __VA_ARGS__)).Add(1);                                              \
    return false;                                                                                \
    }()                                                                                          \
  ) PM_END_WITH_ELSE

#define STATICCOUNTER_ADD(counter_name, value)                                                   \
  if ([](){                                                                                      \
    PerfMonitor::internal::MakeFromFancyString<PerfMonitor::StaticCounter>(                      \
    PM_STRING_TO_LAMBDA("" ## counter_name)).Add(value);                                         \
    return false;                                                                                \
    }()                                                                                          \
  ) PM_END_WITH_ELSE

// Not thread-safe
#define STATICCOUNTER_GET(counter_name)                                                      \
  PerfMonitor::internal::MakeFromFancyString<PerfMonitor::StaticCounter>(PM_STRING_TO_LAMBDA(counter_name)).GetTotal() 
// Not thread-safe
#define STATICCOUNTER_SET(counter_name, value)                                               \
  PerfMonitor::internal::MakeFromFancyString<PerfMonitor::StaticCounter>(PM_STRING_TO_LAMBDA(counter_name)).SetTotal(value) 
// Not thread-safe
#define STATICCOUNTER_RESET(counter_name)                                                    \
  INFO(## counter_name, STATICCOUNTER_GET(counter_name)) STATICCOUNTER_SET(counter_name, 0)

#else
  #define STATICCOUNTER(...)
  #define STATICCOUNTER_ADD(counter_name, value)
  #define STATICCOUNTER_GET(counter_name) size_t{0}
  #define STATICCOUNTER_SET(counter_name, value)
#endif

// TIMERSUM
#undef TIMERSUM_SCOPED
#undef TIMERSUM
#undef TIMERSUM_INIT
#undef TIMERSUM_GET
#undef TIMERSUM_SET
#if OPTIONS_TIMERMEMORY && OPTIONS_COUNTERS

#define TIMERSUM_SCOPED(...)                                                   \
  const auto& PM_INDENTION_NAME = PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__))

#define TIMERSUM(...)                                                          \
  if (const auto& indention_info = PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__ )) \
  ) PM_END_WITH_ELSE

#define TIMERSUM_INIT(...)                                                     \
  PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__ )) ? PerfMonitor::internal::convertible_to_any{} : 

// Not thread-safe
#define TIMERSUM_GET(counter_name)                                              \
  PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA(counter_name)).GetTotal() 
// Not thread-safe
#define TIMERSUM_SET(counter_name, microseconds)                                \
  PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(PM_STRING_TO_LAMBDA(counter_name)).SetTotal(microseconds) 
// Not thread-safe
#define TIMERSUM_RESET(counter_name)                                            \
  INFO(## counter_name, TIMERSUM_GET(counter_name)) TIMERSUM_SET(counter_name, std::chrono::microseconds{0})

#else
  #define TIMERSUM_SCOPED(...)                                                   
  #define TIMERSUM(...)                                                          
  #define TIMERSUM_INIT(...)                                                     
  #define TIMERSUM_GET(counter_name) std::chrono::microseconds{0}                                              
  #define TIMERSUM_SET(counter_name, microseconds)
#endif

// INFO
#undef INFO
#undef INFO_SCOPED
#if OPTIONS_INFO
#define INFO(...)                                                                                        \
  if (auto&& indendent_info = PerfMonitor::internal::MakeFromNothing<PerfMonitor::Indention::Indent>((   \
    [&](){                                                                                               \
    using namespace PerfMonitor;                                                                         \
    PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);                                \
    }(), nullptr))                                                                                       \
  ) PM_END_WITH_ELSE                                                                                 
#define INFO_SCOPED(...)                                                                                     \
  auto PM_INDENTION_NAME = PerfMonitor::internal::MakeFromNothing<PerfMonitor::Indention::Indent>(( \
    [&](){                                                                                                   \
    using namespace PerfMonitor;                                                                             \
    PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);                                    \
    }(), nullptr));                                                                                          

#else
  #define INFO(...)                                                                                        
  #define INFO_SCOPED(...)                                                                                     
#endif

// ASSERT
#undef PASSERT
#if OPTIONS_DEBUG

/**
* @brief Simple assert. Condition should be always true. Nested scope is executed BEFORE triggering debug break.
* Usage examples:
* @code
* PASSERT(i > 0) std::cout << i; @endcode
*/
#define PASSERT(...)                                                                                          \
  if (const auto&& indention_info =                                                                           \
    [&](){                                                                                                    \
      struct Result                                                                                           \
        {                                                                                                     \
          bool m_value;                                                                                       \
        ~Result()                                                                                             \
          {                                                                                                   \
          if (m_value == false)                                                                               \
            __debugbreak();                                                                                   \
          }                                                                                                   \
        operator bool() const                                                                                 \
          {                                                                                                   \
          return m_value;                                                                                     \
          }                                                                                                   \
        };                                                                                                    \
      return Result{__VA_ARGS__};                                                                             \
    }()                                                                                                       \
  ){} else

#else
  #define PASSERT(...) if(true \
  ) PM_END_WITH_ELSE
#endif

/**
* @brief Simple break. Breaks when condition is true. Nested scope is executed BEFORE triggering debug break.
* Usage examples:
* @code
* PBREAK(i > 0) std::cout << i; @endcode
*/
#define PBREAK(...) PASSERT(!(__VA_ARGS__))

// ReSharper restore IdentifierTypo
