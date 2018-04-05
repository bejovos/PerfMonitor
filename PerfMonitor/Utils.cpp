#include "Utils.h"

#include <windows.h>
#include <Psapi.h>

#undef min
#undef max

double frequency = 0;

namespace PerfMonitor
  {
  std::int64_t InitTimeCounter()
    {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
    }

  std::int64_t FinalizeTimeCounter(const std::int64_t counter)
    {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart - counter;
    }

  std::uint64_t GetCurrentMemoryConsumption()
    {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc));
    return pmc.WorkingSetSize;
    }

  std::uint64_t GetPeakMemoryConsumption()
    {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc));
    return pmc.PeakWorkingSetSize;    
    }

  void SetColor(const Color i_color)
    {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(i_color));
    }

  double GetInvFrequency()
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
    frequency = 1 / frequency;
    }
  };

FrequencyInitilization frequency_initilization;