#pragma once

#include <utility>

namespace PerfMonitor
  {
  namespace internal
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

    template <class T, class Indices>
    struct StringToClassImpl;

    template <class T, size_t... Indices>
    struct StringToClassImpl<T, std::integer_sequence<size_t, Indices...> >
      {
        using type = String<(T::Str())[Indices]...>;
      };

    template <class Source>
    using MakeString = typename StringToClassImpl<Source, std::make_index_sequence<strlen(Source::Str())>>; 

    template <class String, class SubString>
    struct BackwardTrim
      {
      static constexpr size_t index = find_backward(String::Str(), SubString::Str());
      using type = typename StringToClassImpl<String, std::make_index_sequence<index>>::type;
      };

    }
  }

#define STRING_TO_CLASS(string, class_name)                                               \
struct class_name ## _impl                                                                \
  {                                                                                       \
    static constexpr const char * Str() { return string; }                                \
  };                                                                                      \
using class_name = typename PerfMonitor::internal::MakeString<class_name ## _impl>::type




