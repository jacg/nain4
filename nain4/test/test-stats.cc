#include "testing.hh"

#include <n4-stats.hh>

#include <unordered_set>

using namespace n4::stats;
using std::vector;
using std::unordered_set;
using std::sqrt;

TEST_CASE("stats sum", "[stats][sum]") {
  // Sums of empty containers
  CHECK     (sum(vector       <int>   {}) == 0);
  CHECK     (sum(unordered_set<int>   {}) == 0);
  CHECK_THAT(sum(vector       <float> {}), Within1ULP(0.f));
  CHECK_THAT(sum(unordered_set<double>{}), Within1ULP(0. ));

  // Sums of single-element containers
  CHECK     (sum(vector       <long> {3}) == 3);
  CHECK_THAT(sum(unordered_set<float>{4}), Within1ULP(4.f));

  // Sums of multiple-element containers
  CHECK_THAT(sum(vector       <float> {3.1, 7.2}), WithinULP (10.3f, 1)); // Avoid casting float to double
  CHECK_THAT(sum(vector       <double>{3.1, 7.2}), Within1ULP(10.3 ));
  CHECK(     sum(unordered_set<long>  {7, 2, 9} )  ==         18    );
}

TEST_CASE("stats mean", "[stats][mean]") {
  // Means of empty containers
  CHECK(! mean(vector    <double>{}).has_value());
  CHECK(! mean(unordered_set<int>{}).has_value());

  // Means of single-element containers
  CHECK_THAT(mean(vector      <double>{2.3 }).value(), Within1ULP( 2.3 ));
  CHECK_THAT(mean(unordered_set<float>{9.1f}).value(), Within1ULP( 9.1f));
  CHECK_THAT(mean(vector         <int>{42}  ).value(), Within1ULP(42.  ));

  // Means of multiple-value containers
  CHECK_THAT(mean(vector       <double>{1.0, 2.0}     ).value(), Within1ULP(1.5 ));
  CHECK_THAT(mean(vector       <float> {3.1, 3.6, 5.9}).value(), Within1ULP(4.2f));
  CHECK_THAT(mean(unordered_set<double>{9.0, 2.0}     ).value(), Within1ULP(5.5f));

  // Input integers give double results
  CHECK_THAT(mean(vector<int>{1,2}).value(), Within1ULP(1.5));
}

TEST_CASE("stats std_dev population", "[stats][std_dev][population]") {
  // Standard deviations of empty containers
  CHECK(! std_dev_population(vector       <int>   {}).has_value());
  CHECK(! std_dev_population(unordered_set<int>   {}).has_value());
  CHECK(! std_dev_population(vector       <float> {}).has_value());
  CHECK(! std_dev_population(unordered_set<double>{}).has_value());

  // Standard deviations of single-element containers
  CHECK_THAT(std_dev_population(vector      <double>{3.6}).value(), Within1ULP(0. ));
  CHECK_THAT(std_dev_population(unordered_set<float>{6.3}).value(), Within1ULP(0.f));

  // Standard deviations of multi-element containers
  auto check_std_and_var = [] (vector<double> data, double expected) {
    CHECK_THAT( std_dev_population(data).value(), Within1ULP(sqrt(expected)));
    CHECK_THAT(variance_population(data).value(), Within1ULP(     expected ));
  };
  check_std_and_var({5, 7}                                                ,  1);
  check_std_and_var({1, 2, 3, 4, 5}                                       ,  2);
  check_std_and_var({2, 4, 4, 6, 6, 6, 8, 8, 8, 8, 10, 10, 10, 12, 12, 14}, 10);

  // Input integers give double results
  CHECK_THAT( std_dev_population(vector<int>{1,2,3}).value(), Within1ULP(sqrt(2.0/3)));
  CHECK_THAT(variance_population(vector<int>{1,2,3}).value(), Within1ULP(     2.0/3 ));
}

TEST_CASE("stats std_dev sample", "[stats][std_dev][sample]") {
  // Standard deviations of empty containers
  CHECK(! std_dev_sample(vector       <int>   {}).has_value());
  CHECK(! std_dev_sample(unordered_set<int>   {}).has_value());
  CHECK(! std_dev_sample(vector       <float> {}).has_value());
  CHECK(! std_dev_sample(unordered_set<double>{}).has_value());

  // Standard deviations of single-element containers
  CHECK(! std_dev_sample(vector      <double>{4.2}).has_value());
  CHECK(! std_dev_sample(unordered_set<float>{7.9}).has_value());

  // Standard deviations of multi-element containers
  auto check_std_and_var = [] (vector<double> data, double expected) {
    CHECK_THAT( std_dev_sample(data).value(), Within1ULP(sqrt(expected)));
    CHECK_THAT(variance_sample(data).value(), Within1ULP(     expected ));
  };
  check_std_and_var({1, 5   },  8);
  check_std_and_var({2, 4   },  2);
  check_std_and_var({1, 2, 9}, 19);

  // Input integers give double results
  CHECK_THAT( std_dev_sample(vector<int>{1,2,3,4}).value(), Within1ULP(sqrt(5.0/3)));
  CHECK_THAT(variance_sample(vector<int>{1,2,3,4}).value(), Within1ULP(     5.0/3 ));
}

TEST_CASE("stats correlation", "[stats][correlation]") {
  auto corr = [] (const vector<double>& a, const vector<double>& b) {
    return n4::stats::correlation(a, b);
  };

  // Basic example of 100% correlation
  CHECK_THAT(corr({1,2},
                  {1,2}).value(), Within1ULP(1.0));

  // Basic example of 100% anti-correlation
  CHECK_THAT(corr({1,2},
                  {2,1}).value(), Within1ULP(-1.0));

  // Basic example of ZERO correlation
  CHECK_THAT(corr({-1,-1,+1,+1},
                  {-8,+8,-8,+8}).value(), Within1ULP(0.0));

  // A non-trivial example
  CHECK_THAT(corr({3,5,2,8,7},
                  {1,9,2,6,3}).value(), Within1ULP(0.4796356153459284));

  // Different lengths: no result
  CHECK(! corr({1,2,3},
               {1,2  }).has_value());

  // One sequence is constant: no result
  CHECK(! corr({1,2},
               {1,1}).has_value());

  // Too short: no result
  CHECK(! corr({1}, {1}).has_value());

  // Input integers give double results
  CHECK_THAT(n4::stats::correlation(
               vector<int>{1,2,3},
               vector<int>{1,2,2}).value(),
             Within1ULP(sqrt(3)/2));
}

TEST_CASE("stats min_max", "[stats][min_max]") {
  vector<double> empty{};
  CHECK(! n4::stats::min_max(empty).has_value());

  auto check_min_max = [] (const auto& data, const auto expected_min, const auto expected_max) {
    auto [min, max] = n4::stats::min_max(data).value();
    CHECK(min == expected_min);
    CHECK(max == expected_max);
  };

  vector<double> a {1.23, -9.62, 12.3, 4.56};
  check_min_max(a,             -9.62, 12.3);

  unordered_set<int> b {6,5,4,3};
  check_min_max(b, 3, 6);
}
