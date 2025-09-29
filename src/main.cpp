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

  Parser Parser;
  std::string cmd = "man ls";
  auto strPage = Parser.getPage(cmd);
  Page page = Parser.parseToPage(strPage);


  // std::cout << "Parsed Page:" << std::endl;
  // page.printSections();
  // std::cout << "------------------------------------------------" << std::endl;
  // std::cout << "Assembling Page" << std::endl;

  Assembler assembler;
  Page newPage = assembler.assemble(page);

  // std::cout << "Styled Page:" << std::endl;
  newPage.printOutput();
  return 0;
}
