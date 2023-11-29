#include <arrow/api.h>

#include <arrow/array/builder_nested.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <arrow/csv/api.h>
#include <arrow/type.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>

#define DBG(stuff) std::cout << stuff << std::endl;

struct vec { double x, y, z; };

//enum struct process { photoelectric, compton, rayleigh };

using process = unsigned;

// struct hit_sipm {
//   size_t sipm_id;
//   size_t photon_count;
// };

struct hit_e {
  vec position;
  double E;
  size_t n_photons;
};

struct hit_gamma {
  vec position;
  process kind;
  std::vector<hit_e> hits_e;
};

struct event {
  // TODO metadata  material, config, sizes etc. git hash
  // size_t event_id? // Is this necessary at all?
  vec                    primary_position;
  vec                    primary_momentum;
  std::vector<hit_gamma> hits_gamma;
//std::vector<hit_sipm>  hits_sipm;
};


std::vector<event> sample_of_events() {
  event e1 = {
    vec{1,2,3},
    vec{4,5,6},
    {
      hit_gamma{{1,1,1}, 1, {hit_e{vec{1.1, 1.1, 1.1}, 1.11, 11},
                             hit_e{vec{1.2, 1.2, 1.2}, 1.22, 12}}},
      hit_gamma{{2,2,2}, 2, {hit_e{vec{2.2, 2.2, 2.2}, 2.22, 22}}},
    }
  };
  event e2 = {
    vec{10,20,30},
    vec{40,50,60},
    {
      hit_gamma{{10,10,10}, 10, {hit_e{vec{10.1, 10.1, 10.1}, 10.11, 110},
                                 hit_e{vec{10.2, 10.2, 10.2}, 10.22, 12}}},
      hit_gamma{{20,20,20}, 20, {hit_e{vec{20.2, 20.2, 20.2}, 20.22, 220}}},
    }
  };
  return {e1, e2};
}

// =================================== Functions for writing data ==================================================
// 2 varieties of  input: 1. table  2. record batch
// 3 varieties of output: 1. Arrow  2. CSV  3. Parquet
// The following functions

#define DEFINE_WRITE_DATA_FUNCTION(INPUT_FORMAT)                                                                             \
arrow::Status write_data(std::shared_ptr<arrow::ipc::RecordBatchWriter> writer, std::shared_ptr<arrow::INPUT_FORMAT> data) { \
  ARROW_RETURN_NOT_OK(writer -> Write##INPUT_FORMAT(*data));                                                                 \
  return arrow::Status::OK();                                                                                                \
}

DEFINE_WRITE_DATA_FUNCTION(Table)
DEFINE_WRITE_DATA_FUNCTION(RecordBatch)

#undef DEFINE_WRITE_DATA_FUNCTION

// Writing to CSV and Arrow is very similar
#define DEFINE_WRITE_FORMAT_FUNCTION(OUTPUT_FORMAT, WRITER, INPUT_FORMAT)       \
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
// But writing to parquet looks a bit different. Seems to need the memory pool.
arrow::Status write_parquet(
  std::string filename,
  std::shared_ptr<arrow::Table> data,
  int64_t chunk_size = parquet::DEFAULT_MAX_ROW_GROUP_LENGTH
) {
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filename));
  PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*data, arrow::default_memory_pool(), outfile, chunk_size));
  return arrow::Status::OK();
}

DEFINE_WRITE_FORMAT_FUNCTION(arrow, ipc::MakeFileWriter, Table)
DEFINE_WRITE_FORMAT_FUNCTION(arrow, ipc::MakeFileWriter, RecordBatch)
DEFINE_WRITE_FORMAT_FUNCTION(csv  , csv::MakeCSVWriter , Table)
DEFINE_WRITE_FORMAT_FUNCTION(csv  , csv::MakeCSVWriter , RecordBatch)

#undef DEFINE_WRITE_FORMAT_FUNCTION

arrow::Result<std::shared_ptr<arrow::Table>> read_parquet(std::string filename) {
  std::shared_ptr<arrow::io::ReadableFile> infile;
  ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open(filename));
  std::unique_ptr<parquet::arrow::FileReader> reader;
  PARQUET_THROW_NOT_OK(parquet::arrow::OpenFile(infile, arrow::default_memory_pool(), &reader));
  std::shared_ptr<arrow::Table> table;
  PARQUET_THROW_NOT_OK(reader->ReadTable(&table));
  return arrow::Result<std::shared_ptr<arrow::Table>>(table);
}

arrow::Result<std::shared_ptr<arrow::RecordBatch>> read_arrow(std::string filename) {
  std::shared_ptr<arrow::io::ReadableFile> infile;
  ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open(filename, arrow::default_memory_pool()));
  ARROW_ASSIGN_OR_RAISE(auto ipc_reader, arrow::ipc::RecordBatchFileReader::Open(infile));
  // Using the reader, we can read Record Batches. Note that this is specific to IPC;
  // for other formats, we focus on Tables, but here, RecordBatches are used.
  std::shared_ptr<arrow::RecordBatch> rbatch;
  ARROW_ASSIGN_OR_RAISE(rbatch, ipc_reader -> ReadRecordBatch(0));
  return arrow::Result<std::shared_ptr<arrow::RecordBatch>>(rbatch);
}

arrow::Result<std::shared_ptr<arrow::Table>> read_csv(std::string filename) {
  std::shared_ptr<arrow::io::ReadableFile> infile;
  ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open("test_in.csv"));
  std::shared_ptr<arrow::Table> csv_table;
  ARROW_ASSIGN_OR_RAISE(
      auto csv_reader, arrow::csv::TableReader::Make(
        arrow::io::default_io_context(), infile,
        arrow::csv::   ReadOptions::Defaults(),
        arrow::csv::  ParseOptions::Defaults(),
        arrow::csv::ConvertOptions::Defaults()));
  std::shared_ptr<arrow::Table> table;
  ARROW_ASSIGN_OR_RAISE(table, csv_reader->Read())
  return arrow::Result<std::shared_ptr<arrow::Table>>(table);
}


// Helper for easier specification of arrow schemas. To be used in conjunction
// with helpers for creating fields. See field_helper_{1,2} below.
template<typename... FIELDS>
std::shared_ptr<arrow::Schema> make_schema(FIELDS&&... fields) {
  std::vector<std::shared_ptr<arrow::Field>> fields_vector;
  (..., fields_vector.push_back(std::forward<FIELDS>(fields)));
  return arrow::schema(fields_vector);
}

// Some typedefs to reduce noise
using DTT = std::shared_ptr<arrow::DataType>;
using FLD = std::shared_ptr<arrow::Field>;
using STR = std::string;
using VFD = std::vector<std::shared_ptr<arrow::Field>>;

namespace field_helper_1 {
  using arrow::field;
  // Need to write a function for creating a field of each and every Arrow type
  const FLD int8   (STR name)                    { return field(name, arrow::int8   (      )); }
  const FLD int16  (STR name)                    { return field(name, arrow::int16  (      )); }
  const FLD int32  (STR name)                    { return field(name, arrow::int32  (      )); }
  const FLD utf8   (STR name)                    { return field(name, arrow::utf8   (      )); }
  const FLD struct_(STR name, const VFD& fields) { return field(name, arrow::struct_(fields)); }
  const FLD list   (STR name, const DTT& dtype ) { return field(name, arrow::list   (dtype )); }
  // ... and so on for every Arrow type
};

namespace field_helper_2 {
  // Constructs a field by combining name, constructor function and any arguments the latter requires
  template<class FACTORY, class ...ARGS>
  const FLD field(STR name, FACTORY&& factory, ARGS&&... args) {
    return arrow::field(name, factory(std::forward<ARGS>(args)...));
  }
  // Can reduce verbosity by ensuring that unqualified constructors are in scope
  using arrow::int8;
  using arrow::int16;
  using arrow::int32;
  using arrow::uint8;
  using arrow::uint32;
  using arrow::uint64;
  using arrow::utf8;
  using arrow::float16;
  using arrow::struct_;
  using arrow::list;
  using arrow::large_list;
  // ... and so on for every Arrow type
}

void show_usage1() {
  using namespace field_helper_1;
  VFD some_fields;
  auto schema2 = make_schema(
    int8("A"),
    utf8("B"),
    struct_("C", some_fields));
}

void show_usage2() {
  using namespace field_helper_2;
  VFD some_fields;
  auto schema2 = make_schema(
    field("A", int8),
    field("B", utf8),
    field("C", struct_, some_fields)
  );
}

void show_usage2b() {
  using field_helper_2::field;
  VFD some_fields;
  auto schema2 = make_schema(
    field("A", arrow::int8),
    field("B", arrow::utf8),
    field("C", arrow::struct_, some_fields)
  );
}

void show_usage_without_field_helper() {
  VFD some_fields;
  make_schema(
    arrow::field("A", arrow::int8()),
    arrow::field("B", arrow::utf8()),
    arrow::field("C", arrow::struct_(some_fields))
 );
}

const FLD make_vec_field(const STR& name) {
  using namespace field_helper_2;
  static auto x = field("x", float16);
  static auto y = field("y", float16);
  static auto z = field("z", float16);
  static VFD vec_fields{x,y,z};
  return field(name, struct_, vec_fields);
}

const FLD make_hit_e_field(const STR& name) {
  using namespace field_helper_2;
  static auto position  = make_vec_field("position");
  static auto E         = field("E", float16);
  static auto n_photons = field("n_photons", uint64);
  static VFD hit_e_fields{position, E, n_photons};
  return field(name, struct_, hit_e_fields);
}

const FLD make_hit_gamma_field(const STR& name) {
  static auto position  = make_vec_field("position");
  static auto kind      = field("kind", arrow::uint8());
  static auto hits_e = arrow::field("hits_e", arrow::list(make_hit_e_field("e_field")));
  static VFD hit_e_fields{position, kind, hits_e};
  return arrow::field(name, arrow::struct_(hit_e_fields));
}

const FLD make_event_field(const STR& name) {
  static auto primary_position = make_vec_field("primary_position");
  static auto primary_momentum = make_vec_field("primary_momentum");
  static auto hits_gamma = arrow::field("hits_gamma", arrow::list(make_hit_gamma_field("hits_gamma")));
  static VFD event_fields{primary_position, primary_momentum, hits_gamma};
  return arrow::field(name, arrow::struct_(event_fields));
}

template<class T> void print(T data, std::ostream& out=std::cout) { out << data -> ToString() << std::endl; }

std::shared_ptr<arrow::Schema> make_crystal_output_schema() {
  return make_schema(arrow::field("Events", arrow::list(make_event_field("event"))));
}

arrow::Status generate_data_files() {
  arrow::Int8Builder int8builder;
  int8_t days_raw[5] = {1, 12, 17, 23, 28};
  ARROW_RETURN_NOT_OK(int8builder.AppendValues(days_raw, 5));
  std::shared_ptr<arrow::Array> days;
  ARROW_ASSIGN_OR_RAISE(days, int8builder.Finish());

  ARROW_RETURN_NOT_OK(int8builder.AppendValues({1, 3, 5, 7, 1}));
  std::shared_ptr<arrow::Array> months;
  ARROW_ASSIGN_OR_RAISE(months, int8builder.Finish());

  arrow::Int16Builder int16builder;
  int16_t years_raw[5] = {1999, 2000, 1995, 2000, 1995};
  ARROW_RETURN_NOT_OK(int16builder.AppendValues(years_raw, 5));
  std::shared_ptr<arrow::Array> years;
  ARROW_ASSIGN_OR_RAISE(years, int16builder.Finish());

  // Using field_helper_1
  auto schema = [] {
    using namespace field_helper_1;
    return make_schema(
      int8 ("Day")  ,
      int8 ("Month"),
      int16("Year") );
  }();

  // Using field_helper_2
 schema = [] {
   using namespace field_helper_2;
   return make_schema(
     field("Day"  , int8 ),
     field("Month", int8 ),
     field("Year" , int16));
  }();

  // Without field_helper
 schema = [] {
   return make_schema(
     arrow::field("Day"  , arrow::int8 ()),
     arrow::field("Month", arrow::int8 ()),
     arrow::field("Year" , arrow::int16()));
  }();

 auto rbatch = arrow::RecordBatch::Make(schema, days->length(), {days, months, years});
 //print(rbatch);

 ARROW_RETURN_NOT_OK(int8builder.AppendValues({6, 12, 3, 30, 22}));
 std::shared_ptr<arrow::Array> days2;
 ARROW_ASSIGN_OR_RAISE(days2, int8builder.Finish());

 ARROW_RETURN_NOT_OK(int8builder.AppendValues({5, 4, 11, 3, 2}));
 std::shared_ptr<arrow::Array> months2;
 ARROW_ASSIGN_OR_RAISE(months2, int8builder.Finish());

 ARROW_RETURN_NOT_OK(int16builder.AppendValues({1980, 2001, 1915, 2020, 1996}));
 std::shared_ptr<arrow::Array> years2;
 ARROW_ASSIGN_OR_RAISE(years2, int16builder.Finish());

 auto   day_chunks = std::make_shared<arrow::ChunkedArray>(arrow::ArrayVector{days  , days2  });
 auto month_chunks = std::make_shared<arrow::ChunkedArray>(arrow::ArrayVector{months, months2});
 auto  year_chunks = std::make_shared<arrow::ChunkedArray>(arrow::ArrayVector{years , years2 });

 auto table = arrow::Table      ::Make(schema, {day_chunks, month_chunks, year_chunks}, 10);
 //print(table);

 ARROW_RETURN_NOT_OK(write_arrow  ("test_in_table.arrow"  , table));
 ARROW_RETURN_NOT_OK(write_csv    ("test_in_table.csv"    , table));
 ARROW_RETURN_NOT_OK(write_parquet("test_in_table.parquet", table));

 ARROW_RETURN_NOT_OK(write_arrow  ("test_in_rbatch.arrow"  , rbatch));
 ARROW_RETURN_NOT_OK(write_csv    ("test_in_rbatch.csv"    , rbatch));
 //ARROW_RETURN_NOT_OK(write_parquet("test_in_rbatch.parquet", rbatch)); // TODO WriteRecordBatch not available for parquet?

  return arrow::Status::OK();
}

arrow::Status read_data_files() {

  ARROW_ASSIGN_OR_RAISE(auto  table, read_csv    ("test_in_table.csv"    ));  // print(table);
  ARROW_ASSIGN_OR_RAISE(      table, read_parquet("test_in_table.parquet"));     print(table);

  ARROW_ASSIGN_OR_RAISE(auto rbatch, read_arrow  ("test_in_rbatch.arrow" ));  // print(rbatch);
  ARROW_ASSIGN_OR_RAISE(      table, read_csv    ("test_in_rbatch.csv"   ));  // print(table);

  return arrow::Status::OK();

}

template <class BuilderType, class T>
arrow::Status append_values(BuilderType* builder, const std::vector<T>& values) {
  ARROW_RETURN_NOT_OK(builder -> AppendValues(values));
  return arrow::Status::OK();
}

template <class ValueType, class T>
arrow::Status append_list(arrow::ListBuilder* builder, const std::vector<std::vector<T>>& values) {
  auto values_builder = dynamic_cast<ValueType*>(builder -> value_builder());
  if (! values_builder) { throw "up"; }
  for (auto& list: values) {
    ARROW_RETURN_NOT_OK(builder -> Append());
    ARROW_RETURN_NOT_OK(append_values(values_builder, list));
  }
  return arrow::Status::OK();
}

std::shared_ptr<arrow::Schema> list_example_schema() {
  using namespace field_helper_1;
  return make_schema(
    int32("f0"),
    utf8 ("f1"),
    list ("f2", arrow::int8()));
}

arrow::Status list_example() {
  auto pool = arrow::default_memory_pool();
  auto schema = list_example_schema();

  std::vector<int32_t>             f0{0,1,2,3};
  std::vector<std::string>         f1{"zero", "one", "two", "three"};
  std::vector<std::vector<int8_t>> f2{{}, {0,1}, {}, {2}};

  std::shared_ptr<arrow::Array> a0, a1, a2;
  auto append_data = [&] (
    arrow::Int32Builder * b0,
    arrow::StringBuilder* b1,
    arrow::ListBuilder  * b2)
  {
    ARROW_RETURN_NOT_OK(append_values                   (b0, f0));
    ARROW_RETURN_NOT_OK(append_values                   (b1, f1));
    ARROW_RETURN_NOT_OK(append_list<arrow::Int8Builder> (b2, f2));
    return arrow::Status::OK();
  };

  arrow::Int32Builder  build_0;
  arrow::StringBuilder build_1;
  arrow::ListBuilder   build_2{pool, std::make_unique<arrow::Int8Builder>(pool)};

  ARROW_RETURN_NOT_OK(append_data(&build_0, &build_1, &build_2));
  ARROW_RETURN_NOT_OK(build_0.Finish(&a0));
  ARROW_RETURN_NOT_OK(build_1.Finish(&a1)); // ARROW_ASSIGN_OR_RAISE(a1, build_1.Finish());
  ARROW_RETURN_NOT_OK(build_2.Finish(&a2));
  auto rbatch = arrow::RecordBatch::Make(schema, a0->length(), {a0, a1, a2});
  //print(rbatch);

  // ARROW_RETURN_NOT_OK(write_csv("with-lists.csv", rbatch)); // Lists not supported by CSV, no surprise there
  ARROW_RETURN_NOT_OK(            write_arrow("with-lists.arrow", rbatch));
  ARROW_ASSIGN_OR_RAISE(auto data, read_arrow("with-lists.arrow"));
  //print(data);

  auto chunks_0 = std::make_shared<arrow::ChunkedArray>(arrow::ArrayVector{a0, a0});
  auto chunks_1 = std::make_shared<arrow::ChunkedArray>(arrow::ArrayVector{a1, a1});
  auto chunks_2 = std::make_shared<arrow::ChunkedArray>(arrow::ArrayVector{a2, a2});

  auto table = arrow::Table      ::Make(schema, {chunks_0, chunks_1, chunks_2});
  print(table);

  ARROW_RETURN_NOT_OK(               write_parquet("with-lists.parquet", table));
  ARROW_ASSIGN_OR_RAISE(auto table2, read_parquet("with-lists.parquet"));
  print(table2);

  return arrow::Status::OK();
}

arrow::Status crystal_io_proof_of_concept() {
  auto pool = arrow::default_memory_pool();
  auto event_schema = make_crystal_output_schema();

}


arrow::Status arrow_main() {
  ARROW_RETURN_NOT_OK(list_example());
  // ARROW_RETURN_NOT_OK(generate_data_files());
  // ARROW_RETURN_NOT_OK(    read_data_files());
  return arrow::Status::OK();
}

int main() {
  arrow::Status st = arrow_main();
  if (!st.ok()) {
    std::cerr << st << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "DONE" << std::endl;
}
