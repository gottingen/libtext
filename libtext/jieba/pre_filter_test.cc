// Copyright 2023 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "gtest/gtest.h"
#include "libtext/jieba/pre_filter.h"
#include <turbo/strings/str_join.h>

using namespace libtext;

TEST(PreFilterTest, Test1) {
  turbo::flat_hash_set<Rune> symbol;
  symbol.insert(65292u); // "，"
  symbol.insert(12290u); // "。"
  std::string expected;
  std::string res;

  {
    std::string s = "你好，美丽的，世界";
    PreFilter filter(symbol, s);
    expected = "你好/，/美丽的/，/世界";
    ASSERT_TRUE(filter.HasNext());
    std::vector<std::string> words;
    while (filter.HasNext()) {
      PreFilter::Range range;
      range = filter.Next();
      words.push_back(GetStringFromRunes(s, range.begin, range.end - 1));
    }
    res = turbo::StrJoin(words.begin(), words.end(), "/");
    ASSERT_EQ(res, expected);
  }

  {
    std::string s = "我来自北京邮电大学。。。学号123456，用AK47";
    PreFilter filter(symbol, s);
    expected = "我来自北京邮电大学/。/。/。/学号123456/，/用AK47";
    ASSERT_TRUE(filter.HasNext());
    std::vector<std::string> words;
    while (filter.HasNext()) {
      PreFilter::Range range;
      range = filter.Next();
      words.push_back(GetStringFromRunes(s, range.begin, range.end - 1));
    }
    res = turbo::StrJoin(words.begin(), words.end(), "/");
    ASSERT_EQ(res, expected);
  }
}
