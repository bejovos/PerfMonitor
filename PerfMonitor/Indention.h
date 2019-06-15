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

    struct Indent : internal::non_copyable, internal::non_moveable, internal::IObject, internal::convertable_to_bool_false
      {
      Indent(nullptr_t)
        {
        PushIndention(' ', this);
        }

      ~Indent() override
        {
        PopIndention();
        }
      };
    }
  }
