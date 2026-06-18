#include "sol/parser/lexer/Lexer.hpp"
#include "sol/error.hpp"
#include <string>

static bool is_alpha(uint8 c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_num(uint8 c) { return (c >= '0' && c <= '9'); }

static bool is_ws(uint8 c) {
  return (c == '\n' || c == ' ' || c == '\t' || c == '\r');
}

static bool is_hex(uint8 c) {
  return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || is_num(c);
}

#define INBOUNDS(input) ((i <= input.size()) && (i >= 0))
#define CAN_PUTC(input) ((i + 1 <= input.size()) && (i + 1 >= 0))

#define UNTERMINATED_STRING_ERROR                                              \
  {                                                                            \
    error::fatal("unterminated string at line {} near {}{}\n", line, term,     \
                 buf.c_str());                                                 \
  }

#define UNTERMINATED_ML_STRING_ERROR                                           \
  {                                                                            \
    error::fatal("unterminated multi-line string at line {} near [[{}\n",      \
                 line, buf.c_str());                                           \
  }

namespace sol::syntax::lex {
struct Mapped {
  TokenType tt;
  const std::string txt;
};

Mapped symtable[] = {
    {TokenType::VarArg, "..."},     {TokenType::Concat, ".."},
    {TokenType::Label, "::"},       {TokenType::Equal, "=="},
    {TokenType::LeftShift, "<<"},   {TokenType::RightShift, ">>"},
    {TokenType::NotEqual, "~="},    {TokenType::LessEqual, "<="},
    {TokenType::MoreEqual, ">="},   {TokenType::Add, "+"},
    {TokenType::Sub, "-"},          {TokenType::Mul, "*"},
    {TokenType::Div, "/"},          {TokenType::Mod, "%"},
    {TokenType::BitXor, "^"},       {TokenType::BitAnd, "&"},
    {TokenType::BitNot, "~"},       {TokenType::BitOr, "|"},
    {TokenType::Len, "#"},          {TokenType::Less, "<"},
    {TokenType::More, ">"},         {TokenType::LeftParen, "("},
    {TokenType::RightParen, ")"},   {TokenType::LeftBracket, "["},
    {TokenType::RightBracket, "]"}, {TokenType::LeftBrace, "{"},
    {TokenType::RightBrace, "}"},   {TokenType::Assign, "="},
    {TokenType::Semicolon, ";"},    {TokenType::Colon, ":"},
    {TokenType::Comma, ","},        {TokenType::Period, "."},
};

Mapped kwtable[] = {
    {TokenType::Function, "function"},
    {TokenType::Export, "export"},
    {TokenType::Repeat, "repeat"},
    {TokenType::Elseif, "elseif"},
    {TokenType::Return, "return"},
    {TokenType::Break, "break"},
    {TokenType::False, "false"},
    {TokenType::Local, "local"},
    {TokenType::Until, "until"},
    {TokenType::While, "while"},
    {TokenType::Else, "else"},
    {TokenType::Goto, "goto"},
    {TokenType::Then, "then"},
    {TokenType::True, "true"},
    {TokenType::Var, "var"},
    {TokenType::And, "and"},
    {TokenType::End, "end"},
    {TokenType::For, "for"},
    {TokenType::Nil, "nil"},
    {TokenType::Not, "not"},
    {TokenType::Do, "do"},
    {TokenType::If, "if"},
    {TokenType::In, "in"},
    {TokenType::Or, "or"},
};

TokenList Lexer::lex(const std::string &input) {
  std::vector<Token> tokens;

  uint64 line = 1;
  size i = 0;
  while (i < input.size()) {
    while (is_ws(input[i])) {
      if (input[i] == '\n')
        line++;
      i++;
    }

    if (input[i] == '-' && CAN_PUTC(input) && input[i + 1] == '-') {
      i += 2;
      while (INBOUNDS(input) && (input[i] != '\n')) {
        i++;
      }
      i++;
      line++;
      continue;
    }

    if (i >= input.size())
      break;

    if (input[i] == '[' && CAN_PUTC(input) && input[i + 1] == '[') {
      i += 2;
      if (!CAN_PUTC(input)) {
        error::fatal("unstarted multi-line string at line {}\n", line);
      }

      std::string buf;
      while (input[i] != ']') {
        if (input[i] == '\\') {
          i++;

          if (!CAN_PUTC(input))
            UNTERMINATED_ML_STRING_ERROR
        }

        buf.push_back(input[i]);
        i++;
      }

      i++;
      if (!CAN_PUTC(input))
        UNTERMINATED_ML_STRING_ERROR

      if (input[i] != ']')
        error::fatal("wrong terminator for multi-line string at line {}\n",
                     line);

      i++;

      Token tk(TokenType::String, buf);
      tk.set_line(line);
      tokens.push_back(tk);
      continue;
    }

    for (size j = 0; j < (sizeof symtable / sizeof symtable[0]); j++) {
      Mapped sym = symtable[j];
      uint64 len = sym.txt.size();
      if ((i + len <= input.size()) && (i + len >= 0)) {
        char buf[len + 1];
        for (size k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (sym.txt.compare(buf) == 0) {
          Token tk(sym.tt, sym.txt);
          tk.set_line(line);
          tokens.push_back(tk);
          i += len;
          goto continue_;
        }
      }
    }

    for (size j = 0; j < (sizeof kwtable / sizeof kwtable[0]); j++) {
      Mapped kw = kwtable[j];
      uint64 len = kw.txt.size();
      if ((i + len <= input.size()) && (i + len >= 0)) {
        char buf[len + 1];
        for (size k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (kw.txt.compare(buf) == 0) {
          char next = input[i + len];
          if (is_alpha(next) || is_num(next) || next == '_')
            continue;
          Token tk(kw.tt, kw.txt);
          tk.set_line(line);
          tokens.push_back(tk);
          i += len;
          goto continue_;
        }
      }
    }

    if (input[i] == '0' && CAN_PUTC(input) && input[i + 1] == 'x') {
      i += 2;
      if (!CAN_PUTC(input))
        error::fatal("unstarted hex literal at line {}\n", line);

      std::string buf;
      while (i < input.size() && is_hex(input[i])) {
        buf.push_back(input[i]);
        i++;
      }

      Token tk(TokenType::HexDigit, buf);
      tk.set_line(line);
      tokens.push_back(tk);
      continue;
    }

    if (is_num(input[i])) {
      std::string buf;

      while (CAN_PUTC(input) && is_num(input[i])) {
        buf.push_back(input[i]);
        i++;
      }

      if (CAN_PUTC(input)) {
        if (input[i] != '.')
          goto push;

        buf.push_back(input[i]);

        i++;
        if (!INBOUNDS(input))
          goto push;

        while (CAN_PUTC(input) && is_num(input[i])) {
          buf.push_back(input[i]);
          i++;
        }
      } else
        goto push;

    push: {
      Token tk(TokenType::Digit, buf);
      tk.set_line(line);
      tokens.push_back(tk);
      continue;
    }
    }

    if (input[i] == '"' || input[i] == '\'') {
      uint8 term = input[i];
      i++;

      if (!INBOUNDS(input))
        error::fatal("unterminated string at line {} near {}\n", line, term);

      std::string buf;
      while (CAN_PUTC(input) && input[i] != term && input[i] != '\n') {
        buf.push_back(input[i]);
        i++;
      }
      if (input[i] == term)
        i++;
      else
        UNTERMINATED_STRING_ERROR

      if (!INBOUNDS(input))
        UNTERMINATED_STRING_ERROR;

      Token tk(TokenType::String, buf);
      tk.set_line(line);
      tokens.push_back(tk);
      continue;
    }

    if (is_alpha(input[i]) || input[i] == '_') {
      std::string buf;
      buf.push_back(input[i]);

      if (CAN_PUTC(input)) {
        i++;
      } else {
        Token tk(TokenType::Ident, buf);
        tk.set_line(line);
        tokens.push_back(tk);
        continue;
      }

      while (CAN_PUTC(input) &&
             (is_alpha(input[i]) || is_num(input[i]) || input[i] == '_')) {
        buf.push_back(input[i]);
        i++;
      }

      Token tk(TokenType::Ident, buf);
      tk.set_line(line);
      tokens.push_back(tk);
      continue;
    }

    error::fatal("unrecognized character '{}' in line {}\n", input[i], line);

  continue_: {}
  }

  tokens.push_back(Token(TokenType::Eof, ""));
  return tokens;
}
}; // namespace sol::syntax::lex
