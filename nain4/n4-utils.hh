#ifndef N4_UTILS_HH
#define N4_UTILS_HH

#include <iterator>
#include <tuple>

template <typename T,
          typename TIter = decltype(std::begin(std::declval<T>())),
          typename       = decltype(std::  end(std::declval<T>()))>
constexpr auto enumerate(T&& iterable) {
  struct iterator {
    size_t i;
    TIter iter;
    bool operator != (const iterator& other) const { return this->iter != other.iter; }
    void operator ++ ()       {               ++i; ++iter; }
    auto operator *  () const { return std::tie(i,  *iter); }
  };
  struct iterable_wrapper {
    T iterable;
    auto begin() { return iterator{0, std::begin(iterable)}; }
    auto end  () { return iterator{0, std::  end(iterable)}; }
  };
  return iterable_wrapper{ std::forward<T>(iterable) };
}


#endif // N4_UTILS_HH
