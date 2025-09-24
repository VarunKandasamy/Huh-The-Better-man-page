#ifndef PAGE_H
#define PAGE_H

#include "section.hpp"
#include <cstddef>
#include <string>
#include <vector>

class Page {
public:
  Page(std::vector<Section> sections = {}, const std::string &title = "");
  Page(const Page &other);
  Page &operator=(const Page &other);

  Section &getSection(int &index);
  Section &getSection(const std::string &title);
  Section &removeSection(int &index);
  Section &removeSection(const std::string &title);

  void setTitle(const std::string &title);
  void addSection(const Section &section);
  const std::size_t getSectionCount() const;
  void printSections();

private:
  std::string title;
  std::vector<Section> sections;
};

#endif // PAGE_H
