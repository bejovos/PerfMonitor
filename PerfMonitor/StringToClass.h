#pragma once

#include <utility>

namespace PerfMonitor {
  // String<'h', 'e', 'l', 'l', 'o'>
  template <char... Chars>
  struct String
  {
    static constexpr char string[] = { Chars..., '\0' };

    static constexpr const char* Str()
    {
      return string;
    }
  };

  namespace internal {
    constexpr size_t strlen(const char* str)
    {
      size_t result = 0;
      while (*str != '\0') {
        ++result;
        ++str;
      }
      return result;
    }

    constexpr size_t find_backward(const char* str)
    {
      const size_t len = strlen(str);
      size_t i = len;
      for (; i >= 2; --i)
        if (str[i] == '<' && str[i - 1] == ':' && str[i - 2] == ':')
          return i - 2;
      return 0; 
    }

    template <class StringBuilder, class Indices>
    struct FancyStringImpl;

    template <class StringBuilder, size_t... Indices>
    struct FancyStringImpl<StringBuilder, std::index_sequence<Indices...> >
    {
      using type = String<StringBuilder::GetChar(Indices)..., '\0'>;
    };

    template <class StringHolder>
    struct FancyString
    {
      static constexpr const char* message = StringHolder::Str(); 
      static constexpr const char* function = StringHolder::Function(); 
      static constexpr const char* line = StringHolder::Line();
      static constexpr size_t message_length = strlen(message);
      static constexpr size_t function_length = strlen(function);
      static constexpr size_t line_length = strlen(line);
      static constexpr size_t function_length2 = find_backward(function);

      struct Builder
      {
        static constexpr size_t Length()
        {
          if (message_length != 0)
            return message_length;
          return function_length2 + line_length;
        }
        static constexpr char GetChar(size_t v)
        {
          if (message_length != 0)
            return message[v];
          if (v < function_length2)
            return function[v];
          return line[v - function_length2];
        }
      };

      using type = typename FancyStringImpl<Builder, std::make_index_sequence<Builder::Length()>>::type;
    };

    template <template <class> class Result, class Str, class... Args>
    [[nodiscard]] Result<typename FancyString<Str>::type> MakeFromFancyString(const Str&, Args&&... i_args)
    {
      return { std::forward<Args>(i_args)... };
    }

    template <class Result>
    Result MakeFromNothing(std::nullptr_t)
    {
      return {};
    }

  }
}
