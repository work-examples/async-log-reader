#include "FnMatch.h"

#include "gtest/gtest.h"


TEST(CFnMatch, MatchEmpty)
{
    CFnMatch match;
    const char* const pattern = "";
    EXPECT_TRUE(match.Match("", pattern));
    EXPECT_FALSE(match.Match("a", pattern));
}

TEST(CFnMatch, MatchText)
{
    CFnMatch match;
    const char* const pattern = "abc";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_FALSE(match.Match("a", pattern));
    EXPECT_FALSE(match.Match("cba, pattern", pattern));
    EXPECT_TRUE(match.Match("abc", pattern));
    EXPECT_FALSE(match.Match("pre abc post", pattern));
}

TEST(CFnMatch, MatchQuestionMark1)
{
    CFnMatch match;
    const char* const pattern = "?";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_TRUE(match.Match("a", pattern));
    EXPECT_TRUE(match.Match("?", pattern));
    EXPECT_TRUE(match.Match("*", pattern));
    EXPECT_FALSE(match.Match("abc", pattern));
}

TEST(CFnMatch, MatchQuestionMark2)
{
    CFnMatch match;
    const char* const pattern = "ab??cd";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_FALSE(match.Match("abcd", pattern));
    EXPECT_FALSE(match.Match("abXcd", pattern));
    EXPECT_TRUE(match.Match("ab??cd", pattern));
    EXPECT_TRUE(match.Match("abXYcd", pattern));
    EXPECT_FALSE(match.Match("abXYZcd", pattern));
}

TEST(CFnMatch, MatchAsterisk1)
{
    CFnMatch match;
    const char* const pattern = "*";
    EXPECT_TRUE(match.Match("", pattern));
    EXPECT_TRUE(match.Match("a", pattern));
    EXPECT_TRUE(match.Match("abc", pattern));
    EXPECT_TRUE(match.Match("?", pattern));
    EXPECT_TRUE(match.Match("???", pattern));
    EXPECT_TRUE(match.Match("*", pattern));
    EXPECT_TRUE(match.Match("***", pattern));
}

TEST(CFnMatch, MatchAsterisk1Prefix)
{
    CFnMatch match;
    const char* const pattern = "a*";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_TRUE(match.Match("a", pattern));
    EXPECT_TRUE(match.Match("abc", pattern));
    EXPECT_FALSE(match.Match("Xa", pattern));
    EXPECT_FALSE(match.Match("Xabc", pattern));
    EXPECT_FALSE(match.Match("**", pattern));
    EXPECT_FALSE(match.Match("?*", pattern));
}

TEST(CFnMatch, MatchAsterisk1Postfix)
{
    CFnMatch match;
    const char* const pattern = "*a";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_TRUE(match.Match("a", pattern));
    EXPECT_TRUE(match.Match("cba", pattern));
    EXPECT_FALSE(match.Match("aX", pattern));
    EXPECT_FALSE(match.Match("cbaX", pattern));
    EXPECT_FALSE(match.Match("**", pattern));
    EXPECT_FALSE(match.Match("*?", pattern));
}

TEST(CFnMatch, MatchAsterisk1Inside)
{
    CFnMatch match;
    const char* const pattern = "ab*cd";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_FALSE(match.Match("<abcd>", pattern));
    EXPECT_TRUE(match.Match("abcd", pattern));
    EXPECT_TRUE(match.Match("abXYXcd", pattern));
}

TEST(CFnMatch, MatchAsterisk2Outside)
{
    CFnMatch match;
    const char* const pattern = "*abc*";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_TRUE(match.Match("abc", pattern));
    EXPECT_TRUE(match.Match("<abcd>", pattern));
    EXPECT_FALSE(match.Match("a-b-c", pattern));
}

TEST(CFnMatch, MatchAsterisk3)
{
    CFnMatch match;
    const char* const pattern = "*ab*cd*";
    EXPECT_FALSE(match.Match("", pattern));
    EXPECT_FALSE(match.Match("a b c d", pattern));
    EXPECT_TRUE(match.Match("abcd", pattern));
    EXPECT_TRUE(match.Match("-=<ab><cd>=-", pattern));
}

TEST(CFnMatch, MatchSpeedTest1)
{
    CFnMatch match;
    const char* const pattern = "*******************";
    EXPECT_TRUE(match.Match("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", pattern));
}

TEST(CFnMatch, MatchSpeedTest2)
{
    CFnMatch match;
    const char* const pattern = "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*";
    EXPECT_TRUE(match.Match("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", pattern));
}
