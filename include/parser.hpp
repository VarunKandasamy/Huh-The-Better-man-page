#ifndef PARSER_H
#define PARSER_H

#include "page.hpp"

class Parser {
public:
  Page parseToPage(const std::string &rawText);
  std::vector<std::string> split(const std::string &s, char delimiter);
  std::string getPage(const std::string &input);
};

#endif // PARSER_H
