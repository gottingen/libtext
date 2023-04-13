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

#include "libtext/jieba/keyword_extrator.h"
#include "gtest/gtest.h"

using namespace libtext;

TEST(KeywordExtractorTest, Test1) {
  KeywordExtractor Extractor("../test/testdata/extra_dict/jieba.dict.small.utf8", "../dict/hmm_model.utf8", "../dict/idf.utf8", "../dict/stop_words.utf8");
  KeywordExtractorWordFormatter f;
  {
    std::string s("你好世界世界而且而且");
    std::string res;
    size_t topN = 5;

    {
      std::vector<std::string> words;
      Extractor.Extract(s, words, topN);
      res = turbo::StrJoin(words, ", ");
      ASSERT_EQ(res, "世界, 你好");
    }

    {
      std::vector<std::pair<std::string, double> > words;
      Extractor.Extract(s, words, topN);
     res = turbo::StrJoin(words, ", ", turbo::PairFormatter(":"));
      ASSERT_EQ(res, "世界:8.73506, 你好:7.95788");
    }

    {
      std::vector<KeywordExtractor::Word> words;
      Extractor.Extract(s, words, topN);
     res = turbo::StrJoin(words, ", ", f);
      ASSERT_EQ(res, "{\"word\": \"\xE4\xB8\x96\xE7\x95\x8C\", \"offset\": [6, 12], \"weight\": 8.73506}, {\"word\": \"\xE4\xBD\xA0\xE5\xA5\xBD\", \"offset\": [0], \"weight\": 7.95788}");
    }
  }

  {
    std::string s("我是拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上CEO，走上人生巅峰。");
    std::string res;
    std::vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", f);
    ASSERT_EQ(res, "{\"word\": \"CEO\", \"offset\": [93], \"weight\": 11.7392}, {\"word\": \"\xE5\x8D\x87\xE8\x81\x8C\", \"offset\": [72], \"weight\": 10.8562}, {\"word\": \"\xE5\x8A\xA0\xE8\x96\xAA\", \"offset\": [78], \"weight\": 10.6426}, {\"word\": \"\xE6\x89\x8B\xE6\x89\xB6\xE6\x8B\x96\xE6\x8B\x89\xE6\x9C\xBA\", \"offset\": [21], \"weight\": 10.0089}, {\"word\": \"\xE5\xB7\x85\xE5\xB3\xB0\", \"offset\": [111], \"weight\": 9.49396}");
  }

  {
    std::string s("一部iPhone6");
    std::string res;
    std::vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", f);
    ASSERT_EQ(res, "{\"word\": \"iPhone6\", \"offset\": [6], \"weight\": 11.7392}, {\"word\": \"\xE4\xB8\x80\xE9\x83\xA8\", \"offset\": [0], \"weight\": 6.47592}");
  }
}

TEST(KeywordExtractorTest, Test2) {
  KeywordExtractor Extractor("../test/testdata/extra_dict/jieba.dict.small.utf8", "../dict/hmm_model.utf8", "../dict/idf.utf8", "../dict/stop_words.utf8", "../test/testdata/userdict.utf8");
  KeywordExtractorWordFormatter f;
  {
    std::string s("蓝翔优秀毕业生");
    std::string res;
    std::vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", f);
    ASSERT_EQ(res, "{\"word\": \"\xE8\x93\x9D\xE7\xBF\x94\", \"offset\": [0], \"weight\": 11.7392}, {\"word\": \"\xE6\xAF\x95\xE4\xB8\x9A\xE7\x94\x9F\", \"offset\": [12], \"weight\": 8.13549}, {\"word\": \"\xE4\xBC\x98\xE7\xA7\x80\", \"offset\": [6], \"weight\": 6.78347}");
  }

  {
    std::string s("一部iPhone6");
    std::string res;
    std::vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res = turbo::StrJoin(wordweights, ", ", f);
    ASSERT_EQ(res, "{\"word\": \"iPhone6\", \"offset\": [6], \"weight\": 11.7392}, {\"word\": \"\xE4\xB8\x80\xE9\x83\xA8\", \"offset\": [0], \"weight\": 6.47592}");
  }
}
