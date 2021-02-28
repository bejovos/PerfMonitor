#include "Counters.h"
#include "Indention.h"
#include "TimeAndMemoryWatcher.h"
#include "WriteToStream.h"

#include <atomic>
#include <csignal>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

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

  size_t CounterUtils::GetTotalValue(const char* ip_name)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->mutex);
    std::int64_t result = 0;
    const size_t id = p_counter_storage->name_w_ids[ip_name]; 
    for (const size_t* ar : p_counter_storage->counters[id].clients)
      result += *ar;
    return result;
    }

  void CounterUtils::SetTotalValue(const char* ip_name, const size_t i_value)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->mutex);
    const size_t id = p_counter_storage->name_w_ids[ip_name]; 
    for (size_t* ar : p_counter_storage->counters[id].clients)
      *ar = 0;
    *p_counter_storage->counters[id].clients.front() = i_value;
    }

  std::vector<std::reference_wrapper<CounterStorage>> GetAllCountersForPrinting()
  {
    CombineAllCounters();

    auto comparator = [&](const std::string& left, const std::string& right) -> bool {
      auto l = left.cbegin();
      auto r = right.cbegin();
      for (; ; ++l, ++r) {
        if (l == left.cend())
          return true;
        if (r == right.cend())
          return false;

        if (isdigit(*l)) {
          if (isdigit(*r)) {
            auto get_number = [](auto it, const auto end) {
              std::string digits;
              for (; it != end && isdigit(*it); ++it)
                digits += *it;
              return digits;
            };

            const std::string left_number = get_number(l, left.cend());
            const std::string right_number = get_number(r, right.cend());
            if (left_number.size() != right_number.size())
              return left_number.size() < right_number.size();
            if (left_number != right_number)
              return left_number < right_number;
            continue;
          }
          return false;
        }

        if (isdigit(*r))
          return true;

        if (*l == *r)
          continue;
        return *l < *r;
      }
    };

    std::vector<std::reference_wrapper<CounterStorage>> counters{ p_counter_storage->counters.begin(), p_counter_storage->counters.end() };
    std::sort(
      counters.begin(),
      counters.end(),
      [&](const std::reference_wrapper<CounterStorage> left, const std::reference_wrapper<CounterStorage> right) {
        return comparator(left.get().name, right.get().name);
      });
    return counters;
  }

  void CounterUtils::ResetCounters(const char* ip_regexp, const char* ip_regexp_to_print)
    {
    std::lock_guard<std::recursive_mutex> lock(p_counter_storage->mutex);
    if (p_counter_storage->counters.empty())
      return;

    auto counters = GetAllCountersForPrinting();

    std::regex reg(ip_regexp);
    std::regex reg_to_print(ip_regexp_to_print);
    for (const auto& counter : counters)
      {
      if (std::regex_match(counter.get().name.data() + 1, reg) == false)
        continue;
      size_t& value = counter.get().data;
      if (std::regex_match(counter.get().name.data() + 1, reg_to_print))
        {
        if (counter.get().name[0] == 'T')
          std::wcout << counter.get().name.c_str() + 1 << L" time: " << std::chrono::microseconds(
            static_cast<long long>(value * GetInvFrequency())) << L"\n";
        if (counter.get().name[0] == 'C')
          std::wcout << counter.get().name.c_str() + 1 << L": " << NumericRecord<std::int64_t> { (std::int64_t)value } << L"\n";
        }
      value = 0;
      }
    }

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
      std::wcout << L"Num threads: " << thread_counter.load() << L"\n";

    if (p_counter_storage->counters.empty())
      return;

    auto counters = GetAllCountersForPrinting();

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
        std::wcout << L"  " << counter.get().name.c_str() + 1 << L": " << NumericRecord<std::int64_t> { value } << L"\n";
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
    std::unique_ptr<IObject> m_indentions_holder;
    std::unique_ptr<IObject> m_counter_storage;
    std::unique_ptr<IObject> m_memory_watchers;

    CoutFinalizer()
      {
      m_indentions_holder = PerfMonitor::Indention::Initialize();
      m_counter_storage = std::make_unique<CountersStorage>();
      m_memory_watchers = PerfMonitor::GetMemoryWatchers();
      p_counter_storage = static_cast<CountersStorage*>(m_counter_storage.get());
      set_terminate(&TerminateHandler);
      signal(SIGSEGV, SIGSEGHandler);
      }

    ~CoutFinalizer()
      {
      // Not a very elegant solution but during application termination this is the only thing we can do
      PerfMonitor::Indention::ForceClear();
      PerfMonitor::PrintAllCounters();
      }
  };

CoutFinalizer g_finalizer;

void SIGSEGHandler(int)
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
