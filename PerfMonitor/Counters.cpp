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
#include <set>
#include <atomic>

struct CounterStorage
  {
    explicit CounterStorage(const char* ip_name)
      : name(ip_name) {}

    size_t data{0};
    std::vector<size_t*> clients;
    std::string name;
  };

struct CountersStorage : PerfMonitor::internal::IObject
  {
  std::vector<CounterStorage> counters;
  std::map<std::string, size_t> name_w_ids;

  std::recursive_mutex mutex;
  };

CountersStorage * p_counter_storage = nullptr;
std::atomic<size_t> thread_counter = 0;
std::atomic<bool> is_thread_count_tracking_enabled = true; 

struct ThreadCounter
  {
    ThreadCounter()
      {
      ++thread_counter;
      }
  };

static thread_local ThreadCounter thread_initialization;

namespace PerfMonitor
  {
  void CombineAllCounters()
    {
    for (auto& counter : p_counter_storage->counters)
      {
      for (size_t* client : counter.clients)
        {
        counter.data += *client;
        *client = 0;
        }
      }
    }

  // std::int64_t GetTotalCounterValue(const size_t i_id)
  //   {
  //   std::lock_guard<std::recursive_mutex> lock(p_counter_storage->m_mutex);
  //   std::int64_t result = 0;
  //   for (const auto& ar : p_counter_storage->m_all_counters_raw)
  //     result += ar[i_id];
  //   return result;
  //   }

  // void SetTotalCounterValue(size_t i_id, std::int64_t i_value)
  //   {
  //   std::lock_guard<std::recursive_mutex> lock(p_counter_storage->m_mutex);
  //   for (auto& ar : p_counter_storage->m_all_counters_raw)
  //     ar[i_id] = 0;
  //   p_counter_storage->m_all_counters_raw.front()[i_id] = i_value;
  //   }

  void DisableThreadCountTracking()
    {
    is_thread_count_tracking_enabled = false;
    }

  void PrintAllCounters()
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->mutex);

    if (is_thread_count_tracking_enabled == false && p_counter_storage->counters.empty())
      return;

    std::wcout << L"\n";
    if (is_thread_count_tracking_enabled)
      std::wcout << L"Num threads: " << thread_counter << L"\n";

    if (p_counter_storage->counters.empty())
      return;
    CombineAllCounters();

    std::vector<std::reference_wrapper<CounterStorage>> counters{p_counter_storage->counters.begin(), p_counter_storage->counters.end()};
    std::sort(counters.begin(), counters.end(), 
      [&](const std::reference_wrapper<CounterStorage> left, const std::reference_wrapper<CounterStorage> right)
      {
      return left.get().name < right.get().name;
      });

    char previous_category = 0;
    for (const auto& counter : counters)
      {
      if (counter.get().name[0] != previous_category)
        {
        previous_category = counter.get().name[0];
        if (previous_category == 'T')
          std::wcout << L"[TIMERSUM]\n";
        if (previous_category == 'C')
          std::wcout << L"[STATISTIC]\n";
        }
      const std::int64_t value = counter.get().data;
      if (previous_category == 'T')
        std::wcout << L"  " << counter.get().name.c_str() + 1 << L" time: " << std::chrono::microseconds(static_cast<long long>(value * GetInvFrequency())) << L"\n";
      if (previous_category == 'C')
        std::wcout << L"  " << counter.get().name.c_str() + 1 << L": " << NumericRecord { value } << L"\n";
      }
    }

  void CounterUtils::RegisterCounter(const char* ip_name, size_t* ip_client)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->mutex);
    auto it = p_counter_storage->name_w_ids.find(ip_name);
    if (it == p_counter_storage->name_w_ids.end())
      {
      const size_t id = p_counter_storage->counters.size();
      p_counter_storage->counters.emplace_back(ip_name);
      p_counter_storage->counters.back().clients.emplace_back(ip_client);
      p_counter_storage->name_w_ids[ip_name] = id;
      }
    else
      {
      p_counter_storage->counters[it->second].clients.emplace_back(ip_client);
      }
    }

  void CounterUtils::UnRegisterCounter(size_t* ip_client)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->mutex);
    for (auto& counter : p_counter_storage->counters)
      {
      for (size_t* client : counter.clients)
        if (client == ip_client)
          {
          counter.data += *client;
          auto& clients = counter.clients;
          clients.erase(std::remove(clients.begin(), clients.end(), ip_client), clients.end());
          return;
          }
      }
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
      m_counter_storage = std::make_unique<CountersStorage>();
      m_memory_watchers = PerfMonitor::GetMemoryWatchers();
      p_counter_storage = static_cast<CountersStorage*>(m_counter_storage.get());
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
