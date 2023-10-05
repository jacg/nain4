#pragma once

#include <ios>
#include <fstream>

namespace nain4 {

// redirect to arbitrary stream or buffer
class redirect {
public:
  redirect(std::ios& stream, std::streambuf* new_buffer);
  redirect(std::ios& stream, std::ios&       new_stream);
  ~redirect();
private:
  std::streambuf* original_buffer;
  std::ios&       stream;
};

// redirect to /dev/null
class silence {
public:
  explicit silence(std::ios& stream);
  ~silence();
private:
  std::streambuf* original_buffer;
  std::ios&       stream;
  std::ofstream   dev_null;
};


} // namespace nain4

namespace n4 { using namespace::nain4; }
