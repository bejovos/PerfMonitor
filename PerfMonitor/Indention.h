#pragma once

#include "_API.h"

#include "IObject.h"

#include <cassert>
#include <memory>

namespace PerfMonitor
  {
  namespace Indention
    {
    PERFMONITOR_API bool SetEndNeeded(bool i_end_needed);
    PERFMONITOR_API void PushIndention(char i_symbol, internal::IObject*);
    PERFMONITOR_API void PopIndention();

    std::unique_ptr<internal::IObject> GetIndentionsHolder();
    std::unique_ptr<internal::IObject> GetStdStreamSwitcher();

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
