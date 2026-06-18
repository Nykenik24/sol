#pragma once

#include "Token.hpp"
#include <string>

namespace sol::syntax::lex {
class Lexer {
public:
  Lexer() = default;
  Lexer(Lexer &&) = default;
  Lexer(const Lexer &) = default;
  Lexer &operator=(Lexer &&) = default;
  Lexer &operator=(const Lexer &) = default;
  ~Lexer() = default;

  TokenList lex(const std::string &input);

private:
};
} // namespace sol::syntax::lex
