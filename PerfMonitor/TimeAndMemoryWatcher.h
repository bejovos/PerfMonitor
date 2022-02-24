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

  template <bool WatchTime, bool WatchMemoryIncrease, bool WatchMemoryPeak>
  struct [[rscpp::guard]] TimeAndMemoryWatcher final : internal::non_copyable, internal::non_moveable, internal::IObject, internal::convertable_to_bool_false
    {
      TimeAndMemoryWatcher()
      {
        if (Indention::GetLastChar() == ' ')
          Indention::PushIndention('|', this);
        else 
          Indention::PushIndention(' ', this);

        if constexpr (WatchMemoryIncrease || WatchMemoryPeak) {          
          auto start = MakeStartingMemoryStamp();          
          m_stamp_index = start.first;
        }
        if constexpr (WatchTime)
          m_time_counter = GetRealTime();
        Indention::SetEndNeeded(true);
      }
      
      ~TimeAndMemoryWatcher() override
      {
        if constexpr (WatchTime)
          m_time_counter = GetRealTime() - m_time_counter;
        bool start_from_whitespace = Indention::SetEndNeeded(false);
        Indention::PopIndention();
        if constexpr (WatchTime) {
          std::wcout << (start_from_whitespace ? L" " : L"");
          std::wcout << L"time: " << std::chrono::microseconds(static_cast<long long>(m_time_counter * GetInvFrequency()));
          start_from_whitespace = true;
        }
        if constexpr (WatchMemoryIncrease || WatchMemoryPeak) {
          const std::pair<std::uint64_t, std::int64_t> end = MakeEndingMemoryStamp(m_stamp_index);

          if constexpr (WatchMemoryIncrease) {
            std::wcout << (start_from_whitespace ? L" " : L"");
            std::wcout << L"memory: " << MemoryRecord{ end.second };
            start_from_whitespace = true;
          }

          if constexpr (WatchMemoryPeak) {
            std::wcout << (start_from_whitespace ? L" " : L"");
            std::wcout << L"peak: " << MemoryRecord { end.first };
            start_from_whitespace = true;
          }
        }
        std::wcout << L"\n";
      }

      std::int64_t m_time_counter;
      size_t m_stamp_index;
    };

  // struct [[rscpp::guard]] KernelTimeWatcher final : internal::non_copyable, internal::non_moveable, internal::IObject, internal::convertable_to_bool_false
  // {
  //   KernelTimeWatcher()
  //   {
  //     if (Indention::GetLastChar() == ' ')
  //       Indention::PushIndention('|', this);
  //     else
  //       Indention::PushIndention(' ', this);
  //
  //     m_kernel_time_counter = GetKernelTime();
  //     m_user_time_counter = GetUserTime();
  //     Indention::SetEndNeeded(true);
  //   }
  //
  //   ~KernelTimeWatcher() override
  //   {
  //     m_kernel_time_counter = GetKernelTime() - m_kernel_time_counter;
  //     m_user_time_counter = GetUserTime() - m_user_time_counter;
  //
  //     bool start_from_whitespace = Indention::SetEndNeeded(false);
  //     Indention::PopIndention();
  //     if constexpr (true) {
  //
  //       std::wcout << (start_from_whitespace ? L" " : L"");
  //       std::wcout << L"kernel: " << std::chrono::microseconds(static_cast<long long>(m_kernel_time_counter * GetInvFrequency()));
  //       start_from_whitespace = true;
  //     }
  //     if constexpr (true) {
  //       std::wcout << (start_from_whitespace ? L" " : L"");
  //       std::wcout << L"user: " << std::chrono::microseconds(static_cast<long long>(m_user_time_counter * GetInvFrequency()));
  //       start_from_whitespace = true;
  //     }
  //     std::wcout << L"\n";
  //   }
  //
  //   std::int64_t m_kernel_time_counter;
  //   std::int64_t m_user_time_counter;
  // };

  }