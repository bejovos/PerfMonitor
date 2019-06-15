#pragma once

namespace PerfMonitor
  {
  namespace internal
    {
    struct IObject
      {
        virtual ~IObject() = default;
      protected:
        bool is_valid = true;
      };    

    struct non_copyable
      {
      non_copyable() = default;
      non_copyable(const non_copyable&) = delete;
      non_copyable& operator =(const non_copyable&) = delete;
      };

    struct non_moveable
      {
      non_moveable() = default;
      non_moveable(const non_moveable&&) = delete;
      non_moveable&& operator =(const non_moveable&&) = delete;
      };

    struct convertable_to_bool_false
      {
      constexpr operator bool() const
        {
        return false;
        }
      };
    }
  }