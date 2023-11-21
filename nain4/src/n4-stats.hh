#pragma once

#include <algorithm>
#include <numeric>
#include <optional>

namespace nain4 {
namespace stats {

template<class CONTAINER>
typename CONTAINER::value_type
sum(const CONTAINER& data) { return std::accumulate(begin(data), end(data), 0.0); }

template<class CONTAINER>
std::optional<typename CONTAINER::value_type> // TODO type trait gymnastics in case contained value is integral?
mean(const CONTAINER& data) {
  if (data.empty()) { return {}; }
  return sum(data) / data.size();
}

}
} // namespace nain4



namespace n4 { using namespace nain4; }
