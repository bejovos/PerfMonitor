#pragma once

#include "_API.h"

#include <cstdint>

namespace PerfMonitor
  {
  enum Color
    {
    Green = 10,
    LightGray = 7,
    Red = 12,
    Yellow = 14
    };

  PERFMONITOR_API std::int64_t InitTimeCounter();
  PERFMONITOR_API std::int64_t FinalizeTimeCounter(std::int64_t);
  PERFMONITOR_API std::uint64_t GetCurrentMemoryConsumption();
  PERFMONITOR_API std::uint64_t GetPeakMemoryConsumption();
  PERFMONITOR_API void SetColor(Color i_color);
  PERFMONITOR_API double GetFrequency();
  }
