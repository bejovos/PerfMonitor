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
std::deque<std::vector<std::int64_t>> g_all_counters_raw;
std::recursive_mutex g_mutex;

namespace PerfMonitor
  {
  std::vector<std::int64_t> CombineAllCounters()
    {
    const size_t num_counters = g_all_counters.size();
    std::vector<std::int64_t> result(num_counters, 0);
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
    const std::vector<std::int64_t> all_counters = CombineAllCounters();
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
      const std::int64_t value = all_counters[v.second];
      if (previous_category == 0)
        std::wcout << L"  " << v.first.second << L" time: " << TimeRecord{static_cast<std::uint64_t>(value)} << L"\n";
      if (previous_category == 1)
        std::wcout << L"  " << v.first.second << L" memory: "  << MemoryRecord{static_cast<std::uint64_t>(value)} << L"\n";
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
    std::shared_ptr<PerfMonitor::internal::IObject> m_indentions_holder;
    std::shared_ptr<PerfMonitor::internal::IObject> m_std_stream_switcher;
    CoutFinalizer()
      {
      m_indentions_holder = PerfMonitor::Indention::GetIndentionsHolder();
      m_std_stream_switcher = PerfMonitor::Indention::GetStdStreamSwitcher();    
      }
    ~CoutFinalizer()
      {
      g_mutex.lock();
      // Not a very elegant solution but during application termination this is the only thing we can do
      m_indentions_holder.get()->~IObject();
      PerfMonitor::PrintAllCounters();
      m_std_stream_switcher.get()->~IObject();
      g_mutex.unlock();
      }
  };

CoutFinalizer g_finalizer;

extern "C"
  {
  std::int64_t * PrepareNewCounterStorage()
    {
    g_mutex.lock();
    g_all_counters_raw.emplace_back();    
    auto & storage = g_all_counters_raw.back();
    storage.resize(1024, 0); // no more than 1024 counters
    std::int64_t * result = storage.data();
    g_mutex.unlock();
    return result;
    }
  }
