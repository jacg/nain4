#include "parquet/properties.h"
#include <arrow/api.h>

#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <arrow/csv/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>


#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>

arrow::Status write_data(std::shared_ptr<arrow::ipc::RecordBatchWriter> ipc_writer, std::shared_ptr<arrow::Table> data) {
  ARROW_RETURN_NOT_OK(ipc_writer -> WriteTable(*data));
  return arrow::Status::OK();
}

arrow::Status write_data(std::shared_ptr<arrow::ipc::RecordBatchWriter> ipc_writer, std::shared_ptr<arrow::RecordBatch> data) {
  ARROW_RETURN_NOT_OK(ipc_writer -> WriteRecordBatch(*data));
  return arrow::Status::OK();
}

template<class T>
arrow::Status write_arrow(
  std::string filename,
  std::shared_ptr<T> data
) {
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filename));
  ARROW_ASSIGN_OR_RAISE(auto ipc_writer, arrow::ipc::MakeFileWriter(outfile, data -> schema()));
  ARROW_RETURN_NOT_OK(write_data(ipc_writer, data));
  ARROW_RETURN_NOT_OK(ipc_writer -> Close());
  return arrow::Status::OK();
}

template<class T>
arrow::Status write_csv(
  std::string filename,
  std::shared_ptr<T> data
) {
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filename));
  ARROW_ASSIGN_OR_RAISE(auto csv_writer, arrow::csv::MakeCSVWriter(outfile, data -> schema()));
  ARROW_RETURN_NOT_OK(write_data(csv_writer, data));
  ARROW_RETURN_NOT_OK(csv_writer -> Close());
  return arrow::Status::OK();
}

template<class T>
arrow::Status write_parquet(
  std::string filename,
  std::shared_ptr<T> data,
  int64_t chunk_size = parquet::DEFAULT_MAX_ROW_GROUP_LENGTH
) {
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filename));
  PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*data, arrow::default_memory_pool(), outfile, chunk_size));
  return arrow::Status::OK();
}

// template<class T>
// arrow::Status write_parquet(
//   std::string filename,
//   std::shared_ptr<T> data,
//   int64_t chunk_size = parquet::DEFAULT_MAX_ROW_GROUP_LENGTH
// ) {
//   std::shared_ptr<arrow::io::FileOutputStream> outfile;
//   ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filename));
//   PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*data, arrow::default_memory_pool(), outfile, chunk_size));
//   return arrow::Status::OK();
// }

  // std::shared_ptr<arrow::io::ReadableFile> infile;

  // ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open("test_in.arrow", arrow::default_memory_pool()));
  // ARROW_ASSIGN_OR_RAISE(auto ipc_reader, arrow::ipc::RecordBatchFileReader::Open(infile));
  // ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::RecordBatch> rbatch, ipc_reader -> ReadRecordBatch(0));



arrow::Status GenerateDataFiles() {
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

  auto field_day   = arrow::field("Day"  , arrow::int8 ());
  auto field_month = arrow::field("Month", arrow::int8 ());
  auto field_year  = arrow::field("Year" , arrow::int16());
  auto schema      = arrow::schema({field_day, field_month, field_year});

  auto rbatch = arrow::RecordBatch::Make(schema, days->length(), {days, months, years});
  std::cout << rbatch -> ToString() << std::endl;

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
  std::cout << table -> ToString() << std::endl;

  ARROW_RETURN_NOT_OK(write_arrow  ("test_in_table.arrow"  , table));
  ARROW_RETURN_NOT_OK(write_csv    ("test_in_table.csv"    , table));
  ARROW_RETURN_NOT_OK(write_parquet("test_in_table.parquet", table));

  ARROW_RETURN_NOT_OK(write_arrow  ("test_in_rbatch.arrow"  , rbatch));
  ARROW_RETURN_NOT_OK(write_csv    ("test_in_rbatch.csv"    , rbatch));
  //ARROW_RETURN_NOT_OK(write_parquet("test_in_rbatch.parquet", rbatch)); // TODO WriteRecordBatch not available for parquet?

  return arrow::Status::OK();
}



arrow::Status xxx() {
  std::shared_ptr<arrow::io::ReadableFile> infile;

  ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open("test_in.arrow", arrow::default_memory_pool()));
  ARROW_ASSIGN_OR_RAISE(auto ipc_reader, arrow::ipc::RecordBatchFileReader::Open(infile));
  ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::RecordBatch> rbatch, ipc_reader -> ReadRecordBatch(0));

  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open("test_out.arrow"));
  ARROW_ASSIGN_OR_RAISE(auto ipc_writer, arrow::ipc::MakeFileWriter(outfile, rbatch -> schema()));
  ARROW_RETURN_NOT_OK(ipc_writer -> WriteRecordBatch(*rbatch));
  ARROW_RETURN_NOT_OK(ipc_writer -> Close());

  // std::shared_ptr<arrow::Table> csv_table;
  // ARROW_ASSIGN_OR_RAISE(
  //   auto csv_reader,
  //   arrow::csv::TableReader::Make( arrow::io::default_io_context()
  //                                , infile
  //                                , arrow::csv::   ReadOptions::Defaults()
  //                                , arrow::csv::  ParseOptions::Defaults()
  //                                , arrow::csv::ConvertOptions::Defaults()));
  // ARROW_ASSIGN_OR_RAISE(csv_table, csv_reader -> Read());


}

int main() {

  arrow::Status st = GenerateDataFiles();
  if (!st.ok()) {
    std::cerr << st << std::endl;
    return EXIT_FAILURE;
  }

  st = xxx();
  if (!st.ok()) {
    std::cerr << st << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "OK ->" << std::endl;
}
