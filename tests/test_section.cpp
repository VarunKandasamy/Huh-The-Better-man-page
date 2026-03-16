#include <gtest/gtest.h>
#include "section.hpp"

// ============================================================
// Construction
// ============================================================

TEST(Section, DefaultConstructorEmptyTitleAndContent) {
    Section s;
    EXPECT_EQ(s.getTitle(), "");
    EXPECT_EQ(s.getContent(), "");
}

TEST(Section, ParamConstructorSetsBoth) {
    Section s("NAME", "   ls - list directory contents");
    EXPECT_EQ(s.getTitle(), "NAME");
    EXPECT_EQ(s.getContent(), "   ls - list directory contents");
}

TEST(Section, ParamConstructorEmptyTitleNonEmptyContent) {
    Section s("", "orphan content");
    EXPECT_EQ(s.getTitle(), "");
    EXPECT_EQ(s.getContent(), "orphan content");
}

TEST(Section, ParamConstructorNonEmptyTitleEmptyContent) {
    Section s("OPTIONS", "");
    EXPECT_EQ(s.getTitle(), "OPTIONS");
    EXPECT_EQ(s.getContent(), "");
}

// ============================================================
// Setters / Getters
// ============================================================

TEST(Section, SetTitleGetTitleRoundtrip) {
    Section s;
    s.setTitle("DESCRIPTION");
    EXPECT_EQ(s.getTitle(), "DESCRIPTION");
}

TEST(Section, SetContentGetContentRoundtrip) {
    Section s;
    s.setContent("   line one\n   line two");
    EXPECT_EQ(s.getContent(), "   line one\n   line two");
}

TEST(Section, SetTitleOverwritesPrevious) {
    Section s("ORIGINAL", "c");
    s.setTitle("REPLACED");
    EXPECT_EQ(s.getTitle(), "REPLACED");
}

TEST(Section, SetContentOverwritesPrevious) {
    Section s("T", "old");
    s.setContent("new");
    EXPECT_EQ(s.getContent(), "new");
}

TEST(Section, SetTitleEmptyStringClearsTitle) {
    Section s("NAME", "c");
    s.setTitle("");
    EXPECT_EQ(s.getTitle(), "");
}

TEST(Section, SetContentEmptyStringClearsContent) {
    Section s("T", "data");
    s.setContent("");
    EXPECT_EQ(s.getContent(), "");
}

// ============================================================
// Copy semantics
// ============================================================

TEST(Section, CopyConstructorDeepCopy) {
    Section s1("NAME", "content");
    Section s2(s1);
    s2.setTitle("OTHER");
    s2.setContent("other content");
    EXPECT_EQ(s1.getTitle(), "NAME");
    EXPECT_EQ(s1.getContent(), "content");
}

TEST(Section, AssignmentOperatorCopiesFields) {
    Section s1("NAME", "content");
    Section s2;
    s2 = s1;
    EXPECT_EQ(s2.getTitle(), "NAME");
    EXPECT_EQ(s2.getContent(), "content");
}

TEST(Section, AssignmentOperatorDeepCopy) {
    Section s1("NAME", "content");
    Section s2;
    s2 = s1;
    s2.setTitle("CHANGED");
    EXPECT_EQ(s1.getTitle(), "NAME");
}

TEST(Section, SelfAssignmentSafe) {
    Section s("NAME", "content");
    s = s;
    EXPECT_EQ(s.getTitle(), "NAME");
    EXPECT_EQ(s.getContent(), "content");
}

// ============================================================
// Edge cases
// ============================================================

TEST(Section, TitleWithSectionNumber) {
    Section s;
    s.setTitle("NAME(1)");
    EXPECT_EQ(s.getTitle(), "NAME(1)");
}

TEST(Section, TitleWithSpaces) {
    Section s;
    s.setTitle("SEE ALSO");
    EXPECT_EQ(s.getTitle(), "SEE ALSO");
}

TEST(Section, ContentPreservesEmbeddedNewlines) {
    Section s;
    s.setContent("line1\nline2\nline3");
    EXPECT_EQ(s.getContent(), "line1\nline2\nline3");
}

TEST(Section, ContentPreservesAnsiEscapeCodes) {
    Section s;
    std::string ansi = "\033[31mRED TEXT\033[0m";
    s.setContent(ansi);
    EXPECT_EQ(s.getContent(), ansi);
}

TEST(Section, ContentPreservesTabCharacters) {
    Section s;
    s.setContent("\t-v\tverbose mode");
    EXPECT_EQ(s.getContent(), "\t-v\tverbose mode");
}

TEST(Section, VeryLongTitle) {
    Section s;
    std::string big(10000, 'X');
    s.setTitle(big);
    EXPECT_EQ(s.getTitle(), big);
}

TEST(Section, VeryLongContent) {
    Section s;
    std::string big(100000, 'A');
    s.setContent(big);
    EXPECT_EQ(s.getContent(), big);
}

// ============================================================
// Adversarial
// ============================================================

TEST(Section, TitleWithShellMetacharactersStoredVerbatim) {
    Section s;
    s.setTitle("; rm -rf /");
    EXPECT_EQ(s.getTitle(), "; rm -rf /");
}

TEST(Section, ContentWithBackslashSequences) {
    Section s;
    s.setContent("\\n\\t\\r literal backslashes");
    EXPECT_EQ(s.getContent(), "\\n\\t\\r literal backslashes");
}
