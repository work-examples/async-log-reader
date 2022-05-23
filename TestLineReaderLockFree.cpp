#include "LineReader.h"

#include "TestHelpers.h"

#include <algorithm>
#include <string>

#include "gtest/gtest.h"


namespace
{
#   define CLineReader CSpinlockLineReader

    const size_t MaxLogLineLength = 1024; // copy-pasted value from LineReader.cpp
}


TEST(CLineReader, Open)
{
    TempFile file("");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    reader.Close();
}

TEST(CLineReader, MissedOpen)
{
    CLineReader reader;
    const auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, EmptyFile)
{
    TempFile file("");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    const auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, OneLineNoLF)
{
    TempFile file("ABCD");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "ABCD");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, OneLineCRLF)
{
    TempFile file("ABCD\r\n");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "ABCD\r\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, OneLineLF)
{
    TempFile file("ABCD\n");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "ABCD\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, TwoLinesLF_NoLF)
{
    TempFile file("abc\nDEFG");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "abc\n");
    line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "DEFG");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, TwoLinesLF_LF)
{
    TempFile file("abc\nDEFG\n");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "abc\n");
    line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "DEFG\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, EmptyLines)
{
    TempFile file("\n\n");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "\n");
    line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, ThreeLines)
{
    TempFile file("Abcdef\n\n3rd Line");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "Abcdef\n");
    line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "\n");
    line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(std::string(*line), "3rd Line");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineMaxLength_1)
{
    const std::string str = std::string(MaxLogLineLength, 'x');
    TempFile file(str);
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(*line, str);
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineMaxLength_2)
{
    const std::string str = std::string(MaxLogLineLength - 1, 'x');
    TempFile file(str + "\n");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(*line, str + "\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineTooLong_1)
{
    const std::string str = std::string(MaxLogLineLength + 1, 'x');
    TempFile file(str);
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineTooLong_2)
{
    const std::string str = std::string(MaxLogLineLength, 'x');
    TempFile file(str + "\n");
    CLineReader reader;
    EXPECT_TRUE(reader.Open(file.GetFilename().c_str()));
    auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}
