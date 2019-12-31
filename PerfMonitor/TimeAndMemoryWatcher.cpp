#include "TimeAndMemoryWatcher.h"

#include <deque>
#include <mutex>

#include <windows.h>
#include <Psapi.h>
#include <algorithm>

// each constructor/destructor invocation creates a new memory stamp.
// In precise mode we have to allocate additional dummy memory
// in order to cover "peak memory - current memory" gap.
// Dummy memory may be released only when last watcher is destroyed.
// (P - peak, C - current, D - extra dummy memory):
// example:                    [A------[B-------A]------B]-----[C-------C]
// without measurements(C): 7---5---8---6---3---5---8---7---2---4---5---6---1
// measurements(P,D):          7,+2   10,+2   10,+0   12,-4   12,+8   14,-8
// with measurements:       7---5--10---8---7---9--12--11---2---4--13--14---1
// obtained stamps:            [7      [10     10]     12]     [12     14]
// calculations(P):            [  10-7 [  10-10  ] 12-10 ]     [  14-12  ]
// calculations(delta):        [   8-7 [   9-10  ] 11-10 ]     [  14-12  ]
// obtained intervals(P,delta):[   3,1 [   0,-1  ]  2,1  ]     [   2,2   ]
// intervals:                   ---^^^- ---^^^^--               ---^^^---
//                                      ------------^^^--
// overall result (P,delta):    A = 3,0 B = 2,1                 C = 2,2

#undef min
#undef max

struct MemoryWatchers : PerfMonitor::internal::IObject
  {
    size_t m_alive_watchers = 0;
    std::deque<std::unique_ptr<std::uint8_t[]>> m_dummy_memory;
    std::deque<std::uint64_t> m_memory_stamps;
    // first - interval peak, second - interval delta
    std::deque<std::pair<std::uint64_t, std::int64_t>> m_intervals;
  };

MemoryWatchers * p_watchers_holder_raw;
std::mutex mutex;

namespace PerfMonitor
  {
  void AllocateDummyArray(std::uint64_t i_size)
    {
    if (i_size == 0)
      return;
    std::unique_ptr<std::uint8_t[]> ar{new std::uint8_t[i_size]};
    for (std::uint64_t i = 0; i < i_size; ++i)
      ar.get()[i] = 1;
    p_watchers_holder_raw->m_dummy_memory.emplace_back(std::move(ar));
    }

  struct MemoryConsumption
    {
      std::uint64_t peak;
      std::uint64_t current;
    };

  MemoryConsumption GetMemoryConsumption()
    {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(pmc);
    GetProcessMemoryInfo(::GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), pmc.cb);
    return {std::max(pmc.PeakWorkingSetSize, pmc.WorkingSetSize), pmc.WorkingSetSize};
    }

  std::pair<size_t, size_t> MakeStartingMemoryStamp()
    {
    std::lock_guard<std::mutex> lock(mutex);
    ++p_watchers_holder_raw->m_alive_watchers;
    MemoryConsumption memory = GetMemoryConsumption();
    auto& stamps = p_watchers_holder_raw->m_memory_stamps;
    if (!p_watchers_holder_raw->m_memory_stamps.empty())
      {
      p_watchers_holder_raw->m_intervals.emplace_back(
        memory.peak - stamps.back(), memory.current - stamps.back());
      }
    const size_t dummy_array_size = memory.peak - memory.current;
    AllocateDummyArray(dummy_array_size);
    stamps.emplace_back(memory.peak);
    return {stamps.size() - 1, dummy_array_size};
    }

  std::pair<std::uint64_t, std::int64_t> MakeEndingMemoryStamp(const size_t i_starting_memory_stamp_index)
    {
    std::lock_guard<std::mutex> lock(mutex);
    --p_watchers_holder_raw->m_alive_watchers;
    MemoryConsumption memory = GetMemoryConsumption();
    auto& stamps = p_watchers_holder_raw->m_memory_stamps;
    auto& intervals = p_watchers_holder_raw->m_intervals;
    intervals.emplace_back(memory.peak - stamps.back(), memory.current - stamps.back());
    
    std::int64_t overall_peak = 0;
    std::int64_t overall_delta = 0;
    for (size_t i = i_starting_memory_stamp_index; i < intervals.size(); ++i)
      {
      std::int64_t current_peak = overall_delta + 
        static_cast<std::int64_t>(intervals[i].first);
      overall_peak = std::max(overall_peak, current_peak);
      overall_delta += intervals[i].second;
      }

    if (p_watchers_holder_raw->m_alive_watchers == 0)
      {
      // this was the last watcher
      p_watchers_holder_raw->m_dummy_memory.clear();
      p_watchers_holder_raw->m_memory_stamps.clear();
      p_watchers_holder_raw->m_intervals.clear();
      }
    else intervals.pop_back();

    return {static_cast<std::uint64_t>(overall_peak), overall_delta};
    }

  std::unique_ptr<internal::IObject> GetMemoryWatchers()
    {
    auto result = std::make_unique<MemoryWatchers>();
    p_watchers_holder_raw = result.get();
    return std::move(result);
    }
  }