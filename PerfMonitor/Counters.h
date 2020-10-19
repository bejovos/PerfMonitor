#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <chrono>

#include "_API.h"
#include "IObject.h"
#include "Utils.h"
#include "StringToClass.h"

// Records
namespace PerfMonitor
  {
  struct MemoryRecord
    {
      std::int64_t counter;
      bool print_sign;
      template <class T, typename std::enable_if<std::is_signed<T>::value>::type* = nullptr>
      explicit MemoryRecord(const T& value)
        : counter(value), print_sign(true)
        {
        }
      template <class T, typename std::enable_if<std::is_unsigned<T>::value>::type* = nullptr>
      explicit MemoryRecord(const T& value)
         : counter(value), print_sign(false)
         {
         }
    };

  template <class T>
  struct NumericRecord
    {
      T counter;
    };

  template <class Stream>
  Stream& operator <<(Stream& stream, const std::chrono::microseconds& record)
    {
    const double time = static_cast<double>(record.count());
    auto precision = stream.precision();
    auto flags = stream.flags();
    stream << std::fixed;
    SetColor(Color::Yellow);
    if (time < 1000)
      stream << std::setprecision(0) << round(time) << " us";
    else if (time < 1. * 1000 * 10)
      stream << std::setprecision(2) << round(time / 1000 * 100) / 100 << " ms";
    else if (time < 1. * 1000 * 100)
      stream << std::setprecision(1) << round(time / 1000 * 10) / 10 << " ms";
    else if (time < 1. * 1000 * 1000)
      stream << std::setprecision(0) << round(time / 1000 * 1) / 1 << " ms";
    else if (time < 1. * 1000 * 1000 * 10)
      stream << std::setprecision(2) << round(time / 1000 / 1000 * 100) / 100 << " s";
    else if (time < 1. * 1000 * 1000 * 100)
      stream << std::setprecision(1) << round(time / 1000 / 1000 * 10) / 10 << " s";
    else
      stream << std::setprecision(0) << round(time / 1000 / 1000 * 1) / 1 << " s";
    SetColor(Color::LightGray);
    stream.flags(flags);
    stream.precision(precision);
    return stream;
    }

  template <class Stream>
  Stream& operator <<(Stream& stream, const MemoryRecord& record)
    {
    double memory = static_cast<double>(record.counter);
    auto precision = stream.precision();
    auto flags = stream.flags();
    stream << std::fixed;
    SetColor(Color::Yellow);
    if (record.print_sign)
      {
      if (memory >= 0)
        stream << '+';
      else
        {
        stream << '-';
        memory = - memory;
        }
      }
    
    if (memory < 1000)
      stream << std::setprecision(0) << round(memory) << " b";
    else if (memory < 1. * 1024 * 10)
      stream << std::setprecision(2) << round(memory / 1024 * 100) / 100 << " kb";
    else if (memory < 1. * 1024 * 100)
      stream << std::setprecision(1) << round(memory / 1024 * 10) / 10 << " kb";
    else if (memory < 1. * 1024 * 1000)
      stream << std::setprecision(0) << round(memory / 1024 * 1) / 1 << " kb";
    else if (memory < 1. * 1024 * 1024 * 10)
      stream << std::setprecision(2) << round(memory / 1024 / 1024 * 100) / 100 << " mb";
    else if (memory < 1. * 1024 * 1024 * 100)
      stream << std::setprecision(1) << round(memory / 1024 / 1024 * 10) / 10 << " mb";
    else if (memory < 1. * 1024 * 1024 * 1000)
      stream << std::setprecision(0) << round(memory / 1024 / 1024 * 1) / 1 << " mb";
    else if (memory < 1. * 1024 * 1024 * 1024 * 10)
      stream << std::setprecision(2) << round(memory / 1024 / 1024 / 1024 * 100) / 100 << " gb";
    else if (memory < 1. * 1024 * 1024 * 1024 * 100)
      stream << std::setprecision(1) << round(memory / 1024 / 1024 / 1024 * 10) / 10 << " gb";
    else
      stream << std::setprecision(0) << round(memory / 1024 / 1024 / 1024 * 1) / 1 << " gb";
    SetColor(Color::LightGray);
    stream.flags(flags);
    stream.precision(precision);
    return stream;
    }

  template <class Stream, class T>
  Stream& operator <<(Stream& stream, const NumericRecord<T>& record)
    {
    std::stringstream buf;
    buf << record.counter;
    const std::string s = buf.str();
    const int l = static_cast<int>(s.size());
    const int v = (l - 1) / 3;
    const int vv = l - v * 3;
    int i = 0;
    for (; i < vv; ++i)
      stream << s[i];
    for (int j = 0; j < v; ++j)
      {
      stream << "\'" << s[i] << s[i + 1] << s[i + 2];
      i += 3;
      }
    return stream;
    }

  }

namespace PerfMonitor
  {
  PERFMONITOR_API void DisableThreadCountTracking();
  namespace CounterUtils
    {
    // if name starts from 'T' - Time counter, 'C' - Regular counter
    PERFMONITOR_API void RegisterCounter(const char* ip_name, size_t* ip_client);
    PERFMONITOR_API void UnRegisterCounter(size_t* ip_client);

    // not thread safe
    PERFMONITOR_API size_t GetTotalValue(const char* ip_name);
    // not thread safe
    PERFMONITOR_API void SetTotalValue(const char* ip_name, size_t i_value);
    // not thread safe
    PERFMONITOR_API void ResetCounters(const char* ip_regexp, const char* ip_regexp_to_print = ".*");
    }
  // not thread safe, called during application exit
  PERFMONITOR_API void PrintAllCounters();

  template <class String>
  struct Counter
    {
    struct Storage
      {
        explicit Storage()
          {
          CounterUtils::RegisterCounter(String::Str(), &data); 
          }
        ~Storage()
          {
          CounterUtils::UnRegisterCounter(&data);
          }

        void Increment(const size_t i_value)
          {
          data += i_value;
          }

      private:
        size_t data{0};
      };

    static Storage& GetStorage()
      {
      return storage;
      }

    static void SetTotal(size_t value)
      {
      CounterUtils::SetTotalValue(String::Str(), value);
      }

    static size_t GetTotal()
      {
      return CounterUtils::GetTotalValue(String::Str());
      }

    static thread_local Storage storage; 
    };

  template <class String>
  thread_local typename Counter<String>::Storage Counter<String>::storage;

  template <class String>
  struct TimerSum;
  template <char... Chars>
  struct TimerSum<String<Chars...>>
    : internal::non_copyable
    , internal::non_moveable
    , internal::convertable_to_bool_false
    , Counter<String<'T', Chars...>>
    {
      ~TimerSum()
        {
        Counter<String<'T', Chars...>>::GetStorage().Increment(FinalizeTimeCounter(m_value));
        }
      static std::chrono::microseconds GetTotal()
        {
        return std::chrono::microseconds{
          static_cast<size_t>(Counter<String<'T', Chars...>>::GetTotal() 
          * GetInvFrequency())};
        }
      static void SetTotal(std::chrono::microseconds time)
        {
        Counter<String<'T', Chars...>>::SetTotal(static_cast<size_t>(time.count() / GetInvFrequency()));
        }

      const std::int64_t m_value = InitTimeCounter();
    };

  template <class String>
  struct StaticCounter;
  template <char... Chars>
  struct StaticCounter<String<Chars...>>
    : internal::non_copyable
    , internal::non_moveable
    , Counter<String<'C', Chars...>> 
    {
      void Add(const size_t value = 1)
        {
        Counter<String<'C', Chars...>>::GetStorage().Increment(value);
        }
    }; 

  }
