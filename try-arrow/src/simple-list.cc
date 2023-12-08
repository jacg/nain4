#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>

#include <iostream>
#include <memory>
#include <vector>

using namespace arrow;

#define DBG(stuff) std::cout << stuff << std::endl;
auto pool = default_memory_pool();

Status arrow_main() {

  // Want a table containing one field of lists, containing *NON-NULLABLE* elements
  auto element_type = int32()            ; auto element_field = field("item" , element_type, false);
  auto    list_type = list(element_field); auto    list_field = field("items",    list_type, true);

  // Build an array with the data to be placed in the table
  auto  int_builder = std::make_shared<Int32Builder>(pool);
  auto list_builder = ListBuilder{pool, int_builder};

  std::vector<std::vector<int>> all_values{{0, 1}, {2}, {3,4,5}, {6,7,8,9}};
  for (const auto& values : all_values) {
    ARROW_RETURN_NOT_OK(list_builder. Append());
    ARROW_RETURN_NOT_OK( int_builder->AppendValues(values));
  }
  ARROW_ASSIGN_OR_RAISE(auto array, list_builder.Finish());

  auto schema = std::make_shared<Schema>(FieldVector{list_field});

  auto table = Table::Make(schema, {array});      DBG(table -> ToString());

  // Try to write the table to parquet
  ARROW_ASSIGN_OR_RAISE(auto outfile, io::FileOutputStream::Open("out-cpp.parquet"));
  DBG("C++ CRASHES WHEN WRITING THE TABLE, BECAUSE OF INCONSISTENCY IN THE NULLABILITY OF THE LIST ELEMENTS");
  PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*table, pool, outfile));
  DBG("TABLE WRITTEN");

  return Status::OK();

}

int main() {
  auto status = arrow_main();
  if (!status.ok()) {
    std::cerr << status.ToString() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


// std::shared_ptr<Schema> the_schema() {
//   auto nullable_list_element = false;
//   auto nullable_list         = true;

//   auto element_field = field( "item", int32()            , nullable_list_element);
//   auto    list_field = field("items", list(element_field), nullable_list);

//   return std::make_shared<Schema>(FieldVector{list_field});
// }

// Status do_it() {
//   auto pool = default_memory_pool();

//   auto  int_builder = std::make_shared<Int32Builder>(pool);
//   auto list_builder = ListBuilder{pool, int_builder};

//   std::vector<std::vector<int>> all_values{{0, 1}, {2}, {3,4,5}, {6,7,8,9}};

//   for (const auto& values : all_values) {
//     ARROW_RETURN_NOT_OK(list_builder. Append());
//     ARROW_RETURN_NOT_OK( int_builder->AppendValues(values));
//   }

//   ARROW_ASSIGN_OR_RAISE(auto array, list_builder.Finish()); //DBG(array -> ToString());
//   auto table = Table::Make(the_schema(), {array});

//   DBG(table -> ToString());
//   // auto schema = table -> schema();
//   // DBG(schema -> ToString());

//   auto filename = "outputfile.parquet";
//   ARROW_ASSIGN_OR_RAISE(auto outfile, io::FileOutputStream::Open(filename));
//   DBG("ABOUT TO CRASH, MAYBE ....");
//   PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*table, pool, outfile));
//   DBG("HOORAY, WE DIDN'T CRASH!");

//   return Status::OK();
// }

// int ex_main() {
//   auto status = do_it();
//   if (!status.ok()) {
//     std::cerr << status.ToString() << std::endl;
//     return EXIT_FAILURE;
//   }
//   DBG("WE SURVIVED ALL THE WAY TO THE END");
//   return EXIT_SUCCESS;
// }

// // C++
// // items: list<item: int32 not null>
// //   child 0, item: int32 not null

// // pyarrow.Table
// // items: list<item: int32 not null> not null
// //   child 0, item: int32 not null
// // ----
// // items: [[[0,1],[2],[3,4,5],[6,7,8,9]]]
