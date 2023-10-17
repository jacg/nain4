#pragma once

#include <exception>
#include <string>

namespace nain4 {
namespace exceptions {

#define WRAP(NAME, FROM, MESSAGE)                              \
  "\n\n"                                                       \
  "########################################################\n" \
  "[n4::exceptions::" + NAME + "]\n\n"                         \
  "[raised by " + FROM + "]: " + MESSAGE + "\n"                \
  "########################################################\n" \
  "\n\n"

struct n4_exception : std::exception {
  n4_exception(const std::string& name, const std::string& from, const std::string& message)
  : message(WRAP(name, from, message)) {}
  const char* what() const noexcept { return message.c_str();}

private:
  std::string message;
};

#define N4_EXCEPTION(NAME)                                  \
struct NAME : n4_exception {                                \
  NAME(const std::string& from, const std::string& message) \
    : n4_exception(#NAME, from, message) {}                 \
};

N4_EXCEPTION(not_found)
N4_EXCEPTION(bad_cast)

#undef N4_EXCEPTION
#undef WRAP

} // namespace exceptions
} // namespace nain4

namespace n4{ using namespace nain4; }
