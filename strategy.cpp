#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "handt.h"
#include "strategy.h"

int main() {

  std::vector<std::pair<std::string, std::vector<long>>> successes;

  // Read latest prices
  const auto &prices = handt::get_prices();
  const unsigned long window_size = 24 * 1;
  const unsigned long look_ahead = window_size * 3;
  unsigned long window_count = 0;
  const double target_percentage = 1.05;

  // Test strategies on each series
  for (const auto &p : prices)
    if (!p.series.empty()) {

      // Set up some iterators
      auto a = p.series.begin();
      auto b = std::next(p.series.begin(), window_size);
      auto c = std::next(p.series.begin(), look_ahead);

      // Back test all historic prices in fixed-size windows starting from the
      // oldest
      while (c < p.series.cend()) {

        // Find the max in the subsequent prices
        const auto max = *std::max_element(b, c);
        const auto spot = *std::prev(b);
        const auto target = target_percentage * spot;

        // Run the strategy library and record if the target has been achieved
        for (const auto &name : strategy::library(a, b)) {

          auto it =
              std::find_if(successes.rbegin(), successes.rend(),
                           [&name](const auto &s) { return s.first == name; });

          // Check if we have an entry for this strategy, if not create an empty
          // one and update the iterator
          if (it == successes.rend()) {
            successes.push_back({name, {}});
            it = successes.rbegin();
          }

          it->second.push_back(max > target ? 1 : 0);
        }

        // Nudge all iterators along
        std::advance(a, 1);
        std::advance(b, 1);
        std::advance(c, 1);

        ++window_count;
      }
    }

  // Sort the strategy summary
  std::sort(
      successes.begin(), successes.end(), [](const auto &a, const auto &b) {

        const auto a_return =
            100.0 * std::accumulate(a.second.cbegin(), a.second.cend(), 0) /
            a.second.size();
        const auto b_return =
            100.0 * std::accumulate(b.second.cbegin(), b.second.cend(), 0) /
            b.second.size();

        return a_return > b_return;
      });

  // Find the top strategies
  std::vector<std::string> popping_strategies;
  for (auto i = successes.cbegin();
       i != successes.cend() && i != std::next(successes.cbegin(), 13); ++i)
    popping_strategies.push_back(i->first);

  // Look over the most recent prices to find what's popping
  std::stringstream popping;
  for (const auto &p : prices)
    if (!p.series.empty()) {

      const auto a = std::prev(p.series.cend(), window_size);
      const auto b = p.series.cend();

      for (const auto &name : strategy::library(a, b))
        for (const auto &popper : popping_strategies)
          if (name.find(popper) != std::string::npos)
            popping << p.from_symbol << '-' << p.to_symbol << ' ' << name
                    << '\n';
    }

  // Calculate strategy summary
  std::stringstream strategy_summary;
  for (const auto &strat : successes) {
    const long orders = strat.second.size();
    const auto sum =
        std::accumulate(strat.second.cbegin(), strat.second.cend(), 0);
    const auto name = strat.first;
    strategy_summary << name << '\t' << std::setprecision(1) << std::fixed
                     << 100.0 * sum / orders << '\t' << orders << '\n';
  }

  // Report possible orders based on the best performing strategies
  std::cout << "\n# What's popping, bro?\n";
  std::cout << "Recent recommendations by the top"
               " performing stategies below. "
               "See the [raw price data](tmp/prices.csv)\n";
  std::cout << "<pre>\n";
  std::cout << (popping.str().empty() ? "I GOT NOTHING :(\n" : popping.str());
  std::cout << "</pre>\n";

  // Create strategy summary
  std::cout << "# Strategy performance\n";
  std::cout << "Strategies are sorted by percentage of orders that returned a "
               "profit of at least "
            << -100 + 100.0 * target_percentage << " % within "
            << look_ahead - window_size
            << " hours. "
               "The more orders the greater the confidence in the result.\n";
  std::cout << "* " << handt::get_pairs().size() << " pairs\n";
  std::cout << "* " << prices.size() << " series of prices\n";
  std::cout << "* " << window_size << " hours window size\n";
  std::cout << "* " << look_ahead - window_size << " hours look ahead\n";
  std::cout << "* " << window_count << " windows processed\n";
  std::cout << "<pre>\n";
  std::cout << "STRATEGY\t\t%\torders\n";
  std::cout << strategy_summary.str();
  std::cout << "</pre>\n";
}