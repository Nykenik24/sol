#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Value.hpp"

namespace sol::runtime {

class Environment {
public:
  explicit Environment(std::shared_ptr<Environment> parent = nullptr)
      : _parent(std::move(parent)) {}

  void set(const std::string &name, const Value &value) {
    _values[name] = value;
  }

  bool contains(const std::string &name) const {
    return _values.contains(name);
  }

  Value get(const std::string &name) const {
    auto it = _values.find(name);

    if (it != _values.end())
      return it->second;

    if (_parent)
      return _parent->get(name);

    return {};
  }

private:
  std::unordered_map<std::string, Value> _values;
  std::shared_ptr<Environment> _parent;
};

} // namespace sol::runtime
