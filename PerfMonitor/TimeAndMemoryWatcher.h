#pragma once

#include "IObject.h"
#include "Indention.h"
#include "Utils.h"
#include "Counters.h"

#include <iostream>

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

      TimeAndMemoryWatcher(TimeAndMemoryWatcher&& i_watcher) noexcept
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
            std::wcout << "(" << MemoryRecord { amount } << ")";
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
          std::wcout << (start_from_whitespace ? L" " : L"") << L"time: " << std::chrono::microseconds(
            static_cast<long long>(time_counter * GetInvFrequency()));
          start_from_whitespace = true;
          }
        if (WatchMemory)
          {
          std::wcout << (start_from_whitespace ? L" " : L"");
          if (memory_new_current > memory_old_current)
            std::wcout << L"memory: +" << MemoryRecord { memory_new_current - memory_old_current };
          else
            std::wcout << L"memory: -" << MemoryRecord { memory_old_current - memory_new_current };

          std::wcout << L" peak: ";
          if (memory_new_peak == memory_old_peak)
            std::wcout << L"< ";
          std::wcout << MemoryRecord { memory_new_peak - memory_old_current };
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