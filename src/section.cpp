#include "section.hpp"
Section::Section(const std::string &title, const std::string &content)
    : title(title), content(content) {}
Section::Section(const Section &other)
    : title(other.title), content(other.content) {}
Section &Section::operator=(const Section &other) {
  if (this != &other) {
    title = other.title;
    content = other.content;
  }
  return *this;
}
const std::string &Section::getTitle() const { return title; }
const std::string &Section::getContent() const { return content; }
void Section::setTitle(const std::string &title) { this->title = title; }
void Section::setContent(const std::string &content) {
  this->content = content;
}
