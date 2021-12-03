#include "CFnMatch.h"

#include "gtest/gtest.h"


TEST(CFnMatch, SetFilter_Empty)
{
    CFnMatch match;
    match.SetPattern("");
    EXPECT_EQ(match.GetPattern(), "");
}

TEST(CFnMatch, SetFilter_Text)
{
    CFnMatch match;
    match.SetPattern("abc");
    EXPECT_EQ(match.GetPattern(), "abc");
}

TEST(CFnMatch, SetFilter_QuestionMarks)
{
    CFnMatch match;
    match.SetPattern("??a??b??c??");
    EXPECT_EQ(match.GetPattern(), "??a??b??c??");
}

TEST(CFnMatch, SetFilter_Asterisks)
{
    CFnMatch match;
    match.SetPattern("***abc***def***");
    EXPECT_EQ(match.GetPattern(), "***abc***def***");
}

//////////////////////////////////////////////////////////////////////////

TEST(CFnMatch, MatchEmpty)
{
    CFnMatch match;
    match.SetPattern("");
    EXPECT_TRUE(match.FullMatch(""));
    EXPECT_FALSE(match.FullMatch("a"));
}

TEST(CFnMatch, MatchText)
{
    CFnMatch match;
    match.SetPattern("abc");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_FALSE(match.FullMatch("a"));
    EXPECT_FALSE(match.FullMatch("cba"));
    EXPECT_TRUE(match.FullMatch("abc"));
    EXPECT_FALSE(match.FullMatch("pre abc post"));
}

TEST(CFnMatch, MatchQuestionMark1)
{
    CFnMatch match;
    match.SetPattern("?");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_TRUE(match.FullMatch("a"));
    EXPECT_TRUE(match.FullMatch("?"));
    EXPECT_TRUE(match.FullMatch("*"));
    EXPECT_FALSE(match.FullMatch("abc"));
}

TEST(CFnMatch, MatchQuestionMark2)
{
    CFnMatch match;
    match.SetPattern("ab??cd");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_FALSE(match.FullMatch("abcd"));
    EXPECT_FALSE(match.FullMatch("abXcd"));
    EXPECT_TRUE(match.FullMatch("ab??cd"));
    EXPECT_TRUE(match.FullMatch("abXYcd"));
    EXPECT_FALSE(match.FullMatch("abXYZcd"));
}

TEST(CFnMatch, MatchAsterisk1)
{
    CFnMatch match;
    match.SetPattern("*");
    EXPECT_TRUE(match.FullMatch(""));
    EXPECT_TRUE(match.FullMatch("a"));
    EXPECT_TRUE(match.FullMatch("abc"));
    EXPECT_TRUE(match.FullMatch("?"));
    EXPECT_TRUE(match.FullMatch("???"));
    EXPECT_TRUE(match.FullMatch("*"));
    EXPECT_TRUE(match.FullMatch("***"));
}

TEST(CFnMatch, MatchAsterisk1Prefix)
{
    CFnMatch match;
    match.SetPattern("a*");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_TRUE(match.FullMatch("a"));
    EXPECT_TRUE(match.FullMatch("abc"));
    EXPECT_FALSE(match.FullMatch("Xa"));
    EXPECT_FALSE(match.FullMatch("Xabc"));
    EXPECT_FALSE(match.FullMatch("**"));
    EXPECT_FALSE(match.FullMatch("?*"));
}

TEST(CFnMatch, MatchAsterisk1Postfix)
{
    CFnMatch match;
    match.SetPattern("*a");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_TRUE(match.FullMatch("a"));
    EXPECT_TRUE(match.FullMatch("cba"));
    EXPECT_FALSE(match.FullMatch("aX"));
    EXPECT_FALSE(match.FullMatch("cbaX"));
    EXPECT_FALSE(match.FullMatch("**"));
    EXPECT_FALSE(match.FullMatch("*?"));
}

TEST(CFnMatch, MatchAsterisk1Inside)
{
    CFnMatch match;
    match.SetPattern("ab*cd");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_FALSE(match.FullMatch("<abcd>"));
    EXPECT_TRUE(match.FullMatch("abcd"));
    EXPECT_TRUE(match.FullMatch("abXYXcd"));
}

TEST(CFnMatch, MatchAsterisk2Outside)
{
    CFnMatch match;
    match.SetPattern("*abc*");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_TRUE(match.FullMatch("abc"));
    EXPECT_TRUE(match.FullMatch("<abcd>"));
    EXPECT_FALSE(match.FullMatch("a-b-c"));
}

TEST(CFnMatch, MatchAsterisk3)
{
    CFnMatch match;
    match.SetPattern("*ab*cd*");
    EXPECT_FALSE(match.FullMatch(""));
    EXPECT_FALSE(match.FullMatch("a b c d"));
    EXPECT_TRUE(match.FullMatch("abcd"));
    EXPECT_TRUE(match.FullMatch("-=<ab><cd>=-"));
}

TEST(CFnMatch, MatchSpeedTest1)
{
    CFnMatch match;
    match.SetPattern("*******************");
    EXPECT_TRUE(match.FullMatch("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
}

TEST(CFnMatch, MatchSpeedTest2)
{
    CFnMatch match;
    match.SetPattern("*a*a*a*a*a*a*a*a*a*a*a*a*a*a*");
    EXPECT_TRUE(match.FullMatch("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
}
