#include "Counters.h"
#include "Indention.h"
#include "TimeAndMemoryWatcher.h"
#include "WriteToStream.h"

#include <csignal>
#include <deque>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

struct CounterStorage : PerfMonitor::internal::IObject
  {
  std::map<std::pair<int, std::string>, size_t> m_all_counters;
  std::deque<std::vector<std::int64_t>> m_all_counters_raw;
  std::recursive_mutex m_mutex;

  std::queue<std::vector<std::int64_t>> m_all_counters_raw_unused;

  CounterStorage()
    {
    for (size_t i = 0; i < 32; ++i)
      {
      m_all_counters_raw_unused.emplace(1024, 0);
      }
    }

  std::int64_t * PrepareNewCounterStorage()
    {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (!m_all_counters_raw_unused.empty())
      {
      m_all_counters_raw.emplace_back(
        std::move(m_all_counters_raw_unused.front()));
      m_all_counters_raw_unused.pop();
      }
    else
      {
      m_all_counters_raw.emplace_back(1024, 0); // no more than 1024 counters
      }

    return m_all_counters_raw.back().data();
    }
  };

CounterStorage * p_counter_storage = nullptr;

namespace PerfMonitor
  {
  std::vector<std::int64_t> CombineAllCounters()
    {
    const size_t num_counters = p_counter_storage->m_all_counters.size();
    std::vector<std::int64_t> result(num_counters, 0);
    for (const auto& ar : p_counter_storage->m_all_counters_raw)
      {
      for (size_t i = 0; i < num_counters; ++i)
        result[i] += ar[i];
      }
    return result;
    }

  std::int64_t GetTotalCounterValue(const size_t i_id)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->m_mutex);
    std::int64_t result = 0;
    for (const auto& ar : p_counter_storage->m_all_counters_raw)
      result += ar[i_id];
    return result;
    }

  void SetTotalCounterValue(size_t i_id, std::int64_t i_value)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->m_mutex);
    for (auto& ar : p_counter_storage->m_all_counters_raw)
      ar[i_id] = 0;
    p_counter_storage->m_all_counters_raw.front()[i_id] = i_value;
    }

  void PrintAllCounters()
    {
    if (p_counter_storage->m_all_counters_raw.empty() || p_counter_storage->m_all_counters.empty())
      return;
    const std::vector<std::int64_t> all_counters = CombineAllCounters();
    std::wcout << L"\n";
    if (p_counter_storage->m_all_counters_raw.size() != std::thread::hardware_concurrency())
      std::wcout << L"Num storages: " << p_counter_storage->m_all_counters_raw.size() << L"\n";
    int previous_category = -1;
    for (const auto& v : p_counter_storage->m_all_counters)
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
        std::wcout << L"  " << v.first.second << L" time: " << std::chrono::microseconds(static_cast<long long>(value * GetInvFrequency())) << L"\n";
      if (previous_category == 1)
        std::wcout << L"  " << v.first.second << L" memory: " << MemoryRecord { static_cast<std::uint64_t>(value) } << L"\n";
      if (previous_category == 2)
        std::wcout << L"  " << v.first.second << L": " << NumericRecord { value } << L"\n";
      }
    }

  size_t RegisterCounter(int i_category, const char* i_name)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->m_mutex);
    const auto it = p_counter_storage->m_all_counters.find({ i_category, i_name });
    size_t index;
    if (it == p_counter_storage->m_all_counters.end())
      {
      index = p_counter_storage->m_all_counters.size();
      p_counter_storage->m_all_counters.emplace(std::make_pair(
        std::make_pair(i_category, i_name),
        index));
      }
    else
      index = it->second;
    return index;
    }
  }

void TerminateHandler();
void SIGSEGHandler(int i_sig);

struct CoutFinalizer : PerfMonitor::internal::IObject
  {
    std::unique_ptr<PerfMonitor::internal::IObject> m_indentions_holder;
    std::unique_ptr<PerfMonitor::internal::IObject> m_std_stream_switcher;
    std::unique_ptr<PerfMonitor::internal::IObject> m_counter_storage;
    std::unique_ptr<PerfMonitor::internal::IObject> m_memory_watchers;

    CoutFinalizer()
      {
      m_indentions_holder = PerfMonitor::Indention::GetIndentionsHolder();
      m_std_stream_switcher = PerfMonitor::Indention::GetStdStreamSwitcher();
      m_counter_storage = std::make_unique<CounterStorage>();
      m_memory_watchers = PerfMonitor::GetMemoryWatchers();
      p_counter_storage = static_cast<CounterStorage*>(m_counter_storage.get());
      set_terminate(&TerminateHandler);
      signal(SIGSEGV, SIGSEGHandler);
      }

    ~CoutFinalizer()
      {
      if (is_valid == false)
        return;
      is_valid = false;
      // Not a very elegant solution but during application termination this is the only thing we can do
      m_indentions_holder.get()->~IObject();
      PerfMonitor::PrintAllCounters();
      m_std_stream_switcher.get()->~IObject();
      }
  };

CoutFinalizer g_finalizer;

void SIGSEGHandler(int i_sig)
  {
  std::cout << PerfMonitor::Color::Red;
  std::cout << "  [Segmentation faults]\n";
  std::cout << PerfMonitor::Color::LightGray;
  g_finalizer.~CoutFinalizer();
  }

void TerminateHandler()
  {
  std::cout << PerfMonitor::Color::Red;
  std::cout << "  [Uncaught exception]\n";
  const std::exception_ptr current_exception = std::current_exception();
  try
    {
    std::rethrow_exception(current_exception);
    }
  catch (std::exception& ex)
    {
    std::cout << "  " << ex.what() << "\n";
    }
  std::cout << PerfMonitor::Color::LightGray;
  g_finalizer.~CoutFinalizer();
  }

extern "C" 
  {
  std::int64_t* PrepareNewCounterStorage()
    {
    return p_counter_storage->PrepareNewCounterStorage();
    }
  }
