#pragma once

#if PERFMONITOR_EXPORTS
#  define PERFMONITOR_API __declspec(dllexport)
#else
#  define PERFMONITOR_API __declspec(dllimport)
#endif
