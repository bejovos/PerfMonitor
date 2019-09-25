#pragma once

#include <utility>

namespace PerfMonitor
  {
  // String<'h', 'e', 'l', 'l', 'o'>
  template <char... Chars> struct String
    {
      static constexpr const char* Str()
        {
        static constexpr char string[] = { Chars..., '\0' };
        return string;
        }
    };

  namespace internal
    {
    constexpr size_t strlen(const char* string)
      {
      size_t length = 0;
      while (string[length] != '\0')
        ++length;
      return length;
      }

    constexpr size_t find_backward(const char* string, const char* substring)
      {
      long string_length = static_cast<long>(strlen(string));
      long substring_length = static_cast<long>(strlen(substring));
      for (long i = string_length - substring_length; i>=0; --i)
        {
        bool is_same = true;
        for (size_t j = 0; j != substring_length; ++j)
          if (string[i + j] != substring[j])
            {
            is_same = false;
            break;
            }
        if (is_same)
          return i;
        }
      return 0;
      }

    constexpr size_t find_forward(const char* string, const char* substring)
      {
      long string_length = static_cast<long>(strlen(string));
      long substring_length = static_cast<long>(strlen(substring));
      for (long i = 0; i<=string_length - substring_length; ++i)
        {
        bool is_same = true;
        for (size_t j = 0; j != substring_length; ++j)
          if (string[i + j] != substring[j])
            {
            is_same = false;
            break;
            }
        if (is_same)
          return i;
        }
      return 0;
      }

    template <class T, class Indices>
    struct StringToClassImpl;

    template <class T, size_t... Indices>
    struct StringToClassImpl<T, std::integer_sequence<size_t, Indices...> >
      {
        using type = String<(T::Str())[Indices]...>;
      };


    template <size_t value, class>
    struct AddToSequence;

    template <size_t value, size_t... values>
    struct AddToSequence<value, std::index_sequence<values...>>
      {
        using type = std::integer_sequence<size_t, (value + values)...>;
      };

    template <size_t from, size_t length>
    using make_index_sequence = typename AddToSequence<from, std::make_index_sequence<length>>::type;

    template <class String, class Prefix, class Suffix>
    struct FancyString
      {
        static constexpr size_t prefix_length = strlen(Prefix::Str());
        static constexpr size_t left = find_forward(String::Str(), Prefix::Str());
        static constexpr size_t right = find_backward(String::Str(), Suffix::Str());
        using type = typename StringToClassImpl<String, make_index_sequence<left + prefix_length, right - left - prefix_length>>::type;
      };

    template <template <class> class Result, class Str>
    Result<typename FancyString<Str, String<'c', 'd', 'e', 'c', 'l', ' '>, String<':',':','<'>>::type> MakeFromFancyString(const Str&)
      {
      return {};
      }
    }
  }



