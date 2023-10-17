#pragma once

#include <exception>
#include <string>

namespace nain4 {
namespace exceptions {

#define WRAP(NAME, FROM, MESSAGE)                              \
  "\n\n"                                                       \
  "########################################################\n" \
  "[n4::exceptions::" #NAME "]\n\n"                            \
  "[raised by " + FROM + "]: " + MESSAGE + "\n"                \
  "########################################################\n" \
  "\n\n"

#define N4_EXCEPTION(NAME)                                     \
struct NAME : std::exception {                                 \
  NAME(const std::string& from, const std::string& message)    \
  : message(WRAP(not_found, from, message)) {}                 \
  const char* what() const noexcept { return message.c_str();} \
                                                               \
private:                                                       \
  std::string message;                                         \
};

N4_EXCEPTION(not_found)
N4_EXCEPTION(bad_cast)

#undef N4_EXCEPTION
#undef WRAP

} // namespace exceptions
} // namespace nain4

namespace n4{ using namespace nain4; }
