#pragma once

#include "_API.h"

#include "IObject.h"

#include <cassert>

namespace PerfMonitor
  {
  namespace Indention
    {
    void Finalize();

    PERFMONITOR_API bool SetEndNeeded(const bool i_end_needed);
    PERFMONITOR_API void PushIndention(char i_symbol, internal::IObject*);
    PERFMONITOR_API void PopIndention();

    struct Indent : internal::non_copyable, internal::IObject, internal::convertable_to_bool_false
      {
      Indent()
        {
        PushIndention(' ', this);
        }

      Indent(Indent&& i_indent) noexcept
        {
        assert(i_indent.is_valid);
        i_indent.is_valid = false;
        PopIndention();
        PushIndention(' ', this);
        }

      ~Indent() override
        {
        if (is_valid == false)
          return;
        is_valid = false;
        PopIndention();
        }
      };
    }
  }
