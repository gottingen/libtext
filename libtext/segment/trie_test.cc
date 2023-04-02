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


#include "libtext/segment/dict_trie.h"
#include "libtext/segment/mps_seg.h"
#include <turbo/strings/str_join.h>
#include "gtest/gtest.h"

using namespace libtext;

static const char* const DICT_FILE = "../test/testdata/extra_dict/jieba.dict.small.utf8";

TEST(TrieTest, Empty) {
  std::vector<Unicode> keys;
  std::vector<const DictUnit*> values;
  Trie trie(keys, values);
}

TEST(TrieTest, Construct) {
  std::vector<Unicode> keys;
  std::vector<const DictUnit*> values;
  keys.push_back(DecodeRunesInString("你"));
  values.push_back((const DictUnit*)(nullptr));
  Trie trie(keys, values);
}

TEST(DictTrieTest, NewAndDelete) {
  DictTrie * trie;
  trie = new DictTrie(DICT_FILE);
  delete trie;
}

TEST(DictTrieTest, Test1) {
  std::string s1, s2;
  DictTrie trie(DICT_FILE);
  ASSERT_LT(trie.GetMinWeight() + 15.6479, 0.001);
  std::string word("来到");
  libtext::RuneStrArray uni;
  ASSERT_TRUE(DecodeRunesInString(word, uni));
  //DictUnit nodeInfo;
  //nodeInfo.word = uni;
  //nodeInfo.tag = "v";
  //nodeInfo.weight = -8.87033;
  //s1 << nodeInfo;
  //s2 << (*trie.Find(uni.begin(), uni.end()));
  const DictUnit* du = trie.Find(uni.begin(), uni.end());
  ASSERT_TRUE(du != nullptr);
  ASSERT_EQ(2u, du->word.size());
  ASSERT_EQ(26469u, du->word[0]);
  ASSERT_EQ(21040u, du->word[1]);
  ASSERT_EQ("v", du->tag);
  ASSERT_NEAR(-8.870, du->weight, 0.001);

  //EXPECT_EQ("[\"26469\", \"21040\"] v -8.870", s2);
  word = "清华大学";
  turbo::InlinedVector<std::pair<size_t, const DictUnit*>, 8> res;
  const char * words[] = {"清", "清华", "清华大学"};
  for (size_t i = 0; i < sizeof(words)/sizeof(words[0]); i++) {
    ASSERT_TRUE(DecodeRunesInString(words[i], uni));
    res.push_back(std::make_pair(uni.size() - 1, trie.Find(uni.begin(), uni.end())));
    //resMap[uni.size() - 1] = trie.Find(uni.begin(), uni.end());
  }
  std::vector<std::pair<size_t, const DictUnit*> > vec;
  std::vector<struct Dag> dags;
  ASSERT_TRUE(DecodeRunesInString(word, uni));
  trie.Find(uni.begin(), uni.end(), dags);
  ASSERT_EQ(dags.size(), uni.size());
  ASSERT_NE(dags.size(), 0u);
  ASSERT_EQ(res, dags[0].nexts);
  
}

TEST(DictTrieTest, UserDict) {
  DictTrie trie(DICT_FILE, "../test/testdata/userdict.utf8");
  std::string word = "云计算";
  libtext::RuneStrArray unicode;
  ASSERT_TRUE(DecodeRunesInString(word, unicode));
  const DictUnit * unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit != nullptr);
  ASSERT_NEAR(unit->weight, -14.100, 0.001);

  word = "蓝翔";
  ASSERT_TRUE(DecodeRunesInString(word, unicode));
  unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit != nullptr);
  ASSERT_EQ(unit->tag, "nz");
  ASSERT_NEAR(unit->weight, -14.100, 0.001);

  word = "区块链";
  ASSERT_TRUE(DecodeRunesInString(word, unicode));
  unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit != nullptr);
  ASSERT_EQ(unit->tag, "nz");
  ASSERT_NEAR(unit->weight, -15.6478, 0.001);
}

TEST(DictTrieTest, UserDictWithMaxWeight) {
  DictTrie trie(DICT_FILE, "../test/testdata/userdict.utf8", DictTrie::WordWeightMax);
  std::string word = "云计算";
  libtext::RuneStrArray unicode;
  ASSERT_TRUE(DecodeRunesInString(word, unicode));
  const DictUnit * unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit);
  ASSERT_NEAR(unit->weight, -2.975, 0.001);
}

TEST(DictTrieTest, Dag) {
  DictTrie trie(DICT_FILE, "../test/testdata/userdict.utf8");

  {
    std::string word = "清华大学";
    libtext::RuneStrArray unicode;
    ASSERT_TRUE(DecodeRunesInString(word, unicode));
    std::vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res);

    size_t nexts_sizes[] = {3, 2, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    std::string word = "北京邮电大学";
    libtext::RuneStrArray unicode;
    ASSERT_TRUE(DecodeRunesInString(word, unicode));
    std::vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res);

    size_t nexts_sizes[] = {3, 1, 2, 2, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    std::string word = "长江大桥";
    libtext::RuneStrArray unicode;
    ASSERT_TRUE(DecodeRunesInString(word, unicode));
    std::vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res);

    size_t nexts_sizes[] = {3, 1, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    std::string word = "长江大桥";
    libtext::RuneStrArray unicode;
    ASSERT_TRUE(DecodeRunesInString(word, unicode));
    std::vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res, 3);

    size_t nexts_sizes[] = {2, 1, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    std::string word = "长江大桥";
    libtext::RuneStrArray unicode;
    ASSERT_TRUE(DecodeRunesInString(word, unicode));
    std::vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res, 4);

    size_t nexts_sizes[] = {3, 1, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }
}
