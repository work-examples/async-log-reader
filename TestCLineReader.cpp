#include "CLineReader.h"

#include <algorithm>
#include <functional>
#include <string>

#include "gtest/gtest.h"


namespace
{
    class FileMock
    {
    public:
        FileMock(const std::string& data): _data(data)
        {
        }

        bool Read(char* buffer, const size_t bufferLength, size_t& readBytes)
        {
            readBytes = std::min(bufferLength, this->_data.size() - this->_dataOffset);
            memcpy(buffer, _data.data() + this->_dataOffset, readBytes);
            this->_dataOffset += readBytes;
            return true;
        }

        std::function<CLineReader::ReadDataFunc> GetReadFunc()
        {
            return std::bind(&FileMock::Read, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

    protected:
        std::string _data;
        size_t      _dataOffset = 0;
    };
}

TEST(CLineReader, MissedSetup)
{
    CLineReader reader;
    const auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, EmptyFile)
{
    FileMock file("");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    const auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, OneLineNoLF)
{
    FileMock file("ABCD");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "ABCD");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, OneLineCRLF)
{
    FileMock file("ABCD\r\n");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "ABCD\r\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, OneLineLF)
{
    FileMock file("ABCD\n");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "ABCD\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, TwoLinesLF_NoLF)
{
    FileMock file("abc\nDEFG");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "abc\n");
    line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "DEFG");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, TwoLinesLF_LF)
{
    FileMock file("abc\nDEFG\n");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "abc\n");
    line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "DEFG\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, EmptyLines)
{
    FileMock file("\n\n");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "\n");
    line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, ThreeLines)
{
    FileMock file("Abcdef\n\n3rd Line");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "Abcdef\n");
    line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "\n");
    line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, "3rd Line");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineMaxLength_1)
{
    const std::string str = std::string(CLineReader::g_MaxLogLineLength, 'x');
    FileMock file(str);
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, str);
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineMaxLength_2)
{
    const std::string str = std::string(CLineReader::g_MaxLogLineLength - 1, 'x');
    FileMock file(str + "\n");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_TRUE(line);
    EXPECT_EQ(*line, str + "\n");
    line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineTooLong_1)
{
    const std::string str = std::string(CLineReader::g_MaxLogLineLength + 1, 'x');
    FileMock file(str);
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}

TEST(CLineReader, LineTooLong_2)
{
    const std::string str = std::string(CLineReader::g_MaxLogLineLength, 'x');
    FileMock file(str + "\n");
    CLineReader reader;
    reader.Setup(file.GetReadFunc());
    auto line = reader.GetNextLine();
    EXPECT_FALSE(line);
}
