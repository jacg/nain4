#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>

#include <iostream>
#include <memory>
#include <vector>

using namespace arrow;

#define DBG(stuff) std::cout << stuff << std::endl;

std::shared_ptr<Schema> the_schema() {
  auto nullable_list_element = false;
  auto nullable_list         = true;

  auto element_field = field( "item", int32()            , nullable_list_element);
  auto    list_field = field("items", list(element_field), nullable_list);

  return std::make_shared<Schema>(FieldVector{list_field});
}

Status do_it() {
  auto pool = default_memory_pool();

  auto  int_builder = std::make_shared<Int32Builder>(pool);
  auto list_builder = ListBuilder{pool, int_builder};

  std::vector<std::vector<int>> all_values{{0, 1}, {2}, {3,4,5}, {6,7,8,9}};
  for (const auto& values : all_values) {
    ARROW_RETURN_NOT_OK(list_builder. Append());
    ARROW_RETURN_NOT_OK( int_builder->AppendValues(values));
  }

  ARROW_ASSIGN_OR_RAISE(auto array, list_builder.Finish());
  auto table = Table::Make(the_schema(), {array});

  auto filename = "outputfile.parquet";
  ARROW_ASSIGN_OR_RAISE(auto outfile, io::FileOutputStream::Open(filename));
  DBG("ABOUT TO CRASH, MAYBE ....");
  PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*table, pool, outfile));
  DBG("HOORAY, WE DIDN'T CRASH!");

  return Status::OK();
}

int main() {
  auto status = do_it();
  if (!status.ok()) {
    std::cerr << status.ToString() << std::endl;
    return EXIT_FAILURE;
  }
  DBG("WE SURVIVED ALL THE WAY TO THE END");
  return EXIT_SUCCESS;
}
