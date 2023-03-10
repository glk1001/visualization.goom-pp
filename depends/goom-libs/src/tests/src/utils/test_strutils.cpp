#include "utils/enum_utils.h"
#include "utils/strutils.h"

#include <string>
#include <vector>

#if __clang_major__ >= 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

using UTILS::EnumToString;
using UTILS::FindAndReplaceAll;
using UTILS::StringJoin;
using UTILS::StringSplit;

TEST_CASE("FindAndReplaceAll")
{
  auto str               = std::string{"hello Everyone out there. hello again and Hello again."};
  const auto expectedStr = std::string{"Hello Everyone out there. Hello again and Hello again."};

  FindAndReplaceAll(str, "hello", "Hello");
  REQUIRE(str == expectedStr);

  FindAndReplaceAll(str, "zzz", "ZZZ");
  REQUIRE(str == expectedStr);
}

TEST_CASE("StringJoin")
{
  REQUIRE(StringJoin({""}, ", ").empty());
  REQUIRE("word1" == StringJoin({"word1"}, ", "));
  REQUIRE("word1, word2, word3" == StringJoin({"word1", "word2", "word3"}, ", "));
  REQUIRE("word1, word2, word3," == StringJoin({"word1", "word2", "word3,"}, ", "));
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("StringSplit")
{
  const auto testString1 = std::string{"line1: word1, word2\nline2: word3, word4\n"};

  const auto test1 = StringSplit(testString1, ",");
  UNSCOPED_INFO("testString1 = \"" << testString1 + "\"");
  for (const auto& str : test1)
  {
    UNSCOPED_INFO("str = " << str);
  }
  REQUIRE(test1.size() == 3);
  REQUIRE(test1[0] == "line1: word1");
  REQUIRE(test1[1] == " word2\nline2: word3");
  REQUIRE(test1[2] == " word4\n");

  const auto test2 = StringSplit(testString1, "\n");
  REQUIRE(test2.size() == 2);
  REQUIRE(test2[0] == "line1: word1, word2");
  REQUIRE(test2[1] == "line2: word3, word4");

  const auto testString2 = std::string{"word1; word2; word3; word4"};
  const auto test3       = StringSplit(testString2, "; ");
  REQUIRE(test3.size() == 4);
  REQUIRE(test3[0] == "word1");
  REQUIRE(test3[1] == "word2");
  REQUIRE(test3[2] == "word3");
  REQUIRE(test3[3] == "word4");

  const auto testString3 = std::string{"word1 \nword2\nword3 \nword4 "};
  const auto test4       = StringSplit(testString3, "\n");
  REQUIRE(test4.size() == 4);
  REQUIRE(test4[0] == "word1 ");
  REQUIRE(test4[1] == "word2");
  REQUIRE(test4[2] == "word3 ");
  REQUIRE(test4[3] == "word4 ");
}
// NOLINTEND(readability-function-cognitive-complexity)

TEST_CASE("EnumToString")
{
#ifdef NO_MAGIC_ENUM_AVAILABLE
  return;
#else
  enum class EnumTester
  {
    _NULL = -1, // NOLINT: Need special name here
    TEST1,
    TEST2,
    TEST3,
    _num // unused, and marks the enum end
  };

  auto test = EnumTester::_NULL;
  REQUIRE(EnumToString(test) == "_NULL");
  test = EnumTester::TEST1;
  REQUIRE(EnumToString(test) == "TEST1");
  test = EnumTester::TEST2;
  REQUIRE(EnumToString(test) == "TEST2");
  test = EnumTester::_num;
  REQUIRE(EnumToString(test) == "_num");

  REQUIRE(EnumToString(EnumTester::TEST3) == "TEST3");
#endif
}

} // namespace GOOM::UNIT_TESTS
