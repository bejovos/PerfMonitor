#pragma once

#include "_API.h"
#include "IObject.h"
#include "Utils.h"
#include "Indention.h"

#include <cstdint>
#include <sstream>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>

// Records
namespace PerfMonitor
  {
  struct TimeRecord
    {
    std::uint64_t counter;
    };

  struct MemoryRecord
    {
    std::uint64_t counter;
    };

  struct NumericRecord
    {
    std::int64_t counter;
    };

  template <class Stream>
  Stream& operator <<(Stream& stream, const TimeRecord& record)
    {
    const double time = record.counter / GetFrequency();
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

namespace PerfMonitor
  {
  template <bool WatchTime, bool WatchMemory, bool PreciseMemory>
  struct TimeAndMemoryWatcher : internal::non_copyable, internal::IObject, internal::convertable_to_bool_false
    {
    TimeAndMemoryWatcher()
      {
      Indention::PushIndention(char(179), this);
      InitCounter();
      Indention::SetEndNeeded(true);
      }

    TimeAndMemoryWatcher(TimeAndMemoryWatcher && i_watcher) noexcept
      {
      assert(i_watcher.is_valid);
      i_watcher.is_valid = false;
      Indention::PopIndention();
      Indention::PushIndention(char(179), this);
      time_counter = i_watcher.time_counter;
      memory_old_current = i_watcher.memory_old_current;
      memory_old_peak = i_watcher.memory_old_peak;
      m_array = i_watcher.m_array;
      }

    void InitCounter()
      {
      if (WatchMemory)
        {
        memory_old_current = GetCurrentMemoryConsumption();
        memory_old_peak = GetPeakMemoryConsumption();
        if (PreciseMemory)
          {
          const std::uint64_t amount = (memory_old_peak > memory_old_current ? memory_old_peak - memory_old_current : 0);
          std::wcout << "(" << MemoryRecord{ amount } << ")";
          if (amount == 0)
            m_array = nullptr;
          else
            m_array = new std::uint8_t[amount];
          for (std::uint64_t i = 0; i < amount; ++i)
            m_array[i] = 0;
          memory_old_current = GetCurrentMemoryConsumption();
          memory_old_peak = GetPeakMemoryConsumption();
          }
        }
      if (WatchTime)
        time_counter = InitTimeCounter();
      }

    ~TimeAndMemoryWatcher() override
      {
      if (is_valid == false)
        return;
      is_valid = false;
      if (WatchTime)
        time_counter = FinalizeTimeCounter(time_counter);
      std::uint64_t memory_new_current = 0, memory_new_peak = 0;
      if (WatchMemory)
        {
        memory_new_current = GetCurrentMemoryConsumption();
        memory_new_peak = GetPeakMemoryConsumption();
        }
      bool start_from_whitespace = Indention::SetEndNeeded(false);
      Indention::PopIndention();
      if (WatchTime)
        {
        std::wcout << (start_from_whitespace ? L" " : L"") << L"time: " << TimeRecord{ static_cast<std::uint64_t>(time_counter) };
        start_from_whitespace = true;
        }
      if (WatchMemory)
        {
        std::wcout << (start_from_whitespace ? L" " : L"");
        if (memory_new_current > memory_old_current)
          std::wcout << L"memory: +" << MemoryRecord{ memory_new_current - memory_old_current };
        else
          std::wcout << L"memory: -" << MemoryRecord{ memory_old_current - memory_new_current };

        std::wcout << L" peak: ";
        if (memory_new_peak == memory_old_peak)
          std::wcout << L"< ";
        std::wcout << MemoryRecord{ memory_new_peak - memory_old_current };
        }
      std::wcout << L"\n";
      if (PreciseMemory)
        delete m_array;
      }

    std::int64_t time_counter;
    std::uint64_t memory_old_current;
    std::uint64_t memory_old_peak;
    std::uint8_t* m_array;
    };
  }

// Static counters, TimerSum
extern "C"
  {  
  PERFMONITOR_API void ASM_IncrementCounter(size_t i_id);
  PERFMONITOR_API void ASM_IncrementCounter2(size_t i_id, std::int64_t i_value);
  }

namespace PerfMonitor
  {
  PERFMONITOR_API size_t RegisterCounter(int i_category, const char * i_name);

  struct CounterId
    {
    explicit CounterId(const int i_category, const char * i_name)
      : id(RegisterCounter(i_category, i_name) * 8) // multiply by 8 to transform counter into offset
      {}
    const size_t id;
    };

  template <int Category, class TString>
  struct CounterInitialization
    {
    static const CounterId id;
    };
  template <int Category, class TString>
  const CounterId CounterInitialization<Category, TString>::id(Category, TString::MakeString().c_str());

  struct TimerSum : internal::non_copyable, internal::convertable_to_bool_false
    {
      explicit TimerSum(const size_t i_id)
        : m_value(InitTimeCounter()), m_id(i_id)
        {}

      TimerSum(TimerSum&& i_object) noexcept
        : m_value(i_object.m_value), m_id(i_object.m_id)
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
        ASM_IncrementCounter2(m_id, m_value);
        }

      std::int64_t m_value;
      const size_t m_id;
      bool is_valid = true;
    };
  }
