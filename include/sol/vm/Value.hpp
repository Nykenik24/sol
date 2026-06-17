#pragma once

#include <memory>
#include <string>
#include <variant>

namespace sol::runtime {
class Interpreter;
struct Callable;

class Value {
public:
  using Storage = std::variant<std::monostate, bool, double, std::string,
                               std::shared_ptr<Callable>>;

  Value() : _v(std::monostate{}) {}
  Value(std::nullptr_t) : _v(std::monostate{}) {}
  Value(bool b) : _v(b) {}
  Value(double n) : _v(n) {}
  Value(std::string s) : _v(std::move(s)) {}
  Value(const char *s) : _v(std::string(s)) {}
  Value(std::shared_ptr<Callable> c) : _v(c) {}

  template <typename T> bool is() const {
    return std::holds_alternative<T>(_v);
  }

  template <typename T> T &as() { return std::get<T>(_v); }

  template <typename T> const T &as() const { return std::get<T>(_v); }

  const Storage &raw() const { return _v; }

  const std::string type_as_str() {
    if (is<std::monostate>())
      return "<null>";
    if (is<bool>())
      return "boolean";
    if (is<double>())
      return "number";
    if (is<std::string>())
      return "string";
    if (is<std::shared_ptr<Callable>>())
      return "callable";

    return "<unknown>";
  }

private:
  Storage _v;
};
} // namespace sol::runtime
