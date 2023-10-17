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

struct exception : std::exception {
  exception(const std::string& name, const std::string& from, const std::string& message)
  : message(WRAP(name, from, message)) {}
  const char* what() const noexcept { return message.c_str();}

private:
  std::string message;
};

#define EXCEPTION(NAME)                                     \
struct NAME : exception {                                   \
  NAME(const std::string& from, const std::string& message) \
    : exception(#NAME, from, message) {}                    \
};

EXCEPTION(not_found)
EXCEPTION(bad_cast)

#undef N4_EXCEPTION
#undef WRAP

} // namespace exceptions
} // namespace nain4

namespace n4{ using namespace nain4; }
