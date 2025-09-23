#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#define BUFFER_SIZE 4096

using namespace std;
string getPage(string input) {
  FILE *ptr;
  input = "man " + input;
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
  if (pclose(ptr) == -1) {
    throw runtime_error("pclose() failed!");
  }
  return result;
}

std::vector<string> split(const string &s, char delimiter) {
  vector<string> tokens;
  string token;
  istringstream tokenStream(s);
  while (getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

int main() {
  string res = getPage("man");
  vector<string> res2 = split(res, '[][]');
  cout << res2 << endl;
  return 0
}
