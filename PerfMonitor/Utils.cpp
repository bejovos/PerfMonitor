#include "Utils.h"

#include <windows.h>
#include <Psapi.h>

#undef min
#undef max

double frequency = 0;

namespace PerfMonitor
  {
  __int64 InitTimeCounter()
    {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
    }

  __int64 FinalizeTimeCounter(const __int64 counter)
    {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart - counter;
    }

  __int64 GetCurrentMemoryConsumption()
    {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc));
    return static_cast<__int64>(pmc.WorkingSetSize);
    }

  __int64 GetPeakMemoryConsumption()
    {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc));
    return static_cast<__int64>(pmc.PeakWorkingSetSize);    
    }

  void SetColor(const Color i_color)
    {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(i_color));
    }

  double GetFrequency()
    {
    return frequency;
    }
  }

struct FrequencyInitilization
  {
  FrequencyInitilization()
    {
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    frequency = double(li.QuadPart) / 1000000.0;
    }
  };

FrequencyInitilization frequency_initilization;