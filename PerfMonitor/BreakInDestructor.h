#pragma once

#include "_API.h"

namespace PerfMonitor
  {
  struct BreakInDestructor
    {
    BreakInDestructor()
      : m_value(false)
      , m_break(false)
      {}

    BreakInDestructor(const bool i_value)
      : m_value(i_value)
      , m_break(true)
      {}

    ~BreakInDestructor()
      {
      if (m_value == false && m_break == true)
        __debugbreak();
      }

    operator bool() const
      {
      return m_value;
      }

    bool m_value;
    bool m_break;
    };
  }


