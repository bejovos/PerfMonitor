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

#define TO_STRING_IMPL(x) # x
#define TO_STRING(x) TO_STRING_IMPL(x)
#define PM_CONCATENATE_IMPL(x, y) x ## y 
#define PM_CONCATENATE(x, y) PM_CONCATENATE_IMPL(x, y)

#define PM_STRING_TO_CLASS(string, class_name)                                                    \
struct class_name {                                                                               \
  static constexpr const char* Str() { return string; }                                           \
  static constexpr const char* File() { return __FILE__; }                                        \
  static constexpr const char* Function() { return __FUNCTION__; }                                \
  static constexpr const char* Line() { return "(" TO_STRING(__LINE__) ")"; }                     \
}                                                                                                

#define PM_STRING_TO_LAMBDA(string)                                                               \
  [](){                                                                                           \
    PM_STRING_TO_CLASS(string, temporary);                                                        \
    return temporary{};                                                                           \
    }()
#define PM_INDENTION_NAME PM_CONCATENATE(indention_info, __COUNTER__)

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
  if (const auto&& indention_info = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, false, false>>((  \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr)); false                                                                                                     \
  ) {} else

#define TIMER_INIT(...) \
  (PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), PerfMonitor::TimeAndMemoryWatcher<true, false, false>()) ? \
    PerfMonitor::internal::convertible_to_any{} :                                                                             \

#define TIMER_SCOPED(...) \
  const auto&& PM_INDENTION_NAME = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, false, false>>((   \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr))                                                                                                            

#define TIMERMEMORY(...) \
  if (const auto&& indention_info = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, true, true>>((   \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr)); false                                                                                                     \
  ) {} else

#define TIMERPEAK(...) \
  if (const auto&& indention_info = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, false, true>>((   \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr)); false                                                                                                     \
  ) {} else

#define TIMERMEMORY_INIT(...) \
  (PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__), PerfMonitor::TimeAndMemoryWatcher<true, true, true>()) ?  \
    PerfMonitor::internal::convertible_to_any{} :                                                                             \

#define TIMERMEMORY_SCOPED(...) \
  const auto&& PM_INDENTION_NAME = PerfMonitor::internal::MakeFromNothing<PerfMonitor::TimeAndMemoryWatcher<true, true, true>>((    \
    [&](){                                                                                                                    \
    using namespace PerfMonitor;                                                                                              \
    PerfMonitor::PrintIfArgsEmpty(false, __FILE__, __LINE__, __VA_ARGS__);                                                    \
    }(), nullptr))                                                                                                            

#else
  #define TIMER(...) 
  #define TIMER_INIT(...) 
  #define TIMER_SCOPED(...) 
  #define TIMERMEMORY(...) 
  #define TIMERPEAK(...)
  #define TIMERMEMORY_INIT(...) 
  #define TIMERMEMORY_SCOPED(...) 
#endif

// STATICCOUNTER
#undef STATICCOUNTER
#undef STATICCOUNTER_ADD
#undef STATICCOUNTER_GET
#undef STATICCOUNTER_SET
#if OPTIONS_COUNTERS

#define STATICCOUNTER_ADD(counter_name, value)                                                   \
  if (                                                                                           \
    PerfMonitor::internal::MakeFromFancyString<PerfMonitor::StaticCounter>(                      \
    PM_STRING_TO_LAMBDA(counter_name)).Add(value), false                                         \
  ) {} else

#define STATICCOUNTER(...) STATICCOUNTER_ADD("" ## __VA_ARGS__, 1)

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
  #define STATICCOUNTER(...) (void)0
  #define STATICCOUNTER_ADD(counter_name, value) (void)0
  #define STATICCOUNTER_GET(counter_name) size_t{0}
  #define STATICCOUNTER_SET(counter_name, value) (void)0
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

#define TIMERSUM(...)                                                                                     \
  if (const auto& PM_INDENTION_NAME = PerfMonitor::internal::MakeFromFancyString<PerfMonitor::TimerSum>(  \
    PM_STRING_TO_LAMBDA("" ## __VA_ARGS__ )); false                                                       \
  ) {} else

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
#undef AT_EXIT
#undef INFO
#undef INFO_SCOPED
#if OPTIONS_INFO
#define AT_EXIT(...)                            \
  if (const auto& PM_INDENTION_NAME = [&]() {   \
    auto l = [&](){ __VA_ARGS__; };             \
    struct Temporary                            \
    {                                           \
      decltype(l) ll;                           \
      ~Temporary() { ll(); }                    \
    };                                          \
    return Temporary{l};                        \
  }(); true )                                         
#define INFO(...)                                                                                        \
  if (auto&& indendent_info = PerfMonitor::internal::MakeFromNothing<PerfMonitor::Indention::Indent>((   \
    [&](){                                                                                               \
    using namespace PerfMonitor;                                                                         \
    PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);                                \
    }(), nullptr)); false                                                                                \
  ) {} else                                                                                 
#define INFO_SCOPED(...)                                                                                     \
  auto PM_INDENTION_NAME = PerfMonitor::internal::MakeFromNothing<PerfMonitor::Indention::Indent>((          \
    [&](){                                                                                                   \
    using namespace PerfMonitor;                                                                             \
    PerfMonitor::PrintIfArgsEmpty(true, __FILE__, __LINE__, __VA_ARGS__);                                    \
    }(), nullptr));                                                                                          
#else
  #define AT_EXIT(...)
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
  if (const auto&& PM_INDENTION_NAME =                                                                        \
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
  ) {} else

#else
  #define PASSERT(...) 
#endif

/**
* @brief Simple break. Breaks when condition is true. Nested scope is executed BEFORE triggering debug break.
* Usage examples:
* @code
* PBREAK(i > 0) std::cout << i; @endcode
*/
#define PBREAK(...) PASSERT(!(__VA_ARGS__))                       

#define STATIC_INITIALIZATION(...)                                \
  struct PM_CONCATENATE(pm_static_initialization, __COUNTER__) {  \
    struct user_defined {                                         \
      user_defined()                                              \
        __VA_ARGS__                                               \
    };                                                            \
    user_defined t;                                               \
  } PM_CONCATENATE(pm_static_initialization_obj, __COUNTER__)

#define PRINT_GIT_BRANCH_HEADER()                                  \
  STATIC_INITIALIZATION({    std::cout << "Branch: ";              \
      system("git branch --show-current");                         \
      INFO("Threads:", tbb::this_task_arena::max_concurrency());   \
  })

#define GLOBAL_VARIABLE(...)                                                   \
  PerfMonitor::internal::MakeFromFancyString<PerfMonitor::GlobalVariableHandler>(PM_STRING_TO_LAMBDA("" ## __VA_ARGS__))


#define OFF if constexpr (true) {} else

// ReSharper restore IdentifierTypo
