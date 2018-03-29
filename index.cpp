#include "handt.h"
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

int main() {

  // Configure debug
  std::stringstream out;
  out.precision(10);
  out << std::boolalpha;
  out << std::fixed;

  out << R"(
<!DOCTYPE html>

<meta charset="UTF-8">
<meta name="robots" content="index,follow" />
<meta http-equiv="refresh" content="30" />
<link rel=icon href="favicon.ico" sizes="any">

<style>
body { font-family: sans-serif; }
</style>

<title>HANDT</title>
<h1>HAVE A NICE DAY TRADER</h1>

<p id="disclaimer">History is no indicator of future performance. Don't invest
what you can't afford to lose. Prices fetched periodically from <a
href="https://www.cryptocompare.com/api/" target="blah">CryptoCompare</a>. See
the documentation on <a href="https://deanturpin.github.io/handt"
target="blah">GitHub</a>.</p>

)";

  out << "<pre>\n";

  // Get recent prices
  const auto prices = handt::get_prices();
  out << prices.size() << " coins updated in the last minute\n";

  // Get the final set of positions after trading is complete
  const auto positions = handt::get_consolidated_positions();
  out << positions.size() << " consolidated positions\n\n";

  // Close all positions and split into cap size
  std::map<std::string, std::vector<double>> small_cap, big_cap;
  for (const auto &position : positions) {

    const auto strategy = position.strategy;
    const auto buy = position.buy_price;
    const auto sell = position.sell_price;
    const auto yield = buy > 0.0 ? sell / buy : 0.0;

    // Check if it's actually kind of a big deal
    if (buy < 10.0)
      small_cap[strategy].push_back(yield);
    else
      big_cap[strategy].push_back(yield);
  }

  // Print strategy summaries
  out << "STRATEGY\t\t POS\t% RETURN\n";
  for (const auto &strategy : {small_cap, big_cap}) {
    for (const auto &i : strategy) {
      const unsigned long positions_held = i.second.size();
      const double yield =
          100.0 * std::accumulate(i.second.cbegin(), i.second.cend(), 0.0) /
          positions_held;

      out << i.first << "\t" << positions_held << "\t" << yield << "\n";
    }

    out << "--\n";
  }

  out << "<pre>\n";
  std::cout << out.str();
}