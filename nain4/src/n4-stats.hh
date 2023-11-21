#pragma once

#include <algorithm>
#include <numeric>

namespace nain4 {
namespace stats {

template<class CONTAINER>
typename CONTAINER::value_type
sum(const CONTAINER& data) { return std::accumulate(begin(data), end(data), 0.0); }

}
} // namespace nain4



namespace n4 { using namespace nain4; }
