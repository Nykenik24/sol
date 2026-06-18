#include "sol/error.hpp"
// #include "sol/parser/AstPrinter.hpp"
#include "sol/parser/Parser.hpp"
#include "sol/vm/Interpreter.hpp"
#include <fstream>
#include <stdlib.h>

#include <string>

std::string read_file(const std::string &path) {
  std::ifstream f(path);

  if (!f.is_open())
    sol::error::fatal("cant read {}\n", path);

  return std::string(std::istreambuf_iterator<char>(f),
                     std::istreambuf_iterator<char>());
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    sol::error::fatal("expected filename\n");

  auto source = read_file(argv[1]);

  sol::syntax::Parser parser;
  auto root = parser.parse(source);

  // sol::parse::ast::AstPrinter printer;
  // root->accept(printer);

  sol::runtime::Interpreter vm;
  vm.execute(std::move(root));

  return 0;
}
