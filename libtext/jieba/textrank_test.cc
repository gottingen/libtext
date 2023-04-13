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


#include "libtext/jieba/text_rank_extractor.h"
#include "gtest/gtest.h"

using namespace libtext;

TEST(TextRankExtractorTest, Test1) {
  TextRankExtractor Extractor(
    "../test/testdata/extra_dict/jieba.dict.small.utf8",
    "../dict/hmm_model.utf8", 
    "../dict/stop_words.utf8");
  {
    std::string s("你好世界世界而且而且");
    std::string res;
    size_t topN = 5;

    {
      std::vector<std::string> words;
      Extractor.Extract(s, words, topN);
      res = turbo::StrJoin(words,", ");
      ASSERT_EQ(res, "世界, 你好");
    }

    {
      std::vector<std::pair<std::string, double> > words;
      Extractor.Extract(s, words, topN);
      res = turbo::StrJoin(words, ", ", turbo::PairFormatter(":"));
      ASSERT_EQ(res, "世界:1, 你好:0.519787");
    }

    {
      std::vector<TextRankExtractor::Word> words;
      Extractor.Extract(s, words, topN);
      res = turbo::StrJoin(words, ", ", TextRankExtractorWordFormatter());
      ASSERT_EQ(res, "{word: 世界, offset: [6, 12], weight: 1}, {word: 你好, offset: [0], weight: 0.519787}");
    }
  }

  { 
    std::string s("\xe6\x88\x91\xe6\x98\xaf\xe6\x8b\x96\xe6\x8b\x89\xe6\x9c\xba\xe5\xad\xa6\xe9\x99\xa2\xe6\x89\x8b\xe6\x89\xb6\xe6\x8b\x96\xe6\x8b\x89\xe6\x9c\xba\xe4\xb8\x93\xe4\xb8\x9a\xe7\x9a\x84\xe3\x80\x82\xe4\xb8\x8d\xe7\x94\xa8\xe5\xa4\x9a\xe4\xb9\x85\xef\xbc\x8c\xe6\x88\x91\xe5\xb0\xb1\xe4\xbc\x9a\xe5\x8d\x87\xe8\x81\x8c\xe5\x8a\xa0\xe8\x96\xaa\xef\xbc\x8c\xe5\xbd\x93\xe4\xb8\x8a CEO\xef\xbc\x8c\xe8\xb5\xb0\xe4\xb8\x8a\xe4\xba\xba\xe7\x94\x9f\xe5\xb7\x85\xe5\xb3\xb0");
    std::string res;
    std::vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", TextRankExtractorWordFormatter());
    ASSERT_EQ(res, "{word: 当上, offset: [87], weight: 1}, {word: 不用, offset: [48], weight: 0.989848}, {word: 多久, offset: [54], weight: 0.985126}, {word: 加薪, offset: [78], weight: 0.983046}, {word: 升职, offset: [72], weight: 0.980278}");
    //ASSERT_EQ(res, "{word: 专业, offset: [36], weight: 1}, {word: CEO, offset: [94], weight: 0.95375}, {word: 手扶拖拉机, offset: [21], weight: 0.801701}, {word: 当上, offset: [87], weight: 0.798968}, {word: 走上, offset: [100], weight: 0.775505}");
  }

  {
    std::string s("一部iPhone6");
    std::string res;
    std::vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", TextRankExtractorWordFormatter());
    ASSERT_EQ(res, "{word: 一部, offset: [0], weight: 1}, {word: iPhone6, offset: [6], weight: 0.996126}");
  }
}

TEST(TextRankExtractorTest, Test2) {
  TextRankExtractor Extractor(
    "../test/testdata/extra_dict/jieba.dict.small.utf8",
    "../dict/hmm_model.utf8",
    "../dict/stop_words.utf8",
    "../test/testdata/userdict.utf8");

  {
    std::string s("\xe8\x93\x9d\xe7\xbf\x94\xe4\xbc\x98\xe7\xa7\x80\xe6\xaf\x95\xe4\xb8\x9a\xe7\x94\x9f");
    std::string res;
    std::vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", TextRankExtractorWordFormatter());
    ASSERT_EQ(res, "{word: 蓝翔, offset: [0], weight: 1}, {word: 毕业生, offset: [12], weight: 0.996685}, {word: 优秀, offset: [6], weight: 0.992994}");
  }

  {
    std::string s("一部iPhone6");
    std::string res;
    std::vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", TextRankExtractorWordFormatter());
    ASSERT_EQ(res, "{word: 一部, offset: [0], weight: 1}, {word: iPhone6, offset: [6], weight: 0.996126}");
  }
}
