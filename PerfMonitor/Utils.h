#pragma once

#include "_API.h"

#include <cstdint>
#include <chrono>

namespace PerfMonitor {
  enum Color
  {
    LightGray = 7,
    Green = 10,
    Red = 12,
    Purple = 13,
    Yellow = 14
  };

  PERFMONITOR_API std::int64_t GetRealTime();
  PERFMONITOR_API std::int64_t GetKernelTime();
  PERFMONITOR_API std::int64_t GetUserTime();
  PERFMONITOR_API void SetColor(Color i_color);
  PERFMONITOR_API double GetInvFrequency();
}
