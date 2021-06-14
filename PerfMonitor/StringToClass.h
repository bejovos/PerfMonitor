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

    constexpr size_t functionlen(const char* str)
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
    struct FancyStringImpl<StringBuilder, std::index_sequence<Indices...>>
    {
      using type = String<StringBuilder::GetChar(Indices)...>;
    };

    template <class StringHolder>
    struct FancyString
    {
      static constexpr const char* message = StringHolder::Str();
      static constexpr const char* file = StringHolder::File();
      static constexpr const char* function = StringHolder::Function();
      static constexpr const char* line = StringHolder::Line();
      static constexpr size_t message_length = strlen(message);
      static constexpr size_t file_length = strlen(file);
      static constexpr size_t function_length = functionlen(function);
      static constexpr size_t line_length = strlen(line);

      template <size_t Id>
      struct Builder
      {
        static constexpr size_t Length()
        {
          if (Id == 0)
            return message_length;
          if (Id == 1)
            return file_length;
          if (Id == 2)
            return function_length;
          return line_length;
        }

        static constexpr char GetChar(size_t v)
        {
          if (Id == 0)
            return message[v];
          if (Id == 1)
            return file[v];
          if (Id == 2)
            return function[v];
          return line[v];
        }
      };

      using message_type = typename FancyStringImpl<Builder<0>, std::make_index_sequence<Builder<0>::Length()>>::type;
      using file_type = typename FancyStringImpl<Builder<1>, std::make_index_sequence<Builder<1>::Length()>>::type;
      using function_type = typename FancyStringImpl<Builder<2>, std::make_index_sequence<Builder<2>::Length()>>::type;
      using line_type = typename FancyStringImpl<Builder<3>, std::make_index_sequence<Builder<3>::Length()>>::type;
    };

    template <template <class...> class Result, class Str, class... Args>
    [[nodiscard]] Result<
      typename FancyString<Str>::message_type,
      typename FancyString<Str>::file_type,
      typename FancyString<Str>::function_type,
      typename FancyString<Str>::line_type
    > MakeFromFancyString(const Str&, Args&&... i_args)
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
