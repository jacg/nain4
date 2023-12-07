#include <arrow/api.h>
#include <arrow/array/builder_nested.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <arrow/result.h>

#include <arrow/type_fwd.h>
#include <parquet/arrow/writer.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

using arrow::DoubleBuilder;
using arrow::Int64Builder;
using arrow::ListBuilder;
using arrow::StructBuilder;

arrow::MemoryPool* pool = arrow::default_memory_pool();


#define DBG(stuff) std::cout << stuff << std::endl;
template<class T> void print(T data, std::ostream& out=std::cout) { out << data -> ToString() << std::endl; }


struct pair { double one, two; };

std::ostream& operator << (std::ostream& out, const pair& p) {
  return out << std::setw(0) << '[' << p.one << ", " << p.two << ']';
}

// --- d ---------------
auto struct_type = arrow::struct_({
  arrow::field("one", arrow::float64(), false),
  arrow::field("two", arrow::float64(), false)
});

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
  pair                d;
  std::vector<pair>   es;
};

std::shared_ptr<arrow::Schema> the_schema() {
  std::vector<std::shared_ptr<arrow::Field>> schema_vector = {
    arrow::field("a", arrow::int64(), false),
    arrow::field("b", arrow::int64(), false),

    arrow::field( "cs"
                , arrow::list(arrow::field( "c"
                                          , arrow::float64()
                                          , true))
                , false),

    arrow::field("d", struct_type, false),

    arrow::field( "es"
                , arrow::list( arrow::field( "e"
                                           , struct_type
                                           , true))
                , false)
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

template<class ListContentBuilder>
class list_builder {
  using InnerType = decltype(std::declval<ListContentBuilder>().GetValue(0));
public:

  list_builder(arrow::MemoryPool* pool = arrow::default_memory_pool())
  : build_outer{pool, std::make_shared<ListContentBuilder>(pool)}
  , build_inner{static_cast<ListContentBuilder*>(build_outer.value_builder())}
  {
  }

  arrow::Status Append(const std::vector<InnerType>& stuff) {
    ARROW_RETURN_NOT_OK(build_outer .  Append      (     ));
    ARROW_RETURN_NOT_OK(build_inner -> AppendValues(stuff));
    return arrow::Status::OK();
  }

  arrow::Result<std::shared_ptr<arrow::Array>> Finish() {
    return build_outer.Finish();;
  }

private:
  ListBuilder         build_outer;
  ListContentBuilder* build_inner;
};


arrow::Result<std::shared_ptr<arrow::Table>> vector_to_columnar_table(const std::vector<struct data_row>& rows) {

  Int64Builder                a_builder{pool};
  Int64Builder                b_builder{pool};
  list_builder<DoubleBuilder> c_builder{pool};
  StructBuilder               d_builder{struct_type, pool, {std::make_shared<DoubleBuilder>(pool), std::make_shared<DoubleBuilder>(pool)}};

  auto d_one_builder = static_cast<DoubleBuilder*>(d_builder.field_builder(0));
  auto d_two_builder = static_cast<DoubleBuilder*>(d_builder.field_builder(1));
  // --- e ---------------
  std::vector<std::shared_ptr<arrow::ArrayBuilder>> vec_of_builders {std::make_shared<DoubleBuilder>(pool), std::make_shared<DoubleBuilder>(pool)};
  ListBuilder e_builder{pool, std::make_shared<StructBuilder>(struct_type, pool, vec_of_builders)};

  StructBuilder* e_inner = static_cast<StructBuilder*>(e_builder.value_builder());

  auto e_one_builder = static_cast<DoubleBuilder*>(e_inner -> field_builder(0));
  auto e_two_builder = static_cast<DoubleBuilder*>(e_inner -> field_builder(1));

  // Now we can loop over our existing data and insert it into the builders. The
  // `Append` calls here may fail (e.g. we cannot allocate enough additional memory).
  // Thus we need to check their return values. For more information on these values,
  // check the documentation about `arrow::Status`.
  for (const data_row& row : rows) {
    ARROW_RETURN_NOT_OK(a_builder.Append(row.a ));
    ARROW_RETURN_NOT_OK(b_builder.Append(row.b ));
    ARROW_RETURN_NOT_OK(c_builder.Append(row.cs));

    ARROW_RETURN_NOT_OK(d_builder.Append());
    ARROW_RETURN_NOT_OK(d_one_builder->Append(row.d.one));
    ARROW_RETURN_NOT_OK(d_two_builder->Append(row.d.two));

    ARROW_RETURN_NOT_OK(e_builder.Append());
    for (auto e_pair: row.es) {
      ARROW_RETURN_NOT_OK(e_inner -> Append());
      ARROW_RETURN_NOT_OK(e_one_builder->Append(e_pair.one));
      ARROW_RETURN_NOT_OK(e_two_builder->Append(e_pair.two));
    }
  }

  // At the end, we finalise the arrays, declare the (type) schema and combine them
  // into a single `arrow::Table`:
  ARROW_ASSIGN_OR_RAISE(auto  a_array, a_builder.Finish());
  ARROW_ASSIGN_OR_RAISE(auto  b_array, b_builder.Finish());
  ARROW_ASSIGN_OR_RAISE(auto cs_array, c_builder.Finish()); // No need to invoke c_builder.Finish because it is implied by the parent builder's Finish invocation.
  ARROW_ASSIGN_OR_RAISE(auto  d_array, d_builder.Finish());
  ARROW_ASSIGN_OR_RAISE(auto es_array, e_builder.Finish());


  // The final `table` is the one we can then pass on to other functions that
  // can consume Apache Arrow memory structures. This object has ownership of
  // all referenced data, thus we don't have to care about undefined references
  // once we leave the scope of the function building the table and its
  // underlying arrays.
  return arrow::Table::Make(the_schema(), {a_array, b_array, cs_array, d_array, es_array});
}

template<class ListContentArray>
class vector_from_list {
public:

  vector_from_list(std::shared_ptr<arrow::Array> chunk)
      : list{std::static_pointer_cast<arrow::ListArray>(chunk)}
      , raw {std::static_pointer_cast<ListContentArray>(list -> values()) -> raw_values()}
    {}

  using RawListContentType =
      std::remove_const_t<
        std::remove_pointer_t<
          decltype(std::static_pointer_cast<ListContentArray>(std::make_shared<arrow::ListArray>() -> values()) -> raw_values())
        >
      >;
  std::vector<RawListContentType> Value(size_t i) {
    return std::vector<double> ( raw + list -> value_offset(i    )
                               , raw + list -> value_offset(i + 1));
  }
private:
  std::shared_ptr<arrow::ListArray> list;
  const RawListContentType* raw;
};

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

  auto as      = std::static_pointer_cast<arrow::Int64Array >(table -> column(0) -> chunk(0));
  auto bs      = std::static_pointer_cast<arrow::Int64Array >(table -> column(1) -> chunk(0));
  vector_from_list<arrow::DoubleArray> cs                    {table -> column(2) -> chunk(0)};

  auto ds   = std::static_pointer_cast<arrow::StructArray>(table -> column(3) -> chunk(0));
  auto ds_1 = std::static_pointer_cast<arrow::DoubleArray>(ds -> field(0));
  auto ds_2 = std::static_pointer_cast<arrow::DoubleArray>(ds -> field(1));

  // To enable zero-copy slices, the native values pointer might need to account
  // for this slicing offset. This is not needed for the higher level functions
  // like Value(…) that already account for this offset internally.
  std::vector<data_row> rows; rows.reserve(table -> num_rows());

  for (int64_t i = 0; i < table->num_rows(); i++) {
    // Another simplification in this example is that we assume that there are
    // no null entries, i.e. each row is filled with valid values.
    rows.push_back({
        as -> Value(i),
        bs -> Value(i),
        cs  . Value(i),
        { ds_1 -> Value(i), ds_2 -> Value(i)}

    });
  }

  return rows;
}

void print_rows(const std::vector<data_row>& rows) {
  std::cout
      << std::left << "A  "
      << std::left << "B  "
      << std::left << "Cs "
      << std::left << "D  "
      << std::left << "Es "
      << std::endl;
  for (const auto& row : rows) {
    std::cout
        << std::left << std::setw(3) << row.a
        << std::left << std::setw(3) << row.b;

    std::cout << " < ";
    for (const auto& cost : row.cs) { std::cout << std::left << std::setw(4) << cost; }
    std::cout << ">  ";

    std::cout
        << std::left << std::setw(3) << row.d;

    std::cout << " { ";
    for (const auto& cost : row.es) { std::cout << std::left << std::setw(20) << cost; }
    std::cout << "}";

    std::cout << std::endl;
  }
}


arrow::Status write_parquet(
  std::string filename,
  std::shared_ptr<arrow::Table> data,
  int64_t chunk_size = parquet::DEFAULT_MAX_ROW_GROUP_LENGTH
) {
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filename));
  PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*data, pool, outfile, chunk_size));
  return arrow::Status::OK();
}


#define DEFINE_WRITE_DATA_FUNCTION(INPUT_FORMAT)                                                                             \
arrow::Status write_data(std::shared_ptr<arrow::ipc::RecordBatchWriter> writer, std::shared_ptr<arrow::INPUT_FORMAT> data) { \
  ARROW_RETURN_NOT_OK(writer -> Write##INPUT_FORMAT(*data));                                                                 \
  return arrow::Status::OK();                                                                                                \
}

DEFINE_WRITE_DATA_FUNCTION(Table)
DEFINE_WRITE_DATA_FUNCTION(RecordBatch)

#undef DEFINE_WRITE_DATA_FUNCTION

#define DEFINE_WRITE_FORMAT_FUNCTION(OUTPUT_FORMAT, WRITER, INPUT_FORMAT) \
arrow::Status write_##OUTPUT_FORMAT(                                            \
  std::string filename,                                                         \
  std::shared_ptr<arrow::INPUT_FORMAT> data                                     \
) {                                                                             \
  std::shared_ptr<arrow::io::FileOutputStream> outfile;                         \
  ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filename));  \
  ARROW_ASSIGN_OR_RAISE(auto writer, arrow::WRITER(outfile, data -> schema())); \
  ARROW_RETURN_NOT_OK(write_data(writer, data));                                \
  ARROW_RETURN_NOT_OK(writer -> Close());                                       \
  return arrow::Status::OK();                                                   \
}
DEFINE_WRITE_FORMAT_FUNCTION(arrow, ipc::MakeFileWriter, Table)

#undef DEFINE_WRITE_FORMAT_FUNCTION



arrow::Status RunRowConversion() {
  std::vector<data_row> original_rows = {
 //  a  b  c                   d               e
    {1, 1, {10.0}            , { 1.23,  9.87}, {{1.11,1.12}
                                               ,{1.21,1.22}}},

    {2, 3, {11.0, 12.0, 13.0}, {22   , 23}   , {{2.11,2.12}
                                               ,{2.21,2.22}
                                               ,{2.31,2.32}}},

    {3, 2, {15.0, 25.0}      , {99   , 88}   , {{3.11,3.12}}}
  };
  std::shared_ptr<arrow::Table> table;
  std::vector<data_row> converted_rows;

  ARROW_ASSIGN_OR_RAISE(table, vector_to_columnar_table(original_rows));
  DBG("Table returned by vector_to_columnar_table");
  print(table);


  ARROW_RETURN_NOT_OK(write_parquet("complex-structure.parquet", table));
  ARROW_RETURN_NOT_OK(write_arrow  ("complex-structure.arrow"  , table));


  ARROW_ASSIGN_OR_RAISE(converted_rows, columnar_table_to_vector(table));

  assert(original_rows.size() == converted_rows.size());

  DBG("original ...") ; print_rows(original_rows);
  DBG("converted ..."); print_rows(converted_rows);

  return arrow::Status::OK();
}

int main(int argc, char** argv) {
  DBG("MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN ");
  auto status = RunRowConversion();
  if (!status.ok()) {
    std::cerr << status.ToString() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
