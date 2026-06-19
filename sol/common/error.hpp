#pragma once

#include <cstdlib>
#include <format>
#include <iostream>
#include <source_location>
#include <string_view>

namespace sol::error {

namespace detail {

struct FormatWithLocation {
  std::string_view fmt;
  std::source_location loc;

  template <typename T>
  constexpr FormatWithLocation(
      const T &f, std::source_location l = std::source_location::current())
      : fmt(f), loc(l) {}
};

} // namespace detail

template <typename... Args>
[[noreturn]] void fatal(std::string_view fmt, Args &&...args) {
  std::cout << std::vformat(fmt, std::make_format_args(args...));
  std::cout.flush();
  std::exit(1);
}

template <typename... Args>
[[noreturn]] void fatal_internal(detail::FormatWithLocation fmt,
                                 Args &&...args) {
  std::cout << std::vformat(fmt.fmt, std::make_format_args(args...));

  std::cout << std::format(
      "\n\n============\n"
      "You've encountered an internal error, meaning it could've happened "
      "because of a memory issue, faulty code in sol's side, etc.\n\n"
      "Worry not, as this is not your fault (even if, sadly, you can't fix "
      "it). Report it immediately so it doesn't happen again at:\n"
      "- Nykenik24@proton.me\n"
      "- https://github.com/Nykenik24/sol/issues\n"
      "\n(raised at {}:{}, in `{}`)\n",
      fmt.loc.file_name(), fmt.loc.line(), fmt.loc.function_name());

  std::cout.flush();
  std::exit(1);
}

} // namespace sol::error
