#pragma once

#include <functional>

#include "_API.h"

namespace PerfMonitor {

using Callback = std::function<void()>;

PERFMONITOR_API void RegisterCallback(Callback i_callback, std::string i_name);
PERFMONITOR_API void UnRegisterCallback(const std::string& i_name);
PERFMONITOR_API void InvokeCallback(const std::string& i_name);

}
