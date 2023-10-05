#pragma once

#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>

#include <iterator>
#include <queue>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

// TODO This is breadth-first; how about depth-first?
// TODO This is an input iterator; how about output/forward?
// TODO Swich to C++ 20 and do it with concepts

namespace nain4 {
namespace test  {

class geometry_iterator {
public:
  geometry_iterator() {}
  geometry_iterator(G4VPhysicalVolume* v) { this->q.push(v); }
  geometry_iterator(G4LogicalVolume  * v) { queue_daughters(*v); }
  geometry_iterator(geometry_iterator const &) = default;
  geometry_iterator(geometry_iterator      &&) = default;

  using iterator_category = std::input_iterator_tag;
  using value_type        = G4VPhysicalVolume;
  using pointer           = value_type *; // TODO should this be pointer to pointer?
  using reference         = value_type * const &;
  using difference_type   = std::ptrdiff_t;

  geometry_iterator  operator++(int) { auto tmp = *this; ++(*this); return tmp; }
  geometry_iterator& operator++(   ) {
    if (!this->q.empty()) {
      auto current = this->q.front();
      this->q.pop();
      queue_daughters(*current->GetLogicalVolume());
    }
    return *this;
  }

  pointer   operator->()       { return this->q.front(); }
  reference operator* () const { return this->q.front(); }

  friend bool operator== (const geometry_iterator& a, const geometry_iterator& b) { return a.q == b.q; };
  friend bool operator!= (const geometry_iterator& a, const geometry_iterator& b) { return a.q != b.q; };

private:
  std::queue<G4VPhysicalVolume*> q{};

  void queue_daughters(G4LogicalVolume const& logical) {
    for(size_t d=0; d<logical.GetNoDaughters(); ++d) {
      this->q.push(logical.GetDaughter(d));
    }
  }
};

} // namespace test
} // namespace nain4

// By overloading begin and end, we can make G4PhysicalVolume
// usable with the STL algorithms and range-based for loops.
nain4::test::geometry_iterator begin(G4VPhysicalVolume&);
nain4::test::geometry_iterator   end(G4VPhysicalVolume&);
nain4::test::geometry_iterator begin(G4VPhysicalVolume*);
nain4::test::geometry_iterator   end(G4VPhysicalVolume*);

nain4::test::geometry_iterator begin(G4LogicalVolume&);
nain4::test::geometry_iterator   end(G4LogicalVolume&);
nain4::test::geometry_iterator begin(G4LogicalVolume*);
nain4::test::geometry_iterator   end(G4LogicalVolume*);


namespace n4 { using namespace nain4; }

#pragma GCC diagnostic pop
