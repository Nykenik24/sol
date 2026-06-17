#pragma once

#include "sol/types.hpp"
#include <string>
#include <vector>

namespace sol::lex {
enum class TokenType {
  Eof,
  Ident,
  String,
  Digit,
  HexDigit,

  VarArg,
  Concat,
  Label,
  Equal,
  LeftShift,
  RightShift,
  NotEqual,
  LessEqual,
  MoreEqual,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  BitXor,
  BitAnd,
  BitNot,
  BitOr,
  Len,
  Less,
  More,
  LeftParen,
  RightParen,
  LeftBracket,
  RightBracket,
  LeftBrace,
  RightBrace,
  Assign,
  Semicolon,
  Colon,
  Comma,
  Period,

  Function,
  Export,
  Repeat,
  Elseif,
  Return,
  Break,
  False,
  Local,
  Until,
  While,
  Else,
  Goto,
  Then,
  True,
  Var,
  And,
  End,
  For,
  Nil,
  Not,
  Do,
  If,
  In,
  Or,
};

class Token {
public:
  Token(TokenType type, const std::string &txt) : _type(type), _txt(txt) {};
  Token(Token &&) = default;
  Token(const Token &) = default;
  Token &operator=(Token &&) = default;
  Token &operator=(const Token &) = default;
  ~Token() = default;

  inline const TokenType type() const { return _type; };
  inline const std::string txt() const { return _txt; };
  inline void set_line(size line) { this->line = line; };
  inline size get_line() { return line; };

private:
  TokenType _type;
  std::string _txt;
  size line = 1;
};

class TokenStream {
public:
  TokenStream() = default;

  explicit TokenStream(std::vector<Token> tokens)
      : _tokens(std::move(tokens)) {}

  const Token &operator[](size i) const { return _tokens[i]; }

  inline unsigned long size() const { return _tokens.size(); }
  inline std::vector<Token> all() const { return _tokens; };

private:
  std::vector<Token> _tokens;
};

} // namespace sol::lex
