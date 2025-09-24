#include "page.hpp"
#include <iostream>
#include <stdexcept>

Page::Page(std::vector<Section> sections, const std::string &title)
    : sections(std::move(sections)), title(title) {}
Page::Page(const Page &other) : sections(other.sections), title(other.title) {}
Page &Page::operator=(const Page &other) {
  if (this != &other) {
    sections = other.sections;
    title = other.title;
  }
  return *this;
}

Section &Page::getSection(int &index) {
  if (index < 0 || index >= sections.size()) {
    throw std::out_of_range("Index out of range");
  }
  return sections[index];
}

Section &Page::getSection(const std::string &title) {
  for (auto &section : sections) {
    if (section.getTitle() == title) {
      return section;
    }
  }
  throw std::invalid_argument("Section with the given title not found");
}

Section &Page::removeSection(int &index) {
  if (index < 0 || index >= sections.size()) {
    throw std::out_of_range("Index out of range");
  }
  Section &removedSection = sections[index];
  sections.erase(sections.begin() + index);
  return removedSection;
}

Section &Page::removeSection(const std::string &title) {
  for (auto it = sections.begin(); it != sections.end(); ++it) {
    if (it->getTitle() == title) {
      Section &removedSection = *it;
      sections.erase(it);
      return removedSection;
    }
  }
  throw std::invalid_argument("Section with the given title not found");
}

const std::size_t Page::getSectionCount() const { return sections.size(); }

void Page::printSections() {
  for (auto i = 0; i < getSectionCount(); ++i) {
    auto sec = getSection(i);
    std::cout << "Section Title: " << sec.getTitle() << std::endl;
    std::cout << "Section Content: " << sec.getContent() << std::endl;
    std::cout << std::endl;
    std::cout << "-----------------------------------------------------"
              << std::endl;
  }
}

void Page::setTitle(const std::string &title) { this->title = title; }

void Page::addSection(const Section &section) { sections.push_back(section); }
