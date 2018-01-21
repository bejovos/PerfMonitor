#include "Counters.h"
#include "WriteToStream.h"
#include "Indention.h"

#include <vector>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <thread>
#include <mutex>

std::map<std::pair<int, std::string>, size_t> g_all_counters;
std::deque<std::vector<__int64>> g_all_counters_raw;
std::recursive_mutex g_mutex;

namespace PerfMonitor
  {
  std::vector<__int64> CombineAllCounters()
    {
    const size_t num_counters = g_all_counters.size();
    std::vector<__int64> result(num_counters, 0);
    for (const auto & ar : g_all_counters_raw)
      {
      for (size_t i=0; i<num_counters; ++i)
        result[i] += ar[i];
      }
    return result;
    }

  void PrintAllCounters()
    {
    if (g_all_counters_raw.empty() || g_all_counters.empty())
      return;
    const std::vector<__int64> all_counters = CombineAllCounters();
    std::wcout << L"\n";
    if (g_all_counters_raw.size() != std::thread::hardware_concurrency())
      std::wcout << L"Num storages: " << g_all_counters_raw.size() << L"\n";
    int previous_category = -1;
    for (const auto & v : g_all_counters)
      {
      if (v.first.first != previous_category)
        {
        previous_category = v.first.first;
        if (previous_category == 0)
          std::wcout << L"[TIMERSUM]\n";
        if (previous_category == 1)
          std::wcout << L"[MEMORY]\n";
        if (previous_category == 2)
          std::wcout << L"[STATISTIC]\n";
        }
      const __int64 value = all_counters[v.second];
      if (previous_category == 0)
        std::wcout << L"  " << v.first.second << L" time: " << TimeRecord{value} << L"\n";
      if (previous_category == 1)
        std::wcout << L"  " << v.first.second << L" memory: "  << MemoryRecord{value} << L"\n";
      if (previous_category == 2)
        std::wcout << L"  " << v.first.second << L": "  << NumericRecord{value} << L"\n";
      }
    }
  
  size_t RegisterCounter(int i_category, const char * i_name)
    {
    g_mutex.lock();
    const auto it = g_all_counters.find({i_category, i_name});
    size_t index;
    if (it == g_all_counters.end())
      {
      index = g_all_counters.size();
      g_all_counters.emplace(std::make_pair(
        std::make_pair(i_category, i_name), 
        index));
      }
    else
      index = it->second;
    g_mutex.unlock();
    return index;
    }
  }

struct CoutFinalizer
  {
    ~CoutFinalizer()
      {
      g_mutex.lock();
      PerfMonitor::Indention::Finalize();
      PerfMonitor::PrintAllCounters();
      g_mutex.unlock();
      }
  };

CoutFinalizer g_finalizer;

extern "C"
  {
  __int64 * PrepareNewCounterStorage()
    {
    g_mutex.lock();
    g_all_counters_raw.emplace_back();    
    auto & storage = g_all_counters_raw.back();
    storage.resize(1024, 0); // no more than 1024 counters
    __int64 * result = storage.data();
    g_mutex.unlock();
    return result;
    }
  }
