#ifndef FORMATTER_H
#define FORMATTER_H

#include <string>
class Formatter {
  public:
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string FAINT;
    static const std::string ITALIC;
    static const std::string UNDERLINE;

    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;

    std::string format(const std::string &text, std::string style);
  private:
};

#endif // FORMATTER_H
