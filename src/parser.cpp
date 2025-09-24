#include "parser.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#define BUFFER_SIZE 4096

using namespace std;
// takes in command and outputs the result of running the command
string Parser::getPage(const string &input) {
  FILE *ptr;
  cout << "input: " << input << endl;

  ptr = popen(input.c_str(), "r");
  if (ptr == nullptr) {
    throw runtime_error("popen() failed!");
  }

  string result;
  char *buffer = new char[BUFFER_SIZE];
  while (fgets(buffer, BUFFER_SIZE, ptr) != nullptr) {
    result += buffer;
  }
  delete[] buffer;

  if (pclose(ptr) == -1) {
    throw runtime_error("pclose() failed!");
  }
  return result;
}

std::vector<std::string> Parser::split(const string &s, char delimiter) {
  vector<string> tokens;
  string token;
  istringstream tokenStream(s);
  while (getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  cout << "finished split" << endl;
  cout << tokens.size() << endl;
  return tokens;
}

Page Parser::parseToPage(const string &rawText) {
  Page page = Page();
  vector<string> lines = split(rawText, '\n');
  Section currentSection = Section();
  std::string currentContent;
  for (auto line : lines) {
    if (line.empty()) {
      continue;
    } else if (line.substr(0, 1) != " ") {
      // this is a new section w/ title
      //
      // check if our previous block was valid. if so, add it
      if (!currentSection.getTitle().empty()) {
        currentSection.setContent(currentContent);
        page.addSection(currentSection);
        currentSection = Section();
        currentContent.clear();
      }
      currentSection.setTitle(line);
    } else {
      // this is part of the description
      //
      // simply add to the section
      if (currentSection.getTitle().empty()) {
        // we have content but no title, skip
        continue;
      }
      currentContent += '\n' + line;
    }
  }

  // add the last section if it exists
  if (!currentSection.getTitle().empty()) {
    currentSection.setContent(currentContent);
    page.addSection(currentSection);
  }
  return page;
}

// test the section parser on man ls
int main() {
  Parser Parser;
  string cmd = "man ls";
  auto strPage = Parser.getPage(cmd);
  auto result = Parser.parseToPage(strPage);

  result.printSections();
  return 0;
}
