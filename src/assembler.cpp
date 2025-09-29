#include "assembler.hpp"
#include "formatter.hpp"
#include <filesystem>
#include <iostream>
#include <toml++/toml.h>

namespace fs=std::filesystem;

Assembler::Assembler(const std::string& path) : path(path) {
  loadConfig();
}

template<typename T>
T toml_value_or(const toml::table& tbl, const std::string& key, const T& fallback) {
    if (auto v = tbl[key].value<T>()) return *v;
    return fallback;
}

void Assembler::loadConfig() {
  std::string root = path;
  sectionStyle base;

  try {
      for (auto const& entry : fs::recursive_directory_iterator(root)) {
          if (entry.is_regular_file() && entry.path().extension() == ".toml") {
              try {
                  loadStylesFromToml(entry.path());
              } catch (const toml::parse_error& err) {
                  std::cerr << "Parse error in " << entry.path()
                            << " at " << err.source().begin
                            << ": " << err.description() << "\n";
              }
          }
      }
  }catch (const std::exception& e) {
      std::cerr << "Directory walk error: " << e.what() << "\n";
  }
}

// parse a single top-level table into a sectionStyle, merging with `base`
sectionStyle Assembler::parseSectionStyle(const toml::table& tbl, const sectionStyle& base) {
    sectionStyle result = base; // start from base to inherit missing fields

    // title
    if (auto t = tbl["title"].as_table()) {
        result.title.bold      = toml_value_or(*t, "bold",      result.title.bold);
        result.title.italic    = toml_value_or(*t, "italic",    result.title.italic);
        result.title.underline = toml_value_or(*t, "underline", result.title.underline);
        result.title.color     = toml_value_or(*t, "color",     result.title.color);
    }

    // section (content)
    if (auto s = tbl["section"].as_table()) {
        result.section.bold      = toml_value_or(*s, "bold",      result.section.bold);
        result.section.italic    = toml_value_or(*s, "italic",    result.section.italic);
        result.section.underline = toml_value_or(*s, "underline", result.section.underline);
        result.section.color     = toml_value_or(*s, "color",     result.section.color);
    }

    // command
    if (auto c = tbl["command"].as_table()) {
        result.command.bold      = toml_value_or(*c, "bold",      result.command.bold);
        result.command.italic    = toml_value_or(*c, "italic",    result.command.italic);
        result.command.underline = toml_value_or(*c, "underline", result.command.underline);
        result.command.color     = toml_value_or(*c, "color",     result.command.color);
    }

    return result;
}

void Assembler::loadStylesFromToml(const fs::path& path) {
    auto tbl = toml::parse_file(path.string());

    // First pass: if there is a [default] table, merge it into defaultStyle
    for (auto&& [key, node] : tbl) {
        if (!node.is_table()) continue;
        if (key == "default") {
            // merge defaults into existing defaultStyle
            defaultStyle = parseSectionStyle(*node.as_table(), defaultStyle);
            std::cout << "Applied [default] from " << path << "\n";
            break; // only one default per file
        }
    }

    // Second pass: load other tables, merging each with the (possibly updated) defaultStyle
    for (auto&& [key, node] : tbl) {
        if (!node.is_table()) continue;
        if (key == "default") continue;

        std::string sectionName = key.str();
        sectionStyle s = parseSectionStyle(*node.as_table(), defaultStyle);
        sectionStyles[sectionName] = s;
        std::cout << "Loaded style for [" << sectionName << "] from " << path << "\n";
    }
}

// returns a new section with the given style applied
Seciton Assembler::style(const sectionStyle& style, const Section& section) const {
  Section styledSection = section;
  Formatter formatter;

  //first bold all sections
  if (style.title.bold) {
    styledSection.title = formatter::format(styledSection.title, Formatter::BOLD);
  }
  if(style.section.bold) {
    styledSection.content = formatter::format(styledSection.content, Formatter::BOLD);
  }
  // if(style.command.bold) {
  //   styledSection.commands = formatter::format(styledSection.commands, Formatter::BOLD);
  // }

  //then italicize all sections
  if (style.title.italic) {
    styledSection.title = formatter::format(styledSection.title, Formatter::ITALIC);
  }
  if(style.section.italic) {
    styledSection.content = formatter::format(styledSection.content, Formatter::ITALIC);
  }
  // if(style.command.italic) {
  //   styledSection.commands = formatter::format(styledSection.commands, Formatter::ITALIC);
  // }

  //then underline all sections
  if (style.title.underline) {
    styledSection.title = formatter::format(styledSection.title, Formatter::UNDERLINE);
  }
  if(style.section.underline) {
    styledSection.content = formatter::format(styledSection.content, Formatter::UNDERLINE);
  }
  // if(style.command.underline) {
  //   styledSection.commands = formatter::format(styledSection.commands, Formatter::UNDERLINE);
  // }

  //finally color all sections
  styledSection.title = formatter::format(styledSection.title, style.title.color);
  styledSection.content = formatter::format(styledSection.content, style.section.color);
  // styledSection.commands = formatter::format(styledSection.commands, style.command.color);

  // reset all styles at the end of each section
  styledSection.title += Formatter::RESET;
  styledSection.content += Formatter::RESET;
  // styledSection.commands += Formatter::RESET;

  return styledSection;
}

// need to add hasSection and getSection methods to Page class
Page Assembler::assemble(const Page& rhs) {
  Page assembledPage;
  // Prepend sections
  for (const auto& sectionName : prependSections) {
    if (rhs.hasSection(sectionName) && !skippedSections.contains(sectionName)) {
      sectionStyle style = sectionStyles.count(sectionName) ? sectionStyles[sectionName] : defaultStyle;
      Section newSection = style(style, rhs.getSection(sectionName));
      assembledPage.addSection(newSection);
    }
  }

  // Main sections
  for (const auto& section : rhs.getSections()) {
    if (prependSections.contains(section.name) || postpendSections.contains(section.name) || skippedSections.contains(section.name)) {
      continue; // Skip sections that are already handled or skipped
    }
    sectionStyle style = sectionStyles.count(section.name) ? sectionStyles[section.name] : defaultStyle;
    Section newSection = style(style, section);
    assembledPage.addSection(newSection);
  }

  // Append sections
  for (const auto& sectionName : postpendSections) {
    if (rhs.hasSection(sectionName) && !skippedSections.contains(sectionName)) {
      sectionStyle style = sectionStyles.count(sectionName) ? sectionStyles[sectionName] : defaultStyle;
      Section newSection = style(style, rhs.getSection(sectionName));
      assembledPage.addSection(newSection);
    }
  }

  return assembledPage;
}

