#pragma once

#include "_API.h"

#include <cstdint>

namespace PerfMonitor
  {
  enum Color
    {
    LightGray = 7,
    Green = 10,
    Red = 12,
    Purple = 13,
    Yellow = 14
    };

  PERFMONITOR_API std::int64_t InitTimeCounter();
  PERFMONITOR_API std::int64_t FinalizeTimeCounter(std::int64_t);
  PERFMONITOR_API void SetColor(Color i_color);
  PERFMONITOR_API double GetInvFrequency();
  }
