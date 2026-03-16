#include <gtest/gtest.h>
#include "assembler.hpp"
#include "formatter.hpp"
#include "page.hpp"
#include "section.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ============================================================
// RAII temp directory for TOML configs
// ============================================================
class TempConfigDir {
public:
    TempConfigDir() {
        path_ = fs::temp_directory_path() /
                ("huh_test_" + std::to_string(
                    std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path_);
    }
    ~TempConfigDir() {
        fs::remove_all(path_);
    }
    std::string path() const { return path_.string(); }
    void write(const std::string& filename, const std::string& content) {
        std::ofstream f(path_ / filename);
        f << content;
    }
private:
    fs::path path_;
};

static Page make_page(std::vector<std::pair<std::string,std::string>> secs) {
    Page p;
    for (auto& [t, c] : secs)
        p.addSection(Section(t, c));
    return p;
}

// After assemble() all titles are ANSI-wrapped, so bare-string hasSection() won't
// find them.  Use these helpers to search assembled output by substring.
static bool assembled_title_contains(const Page& page, const std::string& name) {
    for (const auto& sec : page.getSections())
        if (sec.getTitle().find(name) != std::string::npos) return true;
    return false;
}

static const Section* assembled_find_section(const Page& page, const std::string& name) {
    for (const auto& sec : page.getSections())
        if (sec.getTitle().find(name) != std::string::npos) return &sec;
    return nullptr;
}

// ============================================================
// No-config behaviour (empty config dir)
// ============================================================

TEST(Assembler, EmptyPageReturnsEmptyPage) {
    TempConfigDir dir;
    Assembler a(dir.path());
    Page input;
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 0u);
}

TEST(Assembler, NoConfigPreservesAllSections) {
    TempConfigDir dir;
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"DESCRIPTION","d"},{"OPTIONS","o"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 3u);
}

TEST(Assembler, NoConfigPreservesOriginalOrder) {
    TempConfigDir dir;
    Assembler a(dir.path());
    // Use unique, non-overlapping content strings to distinguish sections
    Page input = make_page({{"NAME","content_name"},{"SYNOPSIS","content_synopsis"},{"DESCRIPTION","content_description"}});
    Page result = a.assemble(input);
    auto secs = result.getSections();
    ASSERT_EQ(secs.size(), 3u);
    // After styling, content has ANSI codes wrapped around it but the original text survives
    EXPECT_NE(secs[0].getContent().find("content_name"), std::string::npos);
    EXPECT_NE(secs[1].getContent().find("content_synopsis"), std::string::npos);
    EXPECT_NE(secs[2].getContent().find("content_description"), std::string::npos);
}

TEST(Assembler, NoConfigAppliesDefaultWhiteStyleToTitle) {
    TempConfigDir dir;
    Assembler a(dir.path());
    Page input = make_page({{"NAME","content"}});
    Page result = a.assemble(input);
    // Default color is WHITE = \033[37m
    int i = 0;
    std::string title = result.getSection(i).getTitle();
    EXPECT_NE(title.find(Formatter::WHITE), std::string::npos);
}

TEST(Assembler, NoConfigAppliesDefaultWhiteStyleToContent) {
    TempConfigDir dir;
    Assembler a(dir.path());
    Page input = make_page({{"NAME","content"}});
    Page result = a.assemble(input);
    int i = 0;
    std::string content = result.getSection(i).getContent();
    EXPECT_NE(content.find(Formatter::WHITE), std::string::npos);
}

TEST(Assembler, NoConfigAppendsResetToTitle) {
    TempConfigDir dir;
    Assembler a(dir.path());
    Page input = make_page({{"NAME","content"}});
    Page result = a.assemble(input);
    int i = 0;
    std::string title = result.getSection(i).getTitle();
    EXPECT_NE(title.find(Formatter::RESET), std::string::npos);
}

TEST(Assembler, NoConfigAppendsResetToContent) {
    TempConfigDir dir;
    Assembler a(dir.path());
    Page input = make_page({{"NAME","content"}});
    Page result = a.assemble(input);
    int i = 0;
    std::string content = result.getSection(i).getContent();
    EXPECT_NE(content.find(Formatter::RESET), std::string::npos);
}

// ============================================================
// skip
// ============================================================

TEST(Assembler, SkipTrueRemovesSectionFromOutput) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[COLOPHON]
skip = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"COLOPHON","boring"},{"OPTIONS","o"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 2u);
    EXPECT_FALSE(result.hasSection("COLOPHON"));
}

TEST(Assembler, SkipFalseKeepsSectionInOutput) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[OPTIONS]
skip = false
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"OPTIONS","o"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 2u);
    // After assemble(), titles have ANSI codes — search by substring
    EXPECT_TRUE(assembled_title_contains(result, "OPTIONS"));
}

TEST(Assembler, SkipNonexistentSectionIsHarmless) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[GHOST]
skip = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 1u);
}

TEST(Assembler, SkipAllSectionsReturnsEmptyPage) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[NAME]
skip = true
[DESCRIPTION]
skip = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"DESCRIPTION","d"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 0u);
}

// ============================================================
// prepend
// ============================================================

TEST(Assembler, PrependSectionAppearsFirst) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[SYNOPSIS]
prepend = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"SYNOPSIS","s"},{"DESCRIPTION","d"}});
    Page result = a.assemble(input);
    // SYNOPSIS should be first (content contains "s", easier to check than styled title)
    int i = 0;
    EXPECT_NE(result.getSection(i).getContent().find("s"), std::string::npos);
    EXPECT_EQ(result.getSectionCount(), 3u);
}

TEST(Assembler, PrependSectionNotDuplicatedInMainBody) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[SYNOPSIS]
prepend = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"SYNOPSIS","s"},{"DESCRIPTION","d"}});
    Page result = a.assemble(input);
    // SYNOPSIS should appear exactly once
    int count = 0;
    for (const auto& sec : result.getSections())
        if (sec.getContent().find("s") != std::string::npos && sec.getContent().size() == 1 + /* RESET */ Formatter::RESET.size() + Formatter::WHITE.size())
            count++;
    EXPECT_EQ(result.getSectionCount(), 3u);
}

TEST(Assembler, PrependNonexistentSectionIsHarmless) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[GHOST]
prepend = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 1u);
}

// ============================================================
// postpend
// ============================================================

TEST(Assembler, PostpendSectionAppearsLast) {
    TempConfigDir dir;
    // TOML bare keys cannot contain spaces — must quote: ["SEE ALSO"]
    dir.write("config.toml", R"(
["SEE ALSO"]
postpend = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"SEE ALSO","sa"},{"DESCRIPTION","d"}});
    Page result = a.assemble(input);
    auto secs = result.getSections();
    EXPECT_EQ(result.getSectionCount(), 3u);
    EXPECT_NE(secs.back().getContent().find("sa"), std::string::npos);
}

TEST(Assembler, PostpendSectionNotDuplicatedInMainBody) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
["SEE ALSO"]
postpend = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"SEE ALSO","sa"},{"DESCRIPTION","d"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 3u);
}

// ============================================================
// Styling — color
// ============================================================

TEST(Assembler, CustomColorAppliedToTitle) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[NAME]
title = { color = "CYAN" }
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","content"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_NE(result.getSection(i).getTitle().find(Formatter::CYAN), std::string::npos);
}

TEST(Assembler, CustomColorAppliedToContent) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[DESCRIPTION]
section = { color = "GREEN" }
)");
    Assembler a(dir.path());
    Page input = make_page({{"DESCRIPTION","body text"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_NE(result.getSection(i).getContent().find(Formatter::GREEN), std::string::npos);
}

// ============================================================
// Styling — effects
// ============================================================

TEST(Assembler, BoldTrueAppliesBoldToTitle) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[NAME]
title = { bold = true }
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_NE(result.getSection(i).getTitle().find(Formatter::BOLD), std::string::npos);
}

TEST(Assembler, BoldFalseDoesNotApplyBoldToTitle) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[NAME]
title = { bold = false }
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_EQ(result.getSection(i).getTitle().find(Formatter::BOLD), std::string::npos);
}

TEST(Assembler, ItalicTrueAppliesItalicToTitle) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[NAME]
title = { italic = true }
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_NE(result.getSection(i).getTitle().find(Formatter::ITALIC), std::string::npos);
}

TEST(Assembler, UnderlineTrueAppliesUnderlineToTitle) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[NAME]
title = { underline = true }
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_NE(result.getSection(i).getTitle().find(Formatter::UNDERLINE), std::string::npos);
}

TEST(Assembler, BoldTrueAppliesBoldToContent) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[DESCRIPTION]
section = { bold = true }
)");
    Assembler a(dir.path());
    Page input = make_page({{"DESCRIPTION","some text"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_NE(result.getSection(i).getContent().find(Formatter::BOLD), std::string::npos);
}

// ============================================================
// [default] block inheritance
// ============================================================

TEST(Assembler, DefaultBlockColorAppliedToUnspecifiedSection) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[default]
title = { color = "RED" }
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"}});
    Page result = a.assemble(input);
    int i = 0;
    EXPECT_NE(result.getSection(i).getTitle().find(Formatter::RED), std::string::npos);
}

TEST(Assembler, SectionSpecificColorOverridesDefault) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[default]
title = { color = "RED" }
[NAME]
title = { color = "BLUE" }
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"OPTIONS","o"}});
    Page result = a.assemble(input);
    // Assembled titles are ANSI-wrapped — find by substring helper
    const Section* name_sec = assembled_find_section(result, "NAME");
    const Section* opts_sec = assembled_find_section(result, "OPTIONS");
    ASSERT_NE(name_sec, nullptr);
    ASSERT_NE(opts_sec, nullptr);
    // NAME should have BLUE, not RED
    EXPECT_NE(name_sec->getTitle().find(Formatter::BLUE), std::string::npos);
    EXPECT_EQ(name_sec->getTitle().find(Formatter::RED), std::string::npos);
    // OPTIONS should fall back to default RED
    EXPECT_NE(opts_sec->getTitle().find(Formatter::RED), std::string::npos);
}

// ============================================================
// Skip takes priority over prepend/postpend
// ============================================================

TEST(Assembler, SkipTakesPriorityOverPrepend) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
[SYNOPSIS]
prepend = true
skip = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"SYNOPSIS","s"}});
    Page result = a.assemble(input);
    EXPECT_FALSE(result.hasSection("SYNOPSIS"));
    EXPECT_EQ(result.getSectionCount(), 1u);
}

TEST(Assembler, SkipTakesPriorityOverPostpend) {
    TempConfigDir dir;
    dir.write("config.toml", R"(
["SEE ALSO"]
postpend = true
skip = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"SEE ALSO","sa"}});
    Page result = a.assemble(input);
    EXPECT_FALSE(assembled_title_contains(result, "SEE ALSO"));
    EXPECT_EQ(result.getSectionCount(), 1u);
}

// ============================================================
// Error handling
// ============================================================

TEST(Assembler, MissingConfigDirHandledGracefully) {
    // Should not throw — loadConfig catches filesystem errors
    EXPECT_NO_THROW({
        Assembler a("/nonexistent/path/that/does/not/exist");
        Page input = make_page({{"NAME","c"}});
        a.assemble(input);
    });
}

TEST(Assembler, MalformedTomlFileHandledGracefully) {
    TempConfigDir dir;
    dir.write("bad.toml", "this is [not valid toml {{{{");
    EXPECT_NO_THROW({
        Assembler a(dir.path());
        Page input = make_page({{"NAME","c"}});
        a.assemble(input);
    });
}

TEST(Assembler, EmptyTomlFileHandledGracefully) {
    TempConfigDir dir;
    dir.write("empty.toml", "");
    EXPECT_NO_THROW({
        Assembler a(dir.path());
        Page input = make_page({{"NAME","c"}});
        Page result = a.assemble(input);
        EXPECT_EQ(result.getSectionCount(), 1u);
    });
}

TEST(Assembler, MultipleTomlFilesAllLoaded) {
    TempConfigDir dir;
    dir.write("a.toml", R"(
[NAME]
title = { color = "CYAN" }
)");
    dir.write("b.toml", R"(
[SYNOPSIS]
skip = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"NAME","c"},{"SYNOPSIS","s"},{"DESCRIPTION","d"}});
    Page result = a.assemble(input);
    EXPECT_EQ(result.getSectionCount(), 2u);
    EXPECT_FALSE(result.hasSection("SYNOPSIS"));
    int i = 0;
    EXPECT_NE(result.getSection(i).getTitle().find(Formatter::CYAN), std::string::npos);
}

// ============================================================
// BUG EXPOSURE: prepend/postpend ordering is alphabetical (std::set)
// ============================================================

TEST(Assembler, BugMultiplePrependSectionsAreAlphabetical) {
    // BUG: prependSections is a std::set<std::string>, so iteration is
    // alphabetical, NOT original document order.
    // ZZSYNOPSIS came first in the page but AANAME comes first alphabetically.
    TempConfigDir dir;
    dir.write("config.toml", R"(
[ZZSYNOPSIS]
prepend = true
[AANAME]
prepend = true
)");
    Assembler a(dir.path());
    Page input = make_page({{"ZZSYNOPSIS","zzz_content"},{"AANAME","aaa_content"},{"DESCRIPTION","d"}});
    Page result = a.assemble(input);
    auto secs = result.getSections();
    // Due to std::set, AANAME (alphabetically first) appears before ZZSYNOPSIS
    EXPECT_NE(secs[0].getContent().find("aaa_content"), std::string::npos);
    EXPECT_NE(secs[1].getContent().find("zzz_content"), std::string::npos);
}
