#include "Utils.h"

#include <windows.h>
#include <Psapi.h>

#undef min
#undef max

double frequency = 0;

namespace PerfMonitor {
  std::int64_t GetRealTime()
  {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
  }

  void SetColor(const Color i_color)
  {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(i_color));
  }

  double GetInvFrequency()
  {
    return frequency;
  }

  std::int64_t GetKernelTime()
  {
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    GetProcessTimes(GetCurrentProcess(),
                    &creation_time,
                    &exit_time, &kernel_time, &user_time);
    return LARGE_INTEGER{ kernel_time.dwLowDateTime,
      (long)kernel_time.dwHighDateTime }.QuadPart;
  }

  std::int64_t GetUserTime()
  {
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    GetProcessTimes(GetCurrentProcess(),
      &creation_time,
      &exit_time, &kernel_time, &user_time);
    return LARGE_INTEGER{ user_time.dwLowDateTime,
      (long)user_time.dwHighDateTime }.QuadPart;
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
