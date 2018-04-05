#pragma once

#include "_API.h"
#include "IObject.h"
#include "Utils.h"

#include <cstdint>
#include <sstream>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <chrono>

// Records
namespace PerfMonitor
  {
  struct MemoryRecord
    {
      std::uint64_t counter;
    };

  struct NumericRecord
    {
      std::int64_t counter;
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

  template <class Stream>
  Stream& operator <<(Stream& stream, const NumericRecord& record)
    {
    std::ostringstream buf;
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
      stream << " " << s[i] << s[i + 1] << s[i + 2];
      i += 3;
      }
    return stream;
    }
  }

// Static counters, TimerSum
extern "C" {
PERFMONITOR_API void ASM_IncrementCounter(size_t i_id);
PERFMONITOR_API void ASM_IncrementCounter2(size_t i_id, std::int64_t i_value);
}

namespace PerfMonitor
  {
  PERFMONITOR_API size_t RegisterCounter(int i_category, const char* i_name);

  // not thread safe
  PERFMONITOR_API std::int64_t GetTotalCounterValue(size_t i_id);
  // not thread safe
  PERFMONITOR_API void SetTotalCounterValue(size_t i_id, std::int64_t i_value);

  struct CounterId
    {
      explicit CounterId(const int i_category, const char* i_name)
        : m_offset(RegisterCounter(i_category, i_name) * 8) // multiply by 8 to transform counter into offset
        {
        }

      size_t GetId() const { return m_offset / 8; }

      size_t GetOffset() const { return m_offset; }

    private:
      const size_t m_offset;
    };

  template <int Category, class TString>
  struct CounterInitialization
    {
      static const CounterId id;
    };

  template <int Category, class TString>
  const CounterId CounterInitialization<Category, TString>::id(Category, TString::MakeString().c_str());

  namespace StaticCounter
    {
    static size_t GetTotalValue(const size_t i_id)
      {
      return GetTotalCounterValue(i_id);
      }

    static void SetTotalValue(const size_t i_id, size_t i_value)
      {
      return SetTotalCounterValue(i_id, i_value);
      }
    }

  struct TimerSum : internal::non_copyable, internal::convertable_to_bool_false
    {
      explicit TimerSum(const size_t i_offset)
        : m_value(InitTimeCounter())
        , m_offset(i_offset)
        {
        }

      TimerSum(TimerSum&& i_object) noexcept
        : m_value(i_object.m_value)
        , m_offset(i_object.m_offset)
        {
        assert(i_object.is_valid);
        i_object.is_valid = false;
        }

      ~TimerSum()
        {
        if (is_valid == false)
          return;
        is_valid = true;
        m_value = FinalizeTimeCounter(m_value);
        ASM_IncrementCounter2(m_offset, m_value);
        }

      static std::chrono::microseconds GetTotalValue(const size_t i_id)
        {
        return std::chrono::microseconds(static_cast<long long>(GetTotalCounterValue(i_id) * GetInvFrequency()));
        }

      static void SetTotalValue(const size_t i_id, std::chrono::microseconds i_value)
        {
        return SetTotalCounterValue(i_id, static_cast<std::int64_t>(i_value.count() / GetInvFrequency()));
        }

      std::int64_t m_value;
      const size_t m_offset;
      bool is_valid = true;
    };
  }
