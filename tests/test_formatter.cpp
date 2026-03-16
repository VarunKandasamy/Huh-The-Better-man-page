#include <gtest/gtest.h>
#include "formatter.hpp"

// ============================================================
// ANSI escape code constants
// ============================================================

TEST(Formatter, ResetConstant) {
    EXPECT_EQ(Formatter::RESET, "\033[0m");
}

TEST(Formatter, BoldConstant) {
    EXPECT_EQ(Formatter::BOLD, "\033[1m");
}

TEST(Formatter, FaintConstant) {
    EXPECT_EQ(Formatter::FAINT, "\033[2m");
}

TEST(Formatter, ItalicConstant) {
    EXPECT_EQ(Formatter::ITALIC, "\033[3m");
}

TEST(Formatter, UnderlineConstant) {
    EXPECT_EQ(Formatter::UNDERLINE, "\033[4m");
}

TEST(Formatter, RedConstant) {
    EXPECT_EQ(Formatter::RED, "\033[31m");
}

TEST(Formatter, GreenConstant) {
    EXPECT_EQ(Formatter::GREEN, "\033[32m");
}

TEST(Formatter, YellowConstant) {
    EXPECT_EQ(Formatter::YELLOW, "\033[33m");
}

TEST(Formatter, BlueConstant) {
    EXPECT_EQ(Formatter::BLUE, "\033[34m");
}

TEST(Formatter, MagentaConstant) {
    EXPECT_EQ(Formatter::MAGENTA, "\033[35m");
}

TEST(Formatter, CyanConstant) {
    EXPECT_EQ(Formatter::CYAN, "\033[36m");
}

TEST(Formatter, WhiteConstant) {
    EXPECT_EQ(Formatter::WHITE, "\033[37m");
}

// ============================================================
// format() — color names
// ============================================================

TEST(Formatter, FormatWithRedPrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "RED"), "\033[31mhello");
}

TEST(Formatter, FormatWithGreenPrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "GREEN"), "\033[32mhello");
}

TEST(Formatter, FormatWithYellowPrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "YELLOW"), "\033[33mhello");
}

TEST(Formatter, FormatWithBluePrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "BLUE"), "\033[34mhello");
}

TEST(Formatter, FormatWithMagentaPrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "MAGENTA"), "\033[35mhello");
}

TEST(Formatter, FormatWithCyanPrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "CYAN"), "\033[36mhello");
}

TEST(Formatter, FormatWithWhitePrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "WHITE"), "\033[37mhello");
}

// ============================================================
// format() — effect names
// ============================================================

TEST(Formatter, FormatWithBoldPrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("text", "BOLD"), "\033[1mtext");
}

TEST(Formatter, FormatWithItalicPrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("text", "ITALIC"), "\033[3mtext");
}

TEST(Formatter, FormatWithUnderlinePrependsCode) {
    Formatter f;
    EXPECT_EQ(f.format("text", "UNDERLINE"), "\033[4mtext");
}

// ============================================================
// format() — unknown / edge cases
// ============================================================

TEST(Formatter, FormatWithUnknownStyleReturnsTextUnchanged) {
    Formatter f;
    EXPECT_EQ(f.format("hello", "PURPLE"), "hello");
}

TEST(Formatter, FormatWithEmptyStyleReturnsTextUnchanged) {
    Formatter f;
    EXPECT_EQ(f.format("hello", ""), "hello");
}

TEST(Formatter, FormatWithLowercaseColorNameReturnsTextUnchanged) {
    // "red" is not "RED" — case-sensitive check
    Formatter f;
    EXPECT_EQ(f.format("hello", "red"), "hello");
}

TEST(Formatter, FormatWithEmptyText) {
    Formatter f;
    EXPECT_EQ(f.format("", "RED"), "\033[31m");
}

TEST(Formatter, FormatWithEmptyTextUnknownStyleReturnsEmpty) {
    Formatter f;
    EXPECT_EQ(f.format("", "UNKNOWN"), "");
}

TEST(Formatter, FormatPreservesMultilineText) {
    Formatter f;
    std::string multiline = "line1\nline2\nline3";
    std::string result = f.format(multiline, "CYAN");
    EXPECT_EQ(result, "\033[36m" + multiline);
}

TEST(Formatter, FormatStacksCorrectlyWhenCalledTwice) {
    // Calling format twice adds two escape codes at the front
    Formatter f;
    std::string step1 = f.format("text", "BOLD");         // \033[1mtext
    std::string step2 = f.format(step1, "RED");            // \033[31m\033[1mtext
    EXPECT_EQ(step2, "\033[31m\033[1mtext");
}

TEST(Formatter, FormatWithTextAlreadyContainingAnsiCodes) {
    Formatter f;
    std::string already_styled = "\033[32mGREEN\033[0m";
    std::string result = f.format(already_styled, "BOLD");
    EXPECT_EQ(result, "\033[1m\033[32mGREEN\033[0m");
}

TEST(Formatter, FormatDoesNotAppendReset) {
    // format() only prepends; RESET is the caller's responsibility
    Formatter f;
    std::string result = f.format("text", "RED");
    EXPECT_EQ(result.find(Formatter::RESET), std::string::npos);
}

TEST(Formatter, FormatWithVeryLongText) {
    Formatter f;
    std::string big(100000, 'Z');
    std::string result = f.format(big, "CYAN");
    EXPECT_EQ(result.size(), big.size() + Formatter::CYAN.size());
    EXPECT_EQ(result.substr(0, Formatter::CYAN.size()), Formatter::CYAN);
}
