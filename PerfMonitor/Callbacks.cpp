#include "Callbacks.h"

#include <map>

namespace PerfMonitor {

std::map<std::string, Callback> g_callbacks;

void RegisterCallback(Callback i_callback, std::string i_name)
  {
  g_callbacks.emplace(std::move(i_name), std::move(i_callback));
  }

void UnRegisterCallback(const std::string& i_name)
  {
  g_callbacks.erase(i_name);
  }

void InvokeCallback(const std::string& i_name)
  {
  auto it = g_callbacks.find(i_name);
  if (it != g_callbacks.end())
    it->second();
  }

}