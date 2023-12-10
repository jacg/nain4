#pragma once

#include <n4-sequences.hh>

#include <boost/math/statistics/univariate_statistics.hpp>
#include <boost/math/statistics/bivariate_statistics.hpp>

#include <algorithm>
#include <numeric>
#include <optional>
#include <type_traits>

#define BSTATS boost::math::statistics

namespace nain4 {
namespace stats {

template<class CONTAINER>
typename CONTAINER::value_type
sum(const CONTAINER& data) {
    return std::accumulate(cbegin(data), cend(data),
                           static_cast<typename CONTAINER::value_type>(0));
}

template<class CONTAINER>
auto mean(const CONTAINER& data) -> std::optional<decltype(BSTATS::mean(data))> {
  if (data.empty()) { return {}; }
  return BSTATS::mean(data);
}

template<class CONTAINER>
auto variance_population(const CONTAINER& data) -> std::optional<decltype(BSTATS::variance(data))> {
  if (data.empty()) { return {}; }
  return BSTATS::variance(data);
}

template<class CONTAINER>
auto variance_sample(const CONTAINER& data) -> std::optional<decltype(BSTATS::sample_variance(data))>  {
  if (data.size() < 2) { return {}; }
  return BSTATS::sample_variance(data);
}

template<class CONTAINER>
auto std_dev_population(const CONTAINER& data) -> decltype(variance_population(data)) {
  auto var = variance_population(data);
  if (var.has_value()) { return std::sqrt(var.value()); } else { return {}; }
}

template<class CONTAINER>
auto std_dev_sample(const CONTAINER& data) -> decltype(variance_sample(data)) {
  auto var = variance_sample(data);
  if (var.has_value()) { return std::sqrt(var.value()); } else { return {}; }
}

template<class CONTAINER>
auto correlation(const CONTAINER& a, const CONTAINER& b) -> std::optional<decltype(BSTATS::correlation_coefficient(a,b))> {
  if (a.size() != b.size()) { return {}; }
  auto corr = BSTATS::correlation_coefficient(a,b);
  if (std::isnan(corr)) { return {}; } else { return corr; }
}

template<class CONTAINER>
auto min_max(const CONTAINER& data)
  -> std::optional<std::tuple<typename CONTAINER::value_type, typename CONTAINER::value_type>>
{
  if (data.empty()) { return {}; }
  auto min = *cbegin(data);
  auto max = *cbegin(data);
  std::for_each(++cbegin(data), cend(data), [&min, &max] (const auto& x) {
    min = std::min(x, min);
    max = std::max(x, max);
  });
  return {{min, max}};
}
}
} // namespace nain4

#undef BSTATS

namespace n4 { using namespace nain4; }
