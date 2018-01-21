#pragma once

#include "_API.h"

namespace PerfMonitor
  {
  enum Color
    {
    Green = 10,
    LightGray = 7,
    Red = 12,
    Yellow = 14
    };

  PERFMONITOR_API __int64 InitTimeCounter();
  PERFMONITOR_API __int64 FinalizeTimeCounter(__int64);
  PERFMONITOR_API __int64 GetCurrentMemoryConsumption();
  PERFMONITOR_API __int64 GetPeakMemoryConsumption();
  PERFMONITOR_API void SetColor(Color i_color);
  PERFMONITOR_API double GetFrequency();
  }
