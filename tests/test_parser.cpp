#include <gtest/gtest.h>
#include "parser.hpp"
#include "page.hpp"

// Simulated man page text that mirrors real man output structure:
// - Section headers start at column 0 (no leading space)
// - Content lines start with at least one space
static const std::string SIMPLE_MAN = R"(NAME
       ls - list directory contents

DESCRIPTION
       List information about the FILEs (the current directory by default).

OPTIONS
       -a     do not ignore entries starting with .
       -l     use a long listing format
)";

static const std::string MULTI_SECTION_MAN = R"(NAME
       grep - print lines that match patterns

SYNOPSIS
       grep [OPTION...] PATTERNS [FILE...]

DESCRIPTION
       grep  searches  for  PATTERNS  in each FILE.

EXIT STATUS
       0      if at least one line is selected.
       1      if no lines were selected.
)";

// ============================================================
// Parser::split
// ============================================================

TEST(Parser, SplitSimpleString) {
    Parser p;
    auto tokens = p.split("a:b:c", ':');
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "a");
    EXPECT_EQ(tokens[1], "b");
    EXPECT_EQ(tokens[2], "c");
}

TEST(Parser, SplitByNewline) {
    Parser p;
    auto tokens = p.split("line1\nline2\nline3", '\n');
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "line1");
    EXPECT_EQ(tokens[1], "line2");
    EXPECT_EQ(tokens[2], "line3");
}

TEST(Parser, SplitDelimiterNotPresent) {
    Parser p;
    auto tokens = p.split("hello", '\n');
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "hello");
}

TEST(Parser, SplitEmptyStringProducesZeroTokens) {
    // getline on an empty istringstream hits EOF immediately and produces nothing
    Parser p;
    auto tokens = p.split("", '\n');
    EXPECT_EQ(tokens.size(), 0u);
}

TEST(Parser, SplitConsecutiveDelimitersProduceEmptyTokens) {
    Parser p;
    auto tokens = p.split("a::b", ':');
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "a");
    EXPECT_EQ(tokens[1], "");
    EXPECT_EQ(tokens[2], "b");
}

TEST(Parser, SplitDelimiterAtStart) {
    Parser p;
    auto tokens = p.split(":abc", ':');
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "");
    EXPECT_EQ(tokens[1], "abc");
}

TEST(Parser, SplitDelimiterAtEnd) {
    Parser p;
    // getline does not produce a trailing empty token for trailing delimiter
    auto tokens = p.split("abc:", ':');
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "abc");
}

TEST(Parser, SplitSingleCharacterTokens) {
    Parser p;
    auto tokens = p.split("a b c d", ' ');
    ASSERT_EQ(tokens.size(), 4u);
}

// ============================================================
// Parser::parseToPage — structure
// ============================================================

TEST(Parser, ParseToPageEmptyStringReturnsEmptyPage) {
    Parser p;
    Page page = p.parseToPage("");
    EXPECT_EQ(page.getSectionCount(), 0u);
}

TEST(Parser, ParseToPageOnlyEmptyLinesReturnsEmptyPage) {
    Parser p;
    Page page = p.parseToPage("\n\n\n");
    EXPECT_EQ(page.getSectionCount(), 0u);
}

TEST(Parser, ParseToPageExtractsSectionCount) {
    Parser p;
    Page page = p.parseToPage(SIMPLE_MAN);
    EXPECT_EQ(page.getSectionCount(), 3u);
}

TEST(Parser, ParseToPageExtractsSectionTitles) {
    Parser p;
    Page page = p.parseToPage(SIMPLE_MAN);
    EXPECT_TRUE(page.hasSection("NAME"));
    EXPECT_TRUE(page.hasSection("DESCRIPTION"));
    EXPECT_TRUE(page.hasSection("OPTIONS"));
}

TEST(Parser, ParseToPageExtractsSectionContent) {
    Parser p;
    Page page = p.parseToPage(SIMPLE_MAN);
    std::string desc_content = page.getSection("DESCRIPTION").getContent();
    EXPECT_NE(desc_content.find("List information"), std::string::npos);
}

TEST(Parser, ParseToPagePreservesMultipleSections) {
    Parser p;
    Page page = p.parseToPage(MULTI_SECTION_MAN);
    EXPECT_EQ(page.getSectionCount(), 4u);
    EXPECT_TRUE(page.hasSection("NAME"));
    EXPECT_TRUE(page.hasSection("SYNOPSIS"));
    EXPECT_TRUE(page.hasSection("DESCRIPTION"));
    EXPECT_TRUE(page.hasSection("EXIT STATUS"));
}

TEST(Parser, ParseToPagePreservesOriginalOrder) {
    Parser p;
    Page page = p.parseToPage(MULTI_SECTION_MAN);
    auto secs = page.getSections();
    EXPECT_EQ(secs[0].getTitle(), "NAME");
    EXPECT_EQ(secs[1].getTitle(), "SYNOPSIS");
    EXPECT_EQ(secs[2].getTitle(), "DESCRIPTION");
    EXPECT_EQ(secs[3].getTitle(), "EXIT STATUS");
}

TEST(Parser, ParseToPageSkipsContentLinesWithNoCurrentTitle) {
    // Content before any section header should be discarded
    Parser p;
    std::string raw = "   orphan content line\nNAME\n   actual content\n";
    Page page = p.parseToPage(raw);
    ASSERT_EQ(page.getSectionCount(), 1u);
    EXPECT_EQ(page.getSection("NAME").getTitle(), "NAME");
}

TEST(Parser, ParseToPageSkipsEmptyLines) {
    Parser p;
    std::string raw = "NAME\n\n\n   content\n\n";
    Page page = p.parseToPage(raw);
    ASSERT_EQ(page.getSectionCount(), 1u);
    std::string content = page.getSection("NAME").getContent();
    // Empty lines are not added to content
    EXPECT_EQ(content.find("\n\n"), std::string::npos);
}

TEST(Parser, ParseToPageHandlesSectionWithNoContent) {
    // A section header followed immediately by another header
    Parser p;
    std::string raw = "NAME\nSYNOPSIS\n   some synopsis\n";
    Page page = p.parseToPage(raw);
    // NAME gets added only when it has content? Let's check:
    // When SYNOPSIS is encountered, NAME's content (empty) is set and NAME is added
    // Actually: currentSection.setContent("") and page.addSection(currentSection)
    // NAME will be added with empty content
    EXPECT_TRUE(page.hasSection("NAME"));
    EXPECT_EQ(page.getSection("NAME").getContent(), "");
}

// ============================================================
// BUG EXPOSURE: leading newline in content
// ============================================================

TEST(Parser, BugLeadingNewlineInContent) {
    // BUG: content always starts with '\n' because the parser does
    // currentContent += '\n' + line even for the first content line.
    // This means every section's content begins with '\n'.
    Parser p;
    std::string raw = "NAME\n   ls - list directory contents\n";
    Page page = p.parseToPage(raw);
    std::string content = page.getSection("NAME").getContent();
    // BUG: content[0] == '\n'
    EXPECT_EQ(content[0], '\n');
}

// ============================================================
// Parser::getPage — integration with real shell
// ============================================================

TEST(Parser, GetPageWithEchoReturnsOutput) {
    Parser p;
    std::string result = p.getPage("echo 'hello world'");
    EXPECT_NE(result.find("hello world"), std::string::npos);
}

TEST(Parser, GetPageWithTrueCommandReturnsEmpty) {
    Parser p;
    std::string result = p.getPage("true");
    EXPECT_EQ(result, "");
}

TEST(Parser, GetPageWithMultiLineEcho) {
    Parser p;
    std::string result = p.getPage("printf 'line1\\nline2\\n'");
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
}

// ============================================================
// Chaos / adversarial
// ============================================================

TEST(Parser, ParseToPageWithOnlySectionHeaders) {
    Parser p;
    std::string raw = "NAME\nDESCRIPTION\nOPTIONS\n";
    Page page = p.parseToPage(raw);
    // Each header seen triggers add of the previous (empty-content) section
    EXPECT_GE(page.getSectionCount(), 2u);
}

TEST(Parser, ParseToPageWithVeryLongContent) {
    Parser p;
    std::string raw = "NAME\n";
    for (int i = 0; i < 10000; ++i)
        raw += "   content line " + std::to_string(i) + "\n";
    Page page = p.parseToPage(raw);
    ASSERT_EQ(page.getSectionCount(), 1u);
    EXPECT_NE(page.getSection("NAME").getContent().find("content line 9999"), std::string::npos);
}

TEST(Parser, ParseToPageWithAnsiCodesInContent) {
    // Man pages sometimes contain backspace-encoded bold (e.g. x^Hx for bold x)
    // We just verify the parser doesn't crash and stores them
    Parser p;
    std::string raw = "NAME\n   \033[1mbold text\033[0m\n";
    EXPECT_NO_THROW({
        Page page = p.parseToPage(raw);
        EXPECT_EQ(page.getSectionCount(), 1u);
    });
}

TEST(Parser, ParseToPageTitleWithSpecialChars) {
    Parser p;
    std::string raw = "SEE ALSO\n   related(1)\n";
    Page page = p.parseToPage(raw);
    EXPECT_TRUE(page.hasSection("SEE ALSO"));
}

TEST(Parser, SplitVeryLargeString) {
    Parser p;
    std::string big(1000000, 'a');
    auto tokens = p.split(big, '\n');
    EXPECT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].size(), 1000000u);
}
