#include "Indention.h"
#include "WriteToStream.h"

#include <vector>
#include <deque>
#include <utility>
#include <xlocbuf>
#include <iomanip>
#include <iostream>

std::vector<std::pair<char, PerfMonitor::internal::IObject*>> indentions;
bool tab_needed = true;
bool end_needed = false;
std::deque<char> unprinted_chars;

namespace PerfMonitor
  {
  namespace Indention
    {
    void Finalize()
      {
      if (!indentions.empty())
        std::cout << "  " << ColoredValue<char*>(static_cast<char*>("[Execution aborted]\n"), Color::Red);

      while (!indentions.empty())
        indentions.back().second->~IObject();
      }

    bool SetEndNeeded(const bool i_end_needed)
      {
      const bool old_end_needed = end_needed;
      end_needed = i_end_needed;
      return old_end_needed;
      }

    void PushIndention(const char i_symbol, internal::IObject* ip_object)
      {
      indentions.emplace_back(i_symbol, ip_object);
      }

    void PopIndention()
      {
      indentions.pop_back();
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

        int& state(state_type& s) const { return *reinterpret_cast<int*>(&s); }

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
              unprinted_chars.push_back('\n');
              tab_needed = true;
              }
            if (tab_needed)
              {
              tab_needed = false;
              for (auto& ind : indentions)
                unprinted_chars.emplace_back(ind.first);
              }
            unprinted_chars.push_back(static_cast<char>(*rStart));
            if (*rStart == '\n' || *rStart == '\r')
              tab_needed = true;
            }

          for (; !unprinted_chars.empty() && wStart < wEnd; ++wStart, unprinted_chars.pop_front())
            *wStart = unprinted_chars.front();

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
  };

struct IndentionInitialization
  {
    ~IndentionInitialization()
      {      
      std::cout.imbue(std::locale(std::locale::classic()));
      std::cerr.imbue(std::locale(std::locale::classic()));
      std::clog.imbue(std::locale(std::locale::classic()));

      std::wcout.imbue(std::locale(std::locale::classic()));
      std::wcerr.imbue(std::locale(std::locale::classic()));
      std::wclog.imbue(std::locale(std::locale::classic()));
      }

    IndentionInitialization()
      {
      using namespace PerfMonitor::Indention;
      std::cout.imbue(std::locale(std::locale::classic(), new IndentFacet<char>()));
      std::cerr.imbue(std::locale(std::locale::classic(), new IndentFacet<char>()));
      std::clog.imbue(std::locale(std::locale::classic(), new IndentFacet<char>()));

      std::wcout.imbue(std::locale(std::locale::classic(), new IndentFacet<wchar_t>()));
      std::wcerr.imbue(std::locale(std::locale::classic(), new IndentFacet<wchar_t>()));
      std::wclog.imbue(std::locale(std::locale::classic(), new IndentFacet<wchar_t>()));
      }
  };

IndentionInitialization indention_initialization;