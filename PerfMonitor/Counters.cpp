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
  std::string label;
  std::string unique_name;
  std::string category;
  std::string sort_name;

  size_t data{ 0 };
  std::vector<size_t*> clients;
};

struct GlobalVariable
{
  std::string name;
  mutable void* data = nullptr;

  bool operator <(const GlobalVariable& other) const
  {
    return name < other.name;
  }
};

struct TotalStorage : PerfMonitor::internal::IObject
{
  std::vector<CounterStorage> counters;
  std::map<std::string, size_t> name_w_ids;
  std::set<GlobalVariable> global_variables;

  std::recursive_mutex mutex;
};

TotalStorage* p_total_storage = nullptr;
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

namespace PerfMonitor {
  void CombineAllCounters()
  {
    for (auto& counter : p_total_storage->counters) {
      for (size_t* client : counter.clients) {
        counter.data += *client;
        *client = 0;
      }
    }
  }

  size_t CounterUtils::GetTotalValue(const char* const* ip_counter)
    {
    std::lock_guard lock(p_total_storage->mutex);
    const char* type = ip_counter[0];
    const char* message = ip_counter[1];
    const char* file = ip_counter[2];
    const char* function = ip_counter[3];
    const char* line = ip_counter[4];

    std::string counter_unique_name;
    if (strlen(message) != 0)
      counter_unique_name = std::string(type) + std::string(message);
    else
      counter_unique_name = std::string(type) + std::string(file) + std::string(line);

    size_t result = 0;
    const auto id = p_total_storage->name_w_ids[counter_unique_name];
    for (const size_t* client : p_total_storage->counters[id].clients)
      result += *client;
    return result;
    }

  void CounterUtils::SetTotalValue(const char* const* ip_counter, const size_t i_value)
    {
    std::lock_guard lock(p_total_storage->mutex);
    const char* type = ip_counter[0];
    const char* message = ip_counter[1];
    const char* file = ip_counter[2];
    const char* function = ip_counter[3];
    const char* line = ip_counter[4];

    std::string counter_unique_name;
    if (strlen(message) != 0)
      counter_unique_name = std::string(type) + std::string(message);
    else
      counter_unique_name = std::string(type) + std::string(file) + std::string(line);

    const auto id = p_total_storage->name_w_ids[counter_unique_name];
    for (size_t* client : p_total_storage->counters[id].clients)
      *client = 0;
    *p_total_storage->counters[id].clients.front() = i_value;
    }

  std::vector<std::reference_wrapper<CounterStorage>> GetAllCountersForPrinting()
  {
    CombineAllCounters();

    auto compare_string_with_numbers = [&](const std::string& left, const std::string& right) -> bool {
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

    std::vector<std::reference_wrapper<CounterStorage>> counters{ p_total_storage->counters.begin(), p_total_storage->counters.end() };
    std::sort(
      counters.begin(),
      counters.end(),
      [&](const std::reference_wrapper<CounterStorage> left, const std::reference_wrapper<CounterStorage> right) {
        return compare_string_with_numbers(left.get().sort_name, right.get().sort_name);
      });
    return counters;
  }

  void PrintCounter(CounterStorage& counter)
  {
    const std::int64_t value = counter.data;
    if (counter.category == "T")
      std::wcout << counter.label << L": " << std::chrono::microseconds(static_cast<long long>(value * GetInvFrequency())) << L"\n";
    if (counter.category == "C")
      std::wcout << counter.label << L": " << NumericRecord<std::int64_t>{ value } << L"\n";
  }

  void CounterUtils::ResetCounters(const char* ip_regexp, const char* ip_regexp_to_print)
  {
    std::lock_guard<std::recursive_mutex> lock(p_total_storage->mutex);
    if (p_total_storage->counters.empty())
      return;

    auto counters = GetAllCountersForPrinting();

    std::regex reg(ip_regexp);
    std::regex reg_to_print(ip_regexp_to_print);
    for (const auto& counter : counters) {
      if (std::regex_match(counter.get().label, reg) == false)
        continue;
      size_t& value = counter.get().data;
      if (std::regex_match(counter.get().label, reg_to_print))
        PrintCounter(counter.get());
      value = 0;
    }
  }

  void DisableThreadCountTracking()
  {
    is_thread_count_tracking_enabled = false;
  }

  void PrintAllCounters()
  {
    std::lock_guard<std::recursive_mutex> lock(p_total_storage->mutex);

    if (is_thread_count_tracking_enabled == false && p_total_storage->counters.empty())
      return;

    std::wcout << L"\n";
    if (is_thread_count_tracking_enabled)
      std::wcout << L"Num threads: " << thread_counter.load() << L"\n";

    if (p_total_storage->counters.empty())
      return;

    auto counters = GetAllCountersForPrinting();

    for (const auto& counter : counters)
      PrintCounter(counter.get());
    std::wcout << L"\n";
  }

  void CounterUtils::RegisterCounter(const char* const* ip_counter, size_t* ip_client)
  {
    std::lock_guard lock(p_total_storage->mutex);

    const char* type = ip_counter[0];
    const char* message = ip_counter[1];
    const char* file = ip_counter[2];
    const char* function = ip_counter[3];
    const char* line = ip_counter[4];

    std::string counter_unique_name;
    std::string counter_label;
    std::string counter_sort_name;
    if (strlen(message) != 0) {
      counter_unique_name = std::string(type) + std::string(message);
      counter_label = std::string(message);
      counter_sort_name = "a" + counter_label;
    }
    else {
      counter_unique_name = std::string(type) + std::string(file) + std::string(line);
      counter_label = std::string(function) + std::string(line);
      counter_label = std::regex_replace(counter_label, std::regex("`anonymous-namespace\'"), "{}");
      counter_label = std::regex_replace(counter_label, std::regex("::"), ".");
      counter_sort_name = "b" + counter_unique_name;
    }

    auto it = p_total_storage->name_w_ids.find(counter_unique_name);
    if (it == p_total_storage->name_w_ids.end()) {
      const size_t id = p_total_storage->counters.size();

      p_total_storage->counters.push_back({ counter_label, counter_unique_name, type, counter_sort_name });

      p_total_storage->counters.back().clients.emplace_back(ip_client);
      p_total_storage->name_w_ids[counter_unique_name] = id;
    }
    else {
      p_total_storage->counters[it->second].clients.emplace_back(ip_client);
    }
  }

  void CounterUtils::UnRegisterCounter(size_t* ip_client)
  {
    std::lock_guard<std::recursive_mutex> lock(p_total_storage->mutex);
    for (auto& counter : p_total_storage->counters) {
      for (size_t* client : counter.clients)
        if (client == ip_client) {
          counter.data += *client;
          auto& clients = counter.clients;
          clients.erase(std::remove(clients.begin(), clients.end(), ip_client), clients.end());
          return;
        }
    }
  }

  void*& CounterUtils::RegisterGlobalVariable(const char* const* ip_counter)
  {
    std::lock_guard lock(p_total_storage->mutex);

    // const char* type = ip_counter[0];
    const char* message = ip_counter[1];
    // const char* file = ip_counter[2];
    // const char* function = ip_counter[3];
    // const char* line = ip_counter[4];

    auto it = p_total_storage->global_variables.insert(GlobalVariable{ message }).first;
    return it->data;
  }
}

void TerminateHandler();
void SIGSEGHandler(int i_sig);

struct CoutFinalizer : PerfMonitor::internal::IObject
{
  std::unique_ptr<IObject> m_indentions_holder;
  std::unique_ptr<IObject> m_total_storage;
  std::unique_ptr<IObject> m_memory_watchers;

  CoutFinalizer()
  {
    m_indentions_holder = PerfMonitor::Indention::Initialize();
    m_total_storage = std::make_unique<TotalStorage>();
    m_memory_watchers = PerfMonitor::GetMemoryWatchers();
    p_total_storage = static_cast<TotalStorage*>(m_total_storage.get());
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
  try {
    std::rethrow_exception(current_exception);
  }
  catch (std::exception& ex) {
    std::cout << "  " << ex.what() << "\n";
  }
  std::cout << PerfMonitor::Color::LightGray;
  g_finalizer.~CoutFinalizer();
}
