#include <gtest/gtest.h>
#include "page.hpp"
#include "section.hpp"

// Helper: build a page with N named sections
static Page make_page(std::vector<std::pair<std::string,std::string>> secs) {
    Page p;
    for (auto& [t, c] : secs)
        p.addSection(Section(t, c));
    return p;
}

// ============================================================
// Construction / basics
// ============================================================

TEST(Page, DefaultConstructorEmpty) {
    Page p;
    EXPECT_EQ(p.getSectionCount(), 0u);
}

TEST(Page, AddSectionIncreasesCount) {
    Page p;
    p.addSection(Section("NAME", "c"));
    EXPECT_EQ(p.getSectionCount(), 1u);
    p.addSection(Section("DESCRIPTION", "c"));
    EXPECT_EQ(p.getSectionCount(), 2u);
}

TEST(Page, AddSectionPreservesOrder) {
    Page p = make_page({{"A","1"},{"B","2"},{"C","3"}});
    EXPECT_EQ(p.getSections()[0].getTitle(), "A");
    EXPECT_EQ(p.getSections()[1].getTitle(), "B");
    EXPECT_EQ(p.getSections()[2].getTitle(), "C");
}

TEST(Page, GetSectionsReturnsCopy) {
    Page p = make_page({{"A","1"}});
    auto secs = p.getSections();
    secs[0].setTitle("MUTATED");
    // original page unchanged
    int idx = 0;
    EXPECT_EQ(p.getSection(idx).getTitle(), "A");
}

// ============================================================
// getSection by index
// ============================================================

TEST(Page, GetSectionByIndexReturnsCorrectSection) {
    Page p = make_page({{"NAME","c1"},{"DESCRIPTION","c2"}});
    int i = 1;
    EXPECT_EQ(p.getSection(i).getTitle(), "DESCRIPTION");
}

TEST(Page, GetSectionByIndexNegativeThrows) {
    Page p = make_page({{"A","c"}});
    int i = -1;
    EXPECT_THROW(p.getSection(i), std::out_of_range);
}

TEST(Page, GetSectionByIndexAtBoundaryThrows) {
    Page p = make_page({{"A","c"}});
    int i = 1; // size is 1, valid range is [0,0]
    EXPECT_THROW(p.getSection(i), std::out_of_range);
}

TEST(Page, GetSectionByIndexOnEmptyPageThrows) {
    Page p;
    int i = 0;
    EXPECT_THROW(p.getSection(i), std::out_of_range);
}

TEST(Page, GetSectionByIndexFirstElement) {
    Page p = make_page({{"FIRST","c"},{"SECOND","c"}});
    int i = 0;
    EXPECT_EQ(p.getSection(i).getTitle(), "FIRST");
}

TEST(Page, GetSectionByIndexLastElement) {
    Page p = make_page({{"A","c"},{"B","c"},{"C","c"}});
    int i = 2;
    EXPECT_EQ(p.getSection(i).getTitle(), "C");
}

TEST(Page, ConstGetSectionByIndexWorks) {
    const Page p = make_page({{"NAME","c"}});
    int i = 0;
    EXPECT_EQ(p.getSection(i).getTitle(), "NAME");
}

// ============================================================
// getSection by title
// ============================================================

TEST(Page, GetSectionByTitleFound) {
    Page p = make_page({{"NAME","c"},{"OPTIONS","opts"}});
    EXPECT_EQ(p.getSection("OPTIONS").getContent(), "opts");
}

TEST(Page, GetSectionByTitleNotFoundThrows) {
    Page p = make_page({{"NAME","c"}});
    EXPECT_THROW(p.getSection("NONEXISTENT"), std::invalid_argument);
}

TEST(Page, GetSectionByTitleEmptyPageThrows) {
    Page p;
    EXPECT_THROW(p.getSection("ANY"), std::invalid_argument);
}

TEST(Page, GetSectionByTitleCaseSensitive) {
    Page p = make_page({{"NAME","c"}});
    // "name" != "NAME"
    EXPECT_THROW(p.getSection("name"), std::invalid_argument);
}

TEST(Page, ConstGetSectionByTitleWorks) {
    const Page p = make_page({{"DESCRIPTION","body"}});
    EXPECT_EQ(p.getSection("DESCRIPTION").getContent(), "body");
}

// ============================================================
// hasSection
// ============================================================

TEST(Page, HasSectionReturnsTrueWhenPresent) {
    Page p = make_page({{"NAME","c"}});
    EXPECT_TRUE(p.hasSection("NAME"));
}

TEST(Page, HasSectionReturnsFalseWhenAbsent) {
    Page p = make_page({{"NAME","c"}});
    EXPECT_FALSE(p.hasSection("DESCRIPTION"));
}

TEST(Page, HasSectionEmptyPageReturnsFalse) {
    Page p;
    EXPECT_FALSE(p.hasSection("ANYTHING"));
}

TEST(Page, HasSectionCaseSensitive) {
    Page p = make_page({{"NAME","c"}});
    EXPECT_FALSE(p.hasSection("name"));
}

// ============================================================
// removeSection by index — test side effects only
// NOTE: The return value is a DANGLING REFERENCE (UB).
//       Tests here only verify the side-effect (section removed).
// ============================================================

TEST(Page, RemoveSectionByIndexDecreasesSectionCount) {
    Page p = make_page({{"A","c"},{"B","c"},{"C","c"}});
    int i = 1;
    p.removeSection(i);
    EXPECT_EQ(p.getSectionCount(), 2u);
}

TEST(Page, RemoveSectionByIndexRemovesCorrectSection) {
    Page p = make_page({{"A","c"},{"B","c"},{"C","c"}});
    int i = 1; // remove "B"
    p.removeSection(i);
    EXPECT_FALSE(p.hasSection("B"));
    EXPECT_TRUE(p.hasSection("A"));
    EXPECT_TRUE(p.hasSection("C"));
}

TEST(Page, RemoveSectionByIndexNegativeThrows) {
    Page p = make_page({{"A","c"}});
    int i = -1;
    EXPECT_THROW(p.removeSection(i), std::out_of_range);
}

TEST(Page, RemoveSectionByIndexOutOfBoundsThrows) {
    Page p = make_page({{"A","c"}});
    int i = 5;
    EXPECT_THROW(p.removeSection(i), std::out_of_range);
}

TEST(Page, RemoveSectionByIndexOnEmptyPageThrows) {
    Page p;
    int i = 0;
    EXPECT_THROW(p.removeSection(i), std::out_of_range);
}

// ============================================================
// removeSection by title — test side effects only
// NOTE: Same dangling reference UB as above.
// ============================================================

TEST(Page, RemoveSectionByTitleDecreasesSectionCount) {
    Page p = make_page({{"NAME","c"},{"OPTIONS","c"}});
    p.removeSection("NAME");
    EXPECT_EQ(p.getSectionCount(), 1u);
}

TEST(Page, RemoveSectionByTitleActuallyRemoves) {
    Page p = make_page({{"NAME","c"},{"OPTIONS","c"}});
    p.removeSection("NAME");
    EXPECT_FALSE(p.hasSection("NAME"));
    EXPECT_TRUE(p.hasSection("OPTIONS"));
}

TEST(Page, RemoveSectionByTitleNotFoundThrows) {
    Page p = make_page({{"NAME","c"}});
    EXPECT_THROW(p.removeSection("NONEXISTENT"), std::invalid_argument);
}

TEST(Page, RemoveSectionByTitleEmptyPageThrows) {
    Page p;
    EXPECT_THROW(p.removeSection("ANYTHING"), std::invalid_argument);
}

// ============================================================
// Copy semantics
// ============================================================

TEST(Page, CopyConstructorIsDeepCopy) {
    Page p1 = make_page({{"A","c"}});
    Page p2(p1);
    p2.addSection(Section("B","c"));
    EXPECT_EQ(p1.getSectionCount(), 1u);
    EXPECT_EQ(p2.getSectionCount(), 2u);
}

TEST(Page, AssignmentOperatorIsDeepCopy) {
    Page p1 = make_page({{"A","c"}});
    Page p2;
    p2 = p1;
    p2.addSection(Section("B","c"));
    EXPECT_EQ(p1.getSectionCount(), 1u);
}

// ============================================================
// Chaos / edge cases
// ============================================================

TEST(Page, AddManySectionsDoesNotCrash) {
    Page p;
    for (int i = 0; i < 10000; ++i)
        p.addSection(Section("SEC" + std::to_string(i), "body"));
    EXPECT_EQ(p.getSectionCount(), 10000u);
}

TEST(Page, DuplicateSectionTitlesAllowed) {
    // The implementation doesn't prevent duplicates; getSection returns the first match
    Page p = make_page({{"NAME","first"},{"NAME","second"}});
    EXPECT_EQ(p.getSectionCount(), 2u);
    EXPECT_EQ(p.getSection("NAME").getContent(), "first");
}

TEST(Page, SetTitleDoesNotAffectSectionCount) {
    Page p = make_page({{"A","c"}});
    p.setTitle("My Page");
    EXPECT_EQ(p.getSectionCount(), 1u);
}
