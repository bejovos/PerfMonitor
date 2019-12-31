#include "Indention.h"
#include "WriteToStream.h"

#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <xlocbuf>
#include <windows.h>

// ReSharper disable CppInconsistentNaming
using namespace PerfMonitor::Indention;

namespace
  {
  struct Data : PerfMonitor::internal::IObject
    {
    std::vector<std::pair<char, IObject*>> m_indentions;    
    std::deque<char> m_unprinted_chars;
    Data();
    ~Data();
    };
  
  Data* g_data = nullptr;
  bool tab_needed = true;
  bool end_needed = false;
  }

namespace PerfMonitor
  {
  namespace Indention
    {
    std::unique_ptr<internal::IObject> Indention::Initialize()
      {
      if (g_data != nullptr)
        throw std::logic_error("Not possible to initialize indentations twice");
      auto data = std::make_unique<Data>();
      g_data = data.get();
      return data;
      }

    void ForceClear()
      {
      if (!g_data->m_indentions.empty())
        {
        std::cout << PerfMonitor::Color::Red;
        std::cout << "  [Execution aborted]\n";
        std::cout << PerfMonitor::Color::LightGray;
        }

      // Not a very elegant solution but during application termination this is the only thing we can do
      while (!g_data->m_indentions.empty())
        g_data->m_indentions.back().second->~IObject();
      }

    bool SetEndNeeded(const bool i_end_needed)
      {
      const bool old_end_needed = end_needed;
      end_needed = i_end_needed;
      return old_end_needed;
      }

    void PushIndention(const char i_symbol, internal::IObject* ip_object)
      {
      g_data->m_indentions.emplace_back(i_symbol, ip_object);
      }

    void PopIndention()
      {
      g_data->m_indentions.pop_back();
      }

    template <class Char>
    class IndentFacet : public std::codecvt<Char, char, std::mbstate_t>
      {
      public:
        explicit IndentFacet(const size_t ref = 0)
          : std::codecvt<Char, char, std::mbstate_t>(ref)
          {
          }

        typedef std::codecvt_base::result result;
        typedef std::codecvt<Char, char, std::mbstate_t> parent;
        typedef typename parent::intern_type intern_type;
        typedef typename parent::extern_type extern_type;
        typedef typename parent::state_type state_type;

        static int& state(state_type& s) { return *reinterpret_cast<int*>(&s); }

      protected:
        result do_out(
          state_type&,
          const intern_type* rStart,
          const intern_type* rEnd,
          const intern_type*& rNewStart,
          extern_type* wStart,
          extern_type* wEnd,
          extern_type*& wNewStart) const override
          {
          result res = std::codecvt_base::ok;
          for (; rStart < rEnd; ++rStart)
            {
            if (end_needed)
              {
              end_needed = false;
              g_data->m_unprinted_chars.push_back('\n');
              tab_needed = true;
              }
            if (tab_needed)
              {
              tab_needed = false;
              for (auto& ind : g_data->m_indentions)
                g_data->m_unprinted_chars.emplace_back(ind.first);
              }
            g_data->m_unprinted_chars.push_back(static_cast<char>(*rStart));
            if (*rStart == '\n' || *rStart == '\r')
              tab_needed = true;
            }

          for (; !g_data->m_unprinted_chars.empty() && wStart < wEnd; 
            ++wStart, g_data->m_unprinted_chars.pop_front())
            *wStart = g_data->m_unprinted_chars.front();

          if (wStart != wEnd)
            res = std::codecvt_base::partial;

          rNewStart = rStart;
          wNewStart = wStart;

          return res;
          }

        // Override so the do_out() virtual function is called.
        bool do_always_noconv() const throw() override
          {
          return false; // Sometime we add extra tabs
          }
      };
    }
  }

Data::Data()
  {
  std::cout.imbue(std::locale(std::locale::classic(), new IndentFacet<char>()));
  std::cerr.imbue(std::locale(std::locale::classic(), new IndentFacet<char>()));
  std::clog.imbue(std::locale(std::locale::classic(), new IndentFacet<char>()));

  std::wcout.imbue(std::locale(std::locale::classic(), new IndentFacet<wchar_t>()));
  std::wcerr.imbue(std::locale(std::locale::classic(), new IndentFacet<wchar_t>()));
  std::wclog.imbue(std::locale(std::locale::classic(), new IndentFacet<wchar_t>()));
  }

Data::~Data()
  {
  std::cout.imbue(std::locale(std::locale::classic()));
  std::cerr.imbue(std::locale(std::locale::classic()));
  std::clog.imbue(std::locale(std::locale::classic()));

  std::wcout.imbue(std::locale(std::locale::classic()));
  std::wcerr.imbue(std::locale(std::locale::classic()));
  std::wclog.imbue(std::locale(std::locale::classic()));

  ForceClear();

  g_data = nullptr;
  }
