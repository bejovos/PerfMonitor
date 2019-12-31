#pragma once

#include "IObject.h"
#include "Indention.h"
#include "Utils.h"
#include "Counters.h"
#include "_API.h"

#include <iostream>
#include <chrono>
#include <memory>
#include <utility>

#pragma warning(disable:5030) // attribute is not recognized

namespace PerfMonitor
  {
  // first - stamp index, second - memory in bytes
  PERFMONITOR_API std::pair<size_t, size_t> MakeStartingMemoryStamp();
  // first - interval peak, second - interval delta
  PERFMONITOR_API std::pair<std::uint64_t, std::int64_t> MakeEndingMemoryStamp(size_t i_starting_memory_stamp_index);

  std::unique_ptr<internal::IObject> GetMemoryWatchers();

  
  template <bool WatchTime, bool WatchMemory>
  struct [[rscpp::guard]] TimeAndMemoryWatcher final : internal::non_copyable, internal::non_moveable, internal::IObject, internal::convertable_to_bool_false
    {
      TimeAndMemoryWatcher()
        {
        if (Indention::GetLastChar() == ' ')
          Indention::PushIndention('|', this);
        else 
          Indention::PushIndention(' ', this);

        if (WatchMemory)
          {          
          auto start = MakeStartingMemoryStamp();          
          // std::wcout << "(" << MemoryRecord { start.second } << ")";
          m_stamp_index = start.first;
          }
        if (WatchTime)
          m_time_counter = InitTimeCounter();
        Indention::SetEndNeeded(true);
        }
      
      ~TimeAndMemoryWatcher() override
        {
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

          std::wcout << (start_from_whitespace ? L" " : L"") << L"memory: " << MemoryRecord{end.second};
          std::wcout << L" peak: " << MemoryRecord { end.first };
          }
        std::wcout << L"\n";
        }

      std::int64_t m_time_counter;
      size_t m_stamp_index;
    };
  }