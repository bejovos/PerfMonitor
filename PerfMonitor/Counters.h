#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <any>
#include <io.h>
#include <stdio.h>

#include "_API.h"
#include "IObject.h"
#include "Utils.h"
#include "StringToClass.h"

// Records
namespace PerfMonitor {
  struct MemoryRecord
  {
    std::int64_t counter;
    bool print_sign;

    template <class T, typename std::enable_if<std::is_signed<T>::value>::type* = nullptr>
    explicit MemoryRecord(const T& value)
      : counter(value)
      , print_sign(true)
    { }

    template <class T, typename std::enable_if<std::is_unsigned<T>::value>::type* = nullptr>
    explicit MemoryRecord(const T& value)
      : counter(value)
      , print_sign(false)
    { }
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
      stream << std::setprecision(0) << round(time) << "us";
    else if (time < 1. * 1000 * 10)
      stream << std::setprecision(2) << round(time / 1000 * 100) / 100 << "ms";
    else if (time < 1. * 1000 * 100)
      stream << std::setprecision(1) << round(time / 1000 * 10) / 10 << "ms";
    else if (time < 1. * 1000 * 1000)
      stream << std::setprecision(0) << round(time / 1000 * 1) / 1 << "ms";
    else if (time < 1. * 1000 * 1000 * 10)
      stream << std::setprecision(2) << round(time / 1000 / 1000 * 100) / 100 << "s";
    else if (time < 1. * 1000 * 1000 * 100)
      stream << std::setprecision(1) << round(time / 1000 / 1000 * 10) / 10 << "s";
    else
      stream << std::setprecision(0) << round(time / 1000 / 1000 * 1) / 1 << "s";
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
    if (record.print_sign) {
      if (memory >= 0)
        stream << '+';
      else {
        stream << '-';
        memory = - memory;
      }
    }

    if (round(memory) < 1000)
      stream << std::setprecision(0) << round(memory) << " b";
    else if (round(memory / 1024 * 100) < 1000)
      stream << std::setprecision(2) << round(memory / 1024 * 100) / 100 << " kb";
    else if (round(memory / 1024 * 10) < 1000)
      stream << std::setprecision(1) << round(memory / 1024 * 10) / 10 << " kb";
    else if (round(memory / 1024 * 1) < 1000)
      stream << std::setprecision(0) << round(memory / 1024 * 1) / 1 << " kb";
    else if (round(memory / 1024 / 1024 * 100) < 1000)
      stream << std::setprecision(2) << round(memory / 1024 / 1024 * 100) / 100 << " mb";
    else if (round(memory / 1024 / 1024 * 10) < 1000)
      stream << std::setprecision(1) << round(memory / 1024 / 1024 * 10) / 10 << " mb";
    else if (round(memory / 1024 / 1024 * 1) < 1000)
      stream << std::setprecision(0) << round(memory / 1024 / 1024 * 1) / 1 << " mb";
    else if (round(memory / 1024 / 1024 / 1024 * 100) < 1000)
      stream << std::setprecision(2) << round(memory / 1024 / 1024 / 1024 * 100) / 100 << " gb";
    else if (round(memory / 1024 / 1024 / 1024 * 10) < 1000)
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
    for (int j = 0; j < v; ++j) {
      stream << "\'" << s[i] << s[i + 1] << s[i + 2];
      i += 3;
    }
    return stream;
  }

}

namespace PerfMonitor {
  PERFMONITOR_API void DisableThreadCountTracking();

  namespace CounterUtils {
    PERFMONITOR_API void RegisterCounter(const char* const* ip_counter, size_t* ip_client);
    PERFMONITOR_API void UnRegisterCounter(size_t* ip_client);
    PERFMONITOR_API void*& RegisterGlobalVariable(const char* const* ip_counter);

    // not thread safe
    // PERFMONITOR_API size_t GetTotalValue(const char* ip_name);
    // not thread safe
    // PERFMONITOR_API void SetTotalValue(const char* ip_name, size_t i_value);
    // not thread safe
    PERFMONITOR_API void ResetCounters(const char* ip_regexp, const char* ip_regexp_to_print = ".*");
  }

  // not thread safe, called during application exit
  PERFMONITOR_API void PrintAllCounters();

  template <class TypeString, class MessageString, class FileString, class FunctionString, class LineString>
  struct CounterParams
  {
    static constexpr const char* strings[] = {
      TypeString::Str(),
      MessageString::Str(),
      FileString::Str(),
      FunctionString::Str(),
      LineString::Str()
    };
  };

  template <class CounterParams>
  struct Counter
  {
    struct Storage
    {
      explicit Storage()
      {
        CounterUtils::RegisterCounter(CounterParams::strings, &data);
      }

      ~Storage()
      {
        CounterUtils::UnRegisterCounter(&data);
      }

      void Increment(const size_t i_value)
      {
        data += i_value;
      }

      size_t data{ 0 };
    };

    static thread_local Storage storage;
  };

  template <class CounterParams>
  thread_local typename Counter<CounterParams>::Storage Counter<CounterParams>::storage;

  template <class... Strings>
  struct TimerSum
    : internal::non_copyable
    , internal::non_moveable
    , internal::convertable_to_bool_false
    , Counter<CounterParams<String<'T'>, Strings...>>
  {
    ~TimerSum()
    {
      Counter<CounterParams<String<'T'>, Strings...>>::storage.Increment(FinalizeTimeCounter(m_value));
    }

    const std::int64_t m_value = InitTimeCounter();
  };

  template <class... Strings>
  struct StaticCounter
    : internal::non_copyable
    , internal::non_moveable
    , Counter<CounterParams<String<'C'>, Strings...>>
  {
    void Add(const size_t value = 1)
    {
      Counter<CounterParams<String<'C'>, Strings...>>::storage.Increment(value);
    }
  };

  template <class... Strings>
  struct GlobalVariableHandler
    : internal::non_copyable
    , internal::non_moveable
  {
    using params = CounterParams<String<'V'>, Strings...>;
    static std::any* Init()
    {
      void*& tmp = 
        CounterUtils::RegisterGlobalVariable(params::strings);
      if (tmp == nullptr)
        tmp = new std::any();
      return (std::any*)tmp;
    }

    template <class T>
    T& get()
    {
      if (data == nullptr)
        data = Init();
      return std::any_cast<T&>(*data);
    }

    template <class T>
    T set(T variable)
    {
      if (data == nullptr)
        data = Init();
      *data = variable;
      std::cout << Purple << "#" << params::strings[1] << LightGray << " = " << variable << "\n"; 
      return variable;
    }

    bool has_value()
    {
      if (data == nullptr)
        data = Init();
      return data->has_value();
    }

    template <class T>
    auto read()
    {
      std::cout << Purple << "#" << params::strings[1] << LightGray << " = ";
      T t;
      std::cin >> std::boolalpha >> t;
      if (IsCinRedirected())
        std::cout << std::boolalpha << t << std::endl;

      if (data == nullptr)
        data = Init();
      *data = t;
    }

  private:
    static bool IsCinRedirected()
      {
        return !_isatty(_fileno(stdin));
      }

    static std::any* data;
  };

  template <class... Strings>
  std::any* GlobalVariableHandler<Strings...>::data = nullptr;
}
