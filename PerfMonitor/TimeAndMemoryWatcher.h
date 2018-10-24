#pragma once

#include "IObject.h"
#include "Indention.h"
#include "Utils.h"
#include "Counters.h"
#include "_API.h"

#include <iostream>
#include <chrono>

namespace PerfMonitor
  {
  // first - stamp index, second - memory in bytes
  PERFMONITOR_API std::pair<size_t, size_t> MakeStartingMemoryStamp();
  // first - interval peak, second - interval delta
  PERFMONITOR_API std::pair<std::uint64_t, std::int64_t> MakeEndingMemoryStamp(size_t i_starting_memory_stamp_index);

  std::unique_ptr<internal::IObject> GetMemoryWatchers();


  template <bool WatchTime, bool WatchMemory>
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
        m_time_counter = i_watcher.m_time_counter;
        m_stamp_index = i_watcher.m_stamp_index;
        }

      void InitCounter()
        {
        if (WatchMemory)
          {          
          auto start = MakeStartingMemoryStamp();          
          std::wcout << "(" << MemoryRecord { start.second } << ")";
          m_stamp_index = start.first;
          }
        if (WatchTime)
          m_time_counter = InitTimeCounter();
        }

      ~TimeAndMemoryWatcher() override
        {
        if (is_valid == false)
          return;
        is_valid = false;
        if (WatchTime)
          m_time_counter = FinalizeTimeCounter(m_time_counter);
        bool start_from_whitespace = Indention::SetEndNeeded(false);
        Indention::PopIndention();
        if (WatchTime)
          {
          std::wcout << (start_from_whitespace ? L" " : L"") << L"time: " << std::chrono::microseconds(
            static_cast<long long>(m_time_counter * GetInvFrequency()));
          start_from_whitespace = true;
          }
        if (WatchMemory)
          {
          const std::pair<std::uint64_t, std::int64_t> end = MakeEndingMemoryStamp(m_stamp_index);

          std::wcout << (start_from_whitespace ? L" " : L"");
          if (end.second > 0)
            std::wcout << L"memory: +" << MemoryRecord { static_cast<std::uint64_t>(end.second) };
          else
            std::wcout << L"memory: -" << MemoryRecord { static_cast<std::uint64_t>(-end.second) };

          std::wcout << L" peak: ";
          std::wcout << MemoryRecord { end.first };
          }
        std::wcout << L"\n";
        }

      std::int64_t m_time_counter;
      size_t m_stamp_index;
    };
  }