#include "CFnMatch.h"

#include "gtest/gtest.h"


TEST(CFnMatch, SetFilter_Empty)
{
    CFnMatch match(10);
    match.SetFilter("");
    EXPECT_EQ(match.GetFilter(), "");
}

TEST(CFnMatch, SetFilter_Text)
{
    CFnMatch match(10);
    match.SetFilter("abc");
    EXPECT_EQ(match.GetFilter(), "abc");
}

TEST(CFnMatch, SetFilter_QuestionMark1)
{
    CFnMatch match(10);
    match.SetFilter("?");
    EXPECT_EQ(match.GetFilter(), "?");
}

TEST(CFnMatch, SetFilter_QuestionMarks)
{
    CFnMatch match(10);
    match.SetFilter("??a??b??c??");
    EXPECT_EQ(match.GetFilter(), "??a??b??c??");
}

TEST(CFnMatch, SetFilter_Asterisk1)
{
    CFnMatch match(10);
    match.SetFilter("*");
    EXPECT_EQ(match.GetFilter(), "*");
}

TEST(CFnMatch, SetFilter_Asterisk3)
{
    CFnMatch match(10);
    match.SetFilter("***");
    EXPECT_EQ(match.GetFilter(), "*");
}

TEST(CFnMatch, SetFilter_Asterisks)
{
    CFnMatch match(10);
    match.SetFilter("**a***b**");
    EXPECT_EQ(match.GetFilter(), "*a*b*");
}

//////////////////////////////////////////////////////////////////////////

TEST(CFnMatch, MatchEmpty)
{
    CFnMatch match(20);
    match.SetFilter("");
    EXPECT_TRUE(match.CheckMatch(""));
    EXPECT_FALSE(match.CheckMatch("a"));
}

TEST(CFnMatch, MatchText)
{
    CFnMatch match(20);
    match.SetFilter("abc");
    EXPECT_FALSE(match.CheckMatch(""));
    EXPECT_FALSE(match.CheckMatch("a"));
    EXPECT_FALSE(match.CheckMatch("cba"));
    EXPECT_TRUE(match.CheckMatch("abc"));
    EXPECT_FALSE(match.CheckMatch("pre abc post"));
}

TEST(CFnMatch, MatchQuestionMark1)
{
    CFnMatch match(20);
    match.SetFilter("?");
    EXPECT_FALSE(match.CheckMatch(""));
    EXPECT_TRUE(match.CheckMatch("a"));
    EXPECT_TRUE(match.CheckMatch("?"));
    EXPECT_TRUE(match.CheckMatch("*"));
    EXPECT_FALSE(match.CheckMatch("abc"));
}

TEST(CFnMatch, MatchQuestionMark2)
{
    CFnMatch match(20);
    match.SetFilter("ab??cd");
    EXPECT_FALSE(match.CheckMatch(""));
    EXPECT_FALSE(match.CheckMatch("abcd"));
    EXPECT_FALSE(match.CheckMatch("abXcd"));
    EXPECT_TRUE(match.CheckMatch("ab??cd"));
    EXPECT_TRUE(match.CheckMatch("abXYcd"));
    EXPECT_FALSE(match.CheckMatch("abXYZcd"));
}

TEST(CFnMatch, MatchAsterisk1)
{
    CFnMatch match(20);
    match.SetFilter("*");
    EXPECT_TRUE(match.CheckMatch(""));
    EXPECT_TRUE(match.CheckMatch("a"));
    EXPECT_TRUE(match.CheckMatch("abc"));
    EXPECT_TRUE(match.CheckMatch("?"));
    EXPECT_TRUE(match.CheckMatch("???"));
    EXPECT_TRUE(match.CheckMatch("*"));
    EXPECT_TRUE(match.CheckMatch("***"));
}

TEST(CFnMatch, MatchAsterisk1Inside)
{
    CFnMatch match(20);
    match.SetFilter("ab*cd");
    EXPECT_FALSE(match.CheckMatch(""));
    EXPECT_FALSE(match.CheckMatch("<abcd>"));
    EXPECT_TRUE(match.CheckMatch("abcd"));
    EXPECT_TRUE(match.CheckMatch("abXYXcd"));
}

TEST(CFnMatch, MatchAsterisk3)
{
    CFnMatch match(20);
    match.SetFilter("*ab*cd*");
    EXPECT_FALSE(match.CheckMatch(""));
    EXPECT_FALSE(match.CheckMatch("a b c d"));
    EXPECT_TRUE(match.CheckMatch("abcd"));
    EXPECT_TRUE(match.CheckMatch("-=<ab><cd>=-"));
}
