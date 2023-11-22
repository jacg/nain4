#pragma once

#include <n4-sequences.hh>

#include <algorithm>
#include <numeric>
#include <optional>

#include <boost/math/statistics/univariate_statistics.hpp>
#include <boost/math/statistics/bivariate_statistics.hpp>

namespace nain4 {
namespace stats {

template<class CONTAINER>
typename CONTAINER::value_type
sum(const CONTAINER& data) {
    return std::accumulate(cbegin(data), cend(data),
                           static_cast<typename CONTAINER::value_type>(0));
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
mean(const CONTAINER& data) {
  if (data.empty()) { return {}; }
  return boost::math::statistics::mean(data);
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
variance_population(const CONTAINER& data) {
  if (data.empty()) { return {}; }
  return boost::math::statistics::variance(data);
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
variance_sample(const CONTAINER& data) {
  if (data.size() < 2) { return {}; }
  return boost::math::statistics::sample_variance(data);
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
std_dev_population(const CONTAINER& data) {
  auto var = variance_population(data);
  if (var.has_value()) { return std::sqrt(var.value()); } else { return {}; }
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
std_dev_sample(const CONTAINER& data) {
  auto var = variance_sample(data);
  if (var.has_value()) { return std::sqrt(var.value()); } else { return {}; }
}

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
correlation(const CONTAINER& a, const CONTAINER& b) {
  if (a.size() != b.size()) { return {}; }
  auto corr = boost::math::statistics::correlation_coefficient(a,b);
  if (std::isnan(corr)) { return {}; } else { return corr; }
}

}
} // namespace nain4



namespace n4 { using namespace nain4; }
