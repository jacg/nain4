#pragma once

#include <n4-sequences.hh>

#include <algorithm>
#include <numeric>
#include <optional>

namespace nain4 {
namespace stats {

template<class CONTAINER>
typename CONTAINER::value_type
sum(const CONTAINER& data) {
    return std::accumulate(begin(data), end(data),
                           static_cast<typename CONTAINER::value_type>(0));
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
mean(const CONTAINER& data) {
  if (data.empty()) { return {}; }
  return sum(data) / data.size();
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
std_dev_population(const CONTAINER& data) {
  auto average = stats::mean(data);
  if (! average.has_value()) { return {}; }
  auto mean = average.value();
  auto squared_deltas = n4::map<typename CONTAINER::value_type>(
      [mean](const auto x) { auto dx = x-mean; return dx*dx; },
      data
);
  return std::sqrt(sum(squared_deltas) / data.size());
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
std_dev_sample(const CONTAINER& data) {
  if (data.size() < 2) { return {}; }
  auto average = stats::mean(data);
  if (! average.has_value()) { return {}; }
  auto mean = average.value();
  auto squared_deltas = n4::map<typename CONTAINER::value_type>(
      [mean](const auto x) { auto dx = x-mean; return dx*dx; },
      data
);
  return std::sqrt(sum(squared_deltas) / (data.size() - 1));
}

}
} // namespace nain4



namespace n4 { using namespace nain4; }
