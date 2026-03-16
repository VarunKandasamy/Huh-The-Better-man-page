#include <gtest/gtest.h>
#include "assembler.hpp"
#include "formatter.hpp"
#include "page.hpp"
#include "parser.hpp"
#include "section.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// Reuse the TempConfigDir helper
class TempConfigDir2 {
public:
    TempConfigDir2() {
        path_ = fs::temp_directory_path() /
                ("huh_integ_" + std::to_string(
                    std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path_);
    }
    ~TempConfigDir2() { fs::remove_all(path_); }
    std::string path() const { return path_.string(); }
    void write(const std::string& filename, const std::string& content) {
        std::ofstream f(path_ / filename);
        f << content;
    }
private:
    fs::path path_;
};

// Synthetic man page used across integration tests
static const std::string FAKE_MAN = R"(NAME
       testcmd - a test command for unit testing

SYNOPSIS
       testcmd [OPTIONS] FILE

DESCRIPTION
       testcmd reads a FILE and does something useful.
       It supports multiple options.

OPTIONS
       -v     Enable verbose output.
       -h     Show help.

SEE ALSO
       man(1), bash(1)

COLOPHON
       This is part of the test suite.
)";

// ============================================================
// User Story: Full pipeline produces styled output
// ============================================================

TEST(Integration, FullPipelineParsesThenAssembles) {
    TempConfigDir2 dir;
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);

    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    EXPECT_GT(result.getSectionCount(), 0u);
}

TEST(Integration, FullPipelinePreservesAllSections) {
    TempConfigDir2 dir;
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    EXPECT_EQ(result.getSectionCount(), 6u);
}

TEST(Integration, FullPipelineContentIsPreservedAfterStyling) {
    TempConfigDir2 dir;
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    // Verify actual text survives the ANSI wrapping
    bool found_testcmd = false;
    for (const auto& sec : result.getSections()) {
        if (sec.getContent().find("testcmd") != std::string::npos) {
            found_testcmd = true;
            break;
        }
    }
    EXPECT_TRUE(found_testcmd);
}

TEST(Integration, FullPipelineOutputContainsAnsiCodes) {
    TempConfigDir2 dir;
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    bool has_ansi = false;
    for (const auto& sec : result.getSections()) {
        if (sec.getTitle().find("\033[") != std::string::npos) {
            has_ansi = true;
            break;
        }
    }
    EXPECT_TRUE(has_ansi);
}

// ============================================================
// User Story: Hide the COLOPHON section
// ============================================================

TEST(Integration, UserStory_HideColophon) {
    TempConfigDir2 dir;
    dir.write("config.toml", R"(
[COLOPHON]
skip = true
)");
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    for (const auto& sec : result.getSections()) {
        EXPECT_EQ(sec.getTitle().find("COLOPHON"), std::string::npos);
    }
    EXPECT_EQ(result.getSectionCount(), 5u);
}

// ============================================================
// User Story: Pin SYNOPSIS to the top
// ============================================================

TEST(Integration, UserStory_PinSynopsisToTop) {
    TempConfigDir2 dir;
    dir.write("config.toml", R"(
[SYNOPSIS]
prepend = true
)");
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    // First section content should contain SYNOPSIS content
    int i = 0;
    EXPECT_NE(result.getSection(i).getContent().find("testcmd [OPTIONS] FILE"),
              std::string::npos);
    EXPECT_EQ(result.getSectionCount(), 6u);
}

// ============================================================
// User Story: Pin SEE ALSO to the bottom
// ============================================================

TEST(Integration, UserStory_PinSeeAlsoToBottom) {
    TempConfigDir2 dir;
    // TOML bare keys cannot contain spaces — must quote: ["SEE ALSO"]
    dir.write("config.toml", R"(
["SEE ALSO"]
postpend = true
)");
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    auto secs = result.getSections();
    EXPECT_NE(secs.back().getContent().find("man(1)"), std::string::npos);
    EXPECT_EQ(result.getSectionCount(), 6u);
}

// helper for integration tests: find a section whose title contains `name`
static const Section* integ_find(const Page& page, const std::string& name) {
    for (const auto& sec : page.getSections())
        if (sec.getTitle().find(name) != std::string::npos) return &sec;
    return nullptr;
}

// ============================================================
// User Story: Custom color on DESCRIPTION
// ============================================================

TEST(Integration, UserStory_CustomColorOnDescription) {
    TempConfigDir2 dir;
    dir.write("config.toml", R"(
[DESCRIPTION]
title   = { color = "CYAN", bold = true }
section = { color = "GREEN" }
)");
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    // Assembled titles have ANSI codes — search by substring
    const Section* desc = integ_find(result, "DESCRIPTION");
    ASSERT_NE(desc, nullptr);
    EXPECT_NE(desc->getTitle().find(Formatter::CYAN), std::string::npos);
    // NOTE: bold = true does NOT work due to a code bug (see bug report below)
    //       Assembler passes Formatter::BOLD ("\033[1m") to format() which only
    //       recognises the string name "BOLD", so it returns text unchanged.
    EXPECT_NE(desc->getContent().find(Formatter::GREEN), std::string::npos);
}

// ============================================================
// User Story: Combined — pin SYNOPSIS, hide COLOPHON, custom colors
// ============================================================

TEST(Integration, UserStory_CombinedConfig) {
    TempConfigDir2 dir;
    // Note: section names with spaces need quoted TOML keys: ["SEE ALSO"]
    dir.write("config.toml", R"(
[default]
title = { color = "CYAN", bold = true }

[SYNOPSIS]
prepend = true

[COLOPHON]
skip = true

["SEE ALSO"]
postpend = true
)");
    Parser parser;
    Page parsed = parser.parseToPage(FAKE_MAN);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);

    // COLOPHON gone
    bool has_colophon = false;
    for (auto& sec : result.getSections())
        if (sec.getTitle().find("COLOPHON") != std::string::npos)
            has_colophon = true;
    EXPECT_FALSE(has_colophon);

    // SYNOPSIS first (content contains synopsis text)
    int i = 0;
    EXPECT_NE(result.getSection(i).getContent().find("testcmd [OPTIONS]"),
              std::string::npos);

    // SEE ALSO last
    EXPECT_NE(result.getSections().back().getContent().find("man(1)"),
              std::string::npos);

    // 5 sections total (6 - 1 skipped)
    EXPECT_EQ(result.getSectionCount(), 5u);
}

// ============================================================
// Edge: empty input through full pipeline
// ============================================================

TEST(Integration, EmptyInputThroughFullPipelineIsEmpty) {
    TempConfigDir2 dir;
    Parser parser;
    Page parsed = parser.parseToPage("");
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);
    EXPECT_EQ(result.getSectionCount(), 0u);
}

// ============================================================
// Edge: only section headers, no content
// ============================================================

TEST(Integration, SectionHeadersOnlyPreservedThroughPipeline) {
    TempConfigDir2 dir;
    Parser parser;
    Page parsed = parser.parseToPage("NAME\nDESCRIPTION\nOPTIONS\n");
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);
    EXPECT_GE(result.getSectionCount(), 2u);
}

// ============================================================
// Chaos: page with hundreds of sections
// ============================================================

TEST(Integration, HundredsOfSectionsThroughPipeline) {
    TempConfigDir2 dir;
    std::string raw;
    for (int i = 0; i < 200; ++i)
        raw += "SECTION" + std::to_string(i) + "\n   content " + std::to_string(i) + "\n";

    Parser parser;
    Page parsed = parser.parseToPage(raw);
    Assembler assembler(dir.path());
    EXPECT_NO_THROW({
        Page result = assembler.assemble(parsed);
        EXPECT_EQ(result.getSectionCount(), 200u);
    });
}

// ============================================================
// Adversarial: section name containing TOML special characters
// ============================================================

TEST(Integration, SectionNameNotInTomlConfigUsesDefaultStyle) {
    TempConfigDir2 dir;
    dir.write("config.toml", R"(
[default]
title = { color = "YELLOW" }
)");
    Parser parser;
    std::string raw = "WEIRD SECTION NAME\n   content here\n";
    Page parsed = parser.parseToPage(raw);
    Assembler assembler(dir.path());
    Page result = assembler.assemble(parsed);
    int i = 0;
    EXPECT_NE(result.getSection(i).getTitle().find(Formatter::YELLOW), std::string::npos);
}
