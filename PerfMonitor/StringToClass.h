#pragma once

namespace PerfMonitor
  {
  namespace internal
    {
    template <class... TString> struct String
      {
      static std::string MakeString()
        {
        const char ar[] = { (TString::MakeChar())... };
        return ar;
        }
      };
    template <char TChar> struct Char
      {
      static constexpr char MakeChar()
        {
        return TChar;
        }
      };

    constexpr unsigned c_strlen(char const* str, unsigned count = 0)
      {
      return ('\0' == str[0]) ? count : c_strlen(str + 1, count + 1);
      }

    template <class T, size_t index = c_strlen(T::Str()) + 1, class... TString>
    struct StringToClassImpl
      {
      using type = typename StringToClassImpl<T, index - 1,
        Char<(T::Str())[index - 1]>, TString...>::type;
      };

    template <class T, class... TString>
    struct StringToClassImpl<T, 0, TString...>
      {
      using type = String<TString...>;
      };

    template <class T>
    struct StringToClass
      {
      using type = typename StringToClassImpl<T>::type;
      };
    }
  }

#define STRING_TO_CLASS(string, class_name)                                               \
struct class_name_impl                                                                    \
  {                                                                                       \
    static constexpr const char * Str() { return string; }                                \
  };                                                                                      \
using class_name = typename PerfMonitor::internal::StringToClass<class_name_impl>::type

