#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <unordered_map>
#include "page.hpp"
#include <filesystem>
#include <toml++/toml.h> 
#include <set>

namespace fs=std::filesystem;
struct styleProperties {
    std::string color="WHITE";
    bool bold=false;
    bool italic=false;
    bool underline=false;
};

struct sectionStyle {
    styleProperties command;
    styleProperties section;
    styleProperties title;
};

class Assembler {
  public:
    Assembler(const std::string& path="~/.config/huhTheBetterManPage");
    Page assemble(const Page& rhs);
  private:
    const std::string path;
    std::unordered_map<std::string, sectionStyle> styleLookup; // maps the name of the section to its style
    std::set<std::string> skippedSections;
    std::set<std::string> postpendSections;
    std::set<std::string> prependSections;
    sectionStyle defaultStyle;

    void loadConfig();
    Section style(const sectionStyle& style, const Section& section) const;
    sectionStyle parseSectionStyle(const toml::table& tbl, const sectionStyle& base);
    void loadStylesFromToml(const fs::path& path);
};

#endif // ASSEMBLER_H
