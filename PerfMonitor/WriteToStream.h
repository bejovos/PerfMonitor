#pragma once

#include "Utils.h"

#include <codecvt>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>

template <class, size_t>
class MMVector;

namespace itk {
template <typename, unsigned int>
class Point;

template <unsigned int>
class Size;
template <unsigned int>
class Index;
template <typename, unsigned int>
class Vector;
}

namespace PerfMonitor
  {
  template <class T>
  struct ColoredValue
    {
    const T value;
    Color color;

    ColoredValue(T&& i_value, const Color i_color)
      : value(std::forward<T>(i_value))
      , color(i_color)
      {}
    };

  template <class T>
  ColoredValue<T> MakeColoredValue(T&& i_value, const Color i_color)
    {
    return ColoredValue<T>{std::forward<T>(i_value), i_color};
    }

  template <size_t DigitsAfterComma>
  struct ProgressValue
    {
      size_t value;
    };

  template <class Stream>
  Stream& operator <<(Stream& stream, const Color & i_value)
    {
    SetColor(i_value);
    return stream;
    }

  template <class T, class Stream>
  Stream& operator <<(Stream& stream, const ColoredValue<T>& i_value)
    {
    SetColor(i_value.color);
    stream << i_value.value;
    SetColor(Color::LightGray);
    return stream;
    }

  template <class T1, class T2>
  ColoredValue<T1> CheckEqual(T1&& t1, const T2& t2)
    {
    if (t1 != t2)
      return { std::forward<T1>(t1), Color::Red };
    return { std::forward<T1>(t1), Color::Green };
    }

  template <typename... Args>
  ColoredValue<std::tuple<Args...>> MakeColoredValue(int i_color, Args&& ... i_values)
    {
    return ColoredValue<std::tuple<Args...>>(std::forward_as_tuple(i_values...), i_color);
    }

  template <class Tuple, class Stream, size_t index>
  struct PrintTuple
    {
    static void Print(Stream& stream, const Tuple& i_tuple)
      {
      PrintTuple<Tuple, Stream, index - 1>::Print(stream, i_tuple);
      if (std::is_same<typename std::decay<typename std::tuple_element<index - 1,Tuple>::type>::type, Color>::value == false)
        stream << " ";
      stream << std::get<index>(i_tuple);
      }
    };

  template <class Tuple, class Stream>
  struct PrintTuple<Tuple, Stream, 0>
    {
    static void Print(Stream& stream, const Tuple& i_tuple)
      {
      stream << std::get<0>(i_tuple);
      }
    };

  template <class Stream, class... Args>
  Stream& operator <<(Stream& stream, const std::tuple<Args...>& i_tuple)
    {
    stream << std::boolalpha;
    PrintTuple<std::tuple<Args...>, Stream, sizeof...(Args) - 1>::Print(stream, i_tuple);
    return stream;
    }

  template <class Stream, class Type1, class Type2>
  Stream& operator <<(Stream& stream, const std::pair<Type1, Type2>& p)
    {
    stream << p.first << " " << p.second;
    return stream;
    }

  template <class Stream, class Type, size_t Count>
  Stream& operator <<(Stream& stream, const std::array<Type, Count>& ar)
    {
    for (auto & v : ar)
      stream << v << " ";
    return stream;
    }

  template <class Stream, class Type, size_t dim>
  Stream& operator <<(Stream& stream, const MMVector<Type, dim>& point)
    {
    auto precision = stream.precision();
    auto flags = stream.flags();
    stream << std::fixed << std::setprecision(2);
    for (size_t i=0; i<dim - 1; ++i)
      stream << point[i] << " ";
    stream << point[dim - 1];
    stream.flags(flags);
    stream.precision(precision);
    return stream;
    }

  template <class Stream, class Type, size_t dim>
  Stream& operator <<(Stream& stream, const itk::Point<Type, dim>& point)
    {
    auto precision = stream.precision();
    auto flags = stream.flags();
    stream << std::fixed << std::setprecision(2);
    for (size_t i=0; i<dim - 1; ++i)
      stream << point[i] << " ";
    stream << point[dim - 1];
    stream.flags(flags);
    stream.precision(precision);
    return stream;
    }

  template <class Stream, size_t Value>
  Stream& operator <<(Stream& stream, const ProgressValue<Value>& value)
    {
    auto precision = stream.precision();
    auto flags = stream.flags();
    stream << std::fixed << std::setprecision(Value);
    stream << value.value * std::pow(10, - int(Value)) << " %";
    stream.flags(flags);
    stream.precision(precision);
    return stream;
    }

  template <class Stream, size_t dim>
  Stream& operator <<(Stream& stream, const itk::Size<dim>& point)
    {
    for (unsigned int i=0; i<dim; ++i)
      stream << point[i] << " ";
    return stream;
    }

  template <class Stream, size_t dim>
  Stream& operator <<(Stream& stream, const itk::Index<dim>& point)
    {
    for (size_t i=0; i<dim; ++i)
      stream << point[i] << " ";
    return stream;
    }

  template <class Stream, class T, size_t dim>
  Stream& operator <<(Stream& stream, const itk::Vector<T, dim>& point)
    {
    for (size_t i=0; i<dim; ++i)
      stream << point[i] << " ";
    return stream;
    }

  inline std::wostream& operator <<(std::wostream& stream, const std::string& i_string)
    {
    stream << std::wstring{i_string.begin(), i_string.end()};
    return stream;
    }

  inline bool PrintIfArgsEmpty(const bool i_print_end, const char* i_file, const int i_line)
    {
    long i = static_cast<long>(strlen(i_file)) - 1;
    for (; i >= 0; --i)
      if (i_file[i] == '\\' || i_file[i] == '/')
        {
        std::cout << i_file + i + 1;
        break;
        }
    if (i == -1)
      std::cout << i_file;
    std::cout << "(" << i_line << ")";
    if (i_print_end)
      std::cout << "\n";
    SetColor(Color::LightGray);
    return true;
    }

  template <typename... Args>
  bool PrintIfArgsEmpty(const bool i_print_end, const char*, int, Args&& ... args)
    {
    std::wcout << std::forward_as_tuple(args...);
    if (i_print_end)
      std::wcout << L"\n";
    SetColor(Color::LightGray);
    return true;
    }
  }