#include <arrow/api.h>
#include <arrow/result.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

using arrow::DoubleBuilder;
using arrow::Int64Builder;
using arrow::ListBuilder;

arrow::MemoryPool* pool = arrow::default_memory_pool();

// While we want to use columnar data structures to build efficient operations, we
// often receive data in a row-wise fashion from other systems. In the following,
// we want give a brief introduction into the classes provided by Apache Arrow by
// showing how to transform row-wise data into a columnar table.
//
// The data in this example is stored in the following struct:
struct data_row {
  int64_t             a;
  int64_t             b;
  std::vector<double> cs;
};

std::shared_ptr<arrow::Schema> the_schema() {
  std::vector<std::shared_ptr<arrow::Field>> schema_vector = {
    arrow::field("a" , arrow::int64()),
    arrow::field("b" , arrow::int64()),
    arrow::field("cs", arrow::list(arrow::float64()))
  };
  return std::make_shared<arrow::Schema>(schema_vector);
}

// Transforming a vector of structs into a columnar Table.
//
// The final representation should be an `arrow::Table` which in turn is made up
// of an `arrow::Schema` and a list of `arrow::ChunkedArray` instances. As the
// first step, we will iterate over the data and build up the arrays
// incrementally. For this task, we provide `arrow::ArrayBuilder` classes that
// help in the construction of the final `arrow::Array` instances.
//
// For each type, Arrow has a specially typed builder class. For the primitive
// values `a` and `b` we can use the `arrow::Int64Builder`. For the `cs` vector,
// we need to have two builders, a top-level `arrow::ListBuilder` that builds
// the array of offsets and a nested `arrow::DoubleBuilder` that constructs the
// underlying values array that is referenced by the offsets in the former
// array.

arrow::Result<std::shared_ptr<arrow::Table>> vector_to_columnar_table(const std::vector<struct data_row>& rows) {

  Int64Builder  a_builder{pool};
  Int64Builder  b_builder{pool};
  ListBuilder  cs_builder{pool, std::make_shared<DoubleBuilder>(pool)};
  // The following builder is owned by cs_builder.
  DoubleBuilder* c_builder = (static_cast<DoubleBuilder*>(cs_builder.value_builder()));

  // Now we can loop over our existing data and insert it into the builders. The
  // `Append` calls here may fail (e.g. we cannot allocate enough additional memory).
  // Thus we need to check their return values. For more information on these values,
  // check the documentation about `arrow::Status`.
  for (const data_row& row : rows) {
    ARROW_RETURN_NOT_OK( a_builder.Append(row.a));
    ARROW_RETURN_NOT_OK( b_builder.Append(row.b));

    ARROW_RETURN_NOT_OK(cs_builder.Append(     )); // Indicate the start of a new list row. This will memorise the current offset in the values builder.
    // Store the actual values. The same memory layout is
    // used for the component cost data, in this case a vector of
    // type double, as for the memory that Arrow uses to hold this
    // data and will be created.
    ARROW_RETURN_NOT_OK(c_builder -> AppendValues(row.cs));
  }

  // At the end, we finalise the arrays, declare the (type) schema and combine them
  // into a single `arrow::Table`:
  ARROW_ASSIGN_OR_RAISE(auto  a_array,  a_builder.Finish());
  ARROW_ASSIGN_OR_RAISE(auto  b_array,  b_builder.Finish());
  ARROW_ASSIGN_OR_RAISE(auto cs_array, cs_builder.Finish());
  // No need to invoke c_builder.Finish because it is implied by the parent builder's Finish invocation.

  // The final `table` is the one we can then pass on to other functions that
  // can consume Apache Arrow memory structures. This object has ownership of
  // all referenced data, thus we don't have to care about undefined references
  // once we leave the scope of the function building the table and its
  // underlying arrays.
  return arrow::Table::Make(the_schema(), {a_array, b_array, cs_array});
}

arrow::Result<std::vector<data_row>> columnar_table_to_vector(const std::shared_ptr<arrow::Table>& table) {
  // Check that the table's schema matches the expected schema
  if (!the_schema() -> Equals(*table->schema())) { return arrow::Status::Invalid("Schemas do not match"); }

  // Unpack the underlying arrays.
  //
  // For the primitive columns `a` and `b` we can use the high level functions
  // to get the values.
  //
  // For the nested column `cs` we need to access the C-pointer to the data to
  // copy its contents into the resulting `std::vector<double>`. Here we need to
  // be careful to also add the offset to the pointer. This offset is needed to
  // enable zero-copy slicing operations. While this could be adjusted
  // automatically for double arrays, this cannot be done for the accompanying
  // bitmap as often the slicing border would be inside a byte.

  auto as      = std::static_pointer_cast<arrow::Int64Array >(table   -> column(0) -> chunk(0));
  auto bs      = std::static_pointer_cast<arrow::Int64Array >(table   -> column(1) -> chunk(0));
  auto cs_list = std::static_pointer_cast<arrow::ListArray  >(table   -> column(2) -> chunk(0));
  auto cs_raw  = std::static_pointer_cast<arrow::DoubleArray>(cs_list -> values()) -> raw_values();
  // To enable zero-copy slices, the native values pointer might need to account
  // for this slicing offset. This is not needed for the higher level functions
  // like Value(…) that already account for this offset internally.
  std::vector<data_row> rows;

  for (int64_t i = 0; i < table->num_rows(); i++) {
    // Another simplification in this example is that we assume that there are
    // no null entries, i.e. each row is filled with valid values.
    int64_t             a = as               -> Value       (i);
    int64_t             b = bs               -> Value       (i);
    std::vector<double> cs( cs_raw + cs_list -> value_offset(i    )
                          , cs_raw + cs_list -> value_offset(i + 1));
    rows.push_back({a, b, cs});
  }

  return rows;
}

arrow::Status RunRowConversion() {
  std::vector<data_row> original_rows = {{1, 1, {10.0}}, {2, 3, {11.0, 12.0, 13.0}}, {3, 2, {15.0, 25.0}}};
  std::shared_ptr<arrow::Table> table;
  std::vector<data_row> converted_rows;

  ARROW_ASSIGN_OR_RAISE(table, vector_to_columnar_table(original_rows));
  ARROW_ASSIGN_OR_RAISE(converted_rows, columnar_table_to_vector(table));

  assert(original_rows.size() == converted_rows.size());

  // Print out contents of table, should get
  // A  B  Cs
  // 1  1  10
  // 2  3  11  12  13
  // 3  2  15  25
  std::cout
      << std::left << "A  "
      << std::left << "B  "
      << std::left << "Cs "
      << std::endl;
  for (const auto& row : converted_rows) {
    std::cout
        << std::left << std::setw(3) << row.a
        << std::left << std::setw(3) << row.b;
    for (const auto& cost : row.cs) { std::cout << std::left << std::setw(4) << cost; }
    std::cout << std::endl;
  }
  return arrow::Status::OK();
}

int main(int argc, char** argv) {
  auto status = RunRowConversion();
  if (!status.ok()) {
    std::cerr << status.ToString() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
