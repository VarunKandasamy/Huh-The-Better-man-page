#include "formatter.hpp"

const std::string Formatter::RESET     = "\033[0m";
const std::string Formatter::BOLD      = "\033[1m";
const std::string Formatter::FAINT     = "\033[2m";
const std::string Formatter::ITALIC    = "\033[3m";
const std::string Formatter::UNDERLINE = "\033[4m";

const std::string Formatter::RED     = "\033[31m";
const std::string Formatter::GREEN   = "\033[32m";
const std::string Formatter::YELLOW  = "\033[33m";
const std::string Formatter::BLUE    = "\033[34m";
const std::string Formatter::MAGENTA = "\033[35m";
const std::string Formatter::CYAN    = "\033[36m";
const std::string Formatter::WHITE   = "\033[37m";

std::string Formatter::format(const std::string &text, const std::string &style) {
    return style + text + RESET;
}
