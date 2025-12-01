#include <iostream>
#include <string>
#include "parser.hpp"
#include "assembler.hpp"
#include "page.hpp"

int main (int argc, char *argv[]) {
//   Parser parser;
//   std::string cmd;
//   std::cin >> cmd;
//   Page page = parser.parseToPage(cmd);
  if (argc < 2) {
      std::cerr << "Usage: huh <command>\n";
      return 1;
  }

  std::string cmd = argv[1];
  cmd = "man " + cmd;

  Parser Parser;
  auto strPage = Parser.getPage(cmd);
  Page page = Parser.parseToPage(strPage);

  Assembler assembler;
  Page newPage = assembler.assemble(page);

  // std::cout << "Styled Page:" << std::endl;
  newPage.printOutput();
  // std::cout << "\033[H" << std::flush;
  return 0;
}
