#ifndef SECTION_H
#define SECTION_H

#include <string>

class Section {
public:
  Section(const std::string &title = "", const std::string &content = "");
  Section(const Section &other);
  Section &operator=(const Section &other);

  const std::string &getTitle() const;
  const std::string &getContent() const;
  void setTitle(const std::string &title);
  void setContent(const std::string &content);

private:
  std::string title;
  std::string content;
};
#endif // SECTION_H
