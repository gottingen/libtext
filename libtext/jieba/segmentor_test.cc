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


#include "libtext/jieba/seg.h"
#include <turbo/strings/str_join.h>
#include "gtest/gtest.h"

using namespace libtext;

TEST(JiebaTest, Test1) {
  libtext::Segmentor jieba("../dict/jieba.dict.utf8",
                        "../dict/hmm_model.utf8",
                        "../dict/user.dict.utf8",
                        "../dict/idf.utf8",
                        "../dict/stop_words.utf8");
  std::vector<std::string> words;
  std::string result;

  jieba.Cut("他来到了网易杭研大厦", words);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("他, 来到, 了, 网易, 杭研, 大厦", result);

  jieba.Cut("我来自北京邮电大学。", words, false);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("我, 来自, 北京邮电大学, 。", result);

  jieba.CutSmall("南京市长江大桥", words, 3);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("南京市, 长江, 大桥", result);

  jieba.CutHMM("我来自北京邮电大学。。。学号123456", words);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("我来, 自北京, 邮电大学, 。, 。, 。, 学号, 123456", result);

  jieba.Cut("我来自北京邮电大学。。。学号123456，用AK47", words);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("我, 来自, 北京邮电大学, 。, 。, 。, 学号, 123456, ，, 用, AK47", result);

  jieba.CutAll("我来自北京邮电大学", words);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ(result, "我, 来自, 北京, 北京邮电, 北京邮电大学, 邮电, 邮电大学, 电大, 大学");

  jieba.CutForSearch("他来到了网易杭研大厦", words);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("他, 来到, 了, 网易, 杭研, 大厦", result);

}
TEST(JiebaTest, WordTest) {
  libtext::Segmentor jieba("../dict/jieba.dict.utf8",
                        "../dict/hmm_model.utf8",
                        "../dict/user.dict.utf8",
                        "../dict/idf.utf8",
                        "../dict/stop_words.utf8");
  std::vector<Word> words;
  std::string result;
  WordFormatter wf;
  jieba.Cut("他来到了网易杭研大厦", words);
  result  =  turbo::StrJoin(words, ", ", wf);
  ASSERT_EQ("{word: \xE4\xBB\x96, offset: 0}, {word: \xE6\x9D\xA5\xE5\x88\xB0, offset: 3}, {word: \xE4\xBA\x86, offset: 9}, {word: \xE7\xBD\x91\xE6\x98\x93, offset: 12}, {word: \xE6\x9D\xAD\xE7\xA0\x94, offset: 18}, {word: \xE5\xA4\xA7\xE5\x8E\xA6, offset: 24}", result);

  jieba.Cut("我来自北京邮电大学。", words, false);
  result  =  turbo::StrJoin(words, ", ", wf);
  //ASSERT_EQ("[我, 来自, 北京邮电大学, 。]", result);
  ASSERT_EQ("{word: \xE6\x88\x91, offset: 0}, {word: \xE6\x9D\xA5\xE8\x87\xAA, offset: 3}, {word: \xE5\x8C\x97\xE4\xBA\xAC\xE9\x82\xAE\xE7\x94\xB5\xE5\xA4\xA7\xE5\xAD\xA6, offset: 9}, {word: \xE3\x80\x82, offset: 27}", result);

  jieba.CutSmall("南京市长江大桥", words, 3);
  //ASSERT_EQ("[南京市, 长江, 大桥]", result << words);
  result  =  turbo::StrJoin(words, ", ", wf);
  ASSERT_EQ("{word: \xE5\x8D\x97\xE4\xBA\xAC\xE5\xB8\x82, offset: 0}, {word: \xE9\x95\xBF\xE6\xB1\x9F, offset: 9}, {word: \xE5\xA4\xA7\xE6\xA1\xA5, offset: 15}", result);

  jieba.CutHMM("我来自北京邮电大学。。。学号123456", words);
  result  =  turbo::StrJoin(words, ", ", wf);
  ASSERT_EQ("{word: \xE6\x88\x91\xE6\x9D\xA5, offset: 0}, {word: \xE8\x87\xAA\xE5\x8C\x97\xE4\xBA\xAC, offset: 6}, {word: \xE9\x82\xAE\xE7\x94\xB5\xE5\xA4\xA7\xE5\xAD\xA6, offset: 15}, {word: \xE3\x80\x82, offset: 27}, {word: \xE3\x80\x82, offset: 30}, {word: \xE3\x80\x82, offset: 33}, {word: \xE5\xAD\xA6\xE5\x8F\xB7, offset: 36}, {word: 123456, offset: 42}", result);

  jieba.Cut("我来自北京邮电大学。。。学号123456，用AK47", words);
  result  =  turbo::StrJoin(words, ", ", wf);
  //ASSERT_EQ("[我, 来自, 北京邮电大学, 。, 。, 。, 学号, 123456, ，, 用, AK47]", result);
  ASSERT_EQ("{word: \xE6\x88\x91, offset: 0}, {word: \xE6\x9D\xA5\xE8\x87\xAA, offset: 3}, {word: \xE5\x8C\x97\xE4\xBA\xAC\xE9\x82\xAE\xE7\x94\xB5\xE5\xA4\xA7\xE5\xAD\xA6, offset: 9}, {word: \xE3\x80\x82, offset: 27}, {word: \xE3\x80\x82, offset: 30}, {word: \xE3\x80\x82, offset: 33}, {word: \xE5\xAD\xA6\xE5\x8F\xB7, offset: 36}, {word: 123456, offset: 42}, {word: \xEF\xBC\x8C, offset: 48}, {word: \xE7\x94\xA8, offset: 51}, {word: AK47, offset: 54}", result);

  jieba.CutAll("我来自北京邮电大学", words);
  result  =  turbo::StrJoin(words, ", ", wf);
  //ASSERT_EQ(result, "[我, 来自, 北京, 北京邮电, 北京邮电大学, 邮电, 邮电大学, 电大, 大学]");
  ASSERT_EQ("{word: \xE6\x88\x91, offset: 0}, {word: \xE6\x9D\xA5\xE8\x87\xAA, offset: 3}, {word: \xE5\x8C\x97\xE4\xBA\xAC, offset: 9}, {word: \xE5\x8C\x97\xE4\xBA\xAC\xE9\x82\xAE\xE7\x94\xB5, offset: 9}, {word: \xE5\x8C\x97\xE4\xBA\xAC\xE9\x82\xAE\xE7\x94\xB5\xE5\xA4\xA7\xE5\xAD\xA6, offset: 9}, {word: \xE9\x82\xAE\xE7\x94\xB5, offset: 15}, {word: \xE9\x82\xAE\xE7\x94\xB5\xE5\xA4\xA7\xE5\xAD\xA6, offset: 15}, {word: \xE7\x94\xB5\xE5\xA4\xA7, offset: 18}, {word: \xE5\xA4\xA7\xE5\xAD\xA6, offset: 21}", result);

  jieba.CutForSearch("他来到了网易杭研大厦", words);
  result  =  turbo::StrJoin(words, ", ", wf);
  //ASSERT_EQ("[他, 来到, 了, 网易, 杭研, 大厦]", result);
  ASSERT_EQ("{word: \xE4\xBB\x96, offset: 0}, {word: \xE6\x9D\xA5\xE5\x88\xB0, offset: 3}, {word: \xE4\xBA\x86, offset: 9}, {word: \xE7\xBD\x91\xE6\x98\x93, offset: 12}, {word: \xE6\x9D\xAD\xE7\xA0\x94, offset: 18}, {word: \xE5\xA4\xA7\xE5\x8E\xA6, offset: 24}", result);
}

TEST(JiebaTest, InsertUserWord) {
  libtext::Segmentor jieba("../dict/jieba.dict.utf8",
                        "../dict/hmm_model.utf8",
                        "../dict/user.dict.utf8",
                        "../dict/idf.utf8",
                        "../dict/stop_words.utf8");
  std::vector<std::string> words;
  std::string result;

  jieba.Cut("男默女泪", words);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("男默, 女泪", result);

  ASSERT_TRUE(jieba.InsertUserWord("男默女泪"));

  jieba.Cut("男默女泪", words);
  result  =  turbo::StrJoin(words, ", ");
  ASSERT_EQ("男默女泪", result);

  for (size_t i = 0; i < 100; i++) {
    std::string newWord;
    turbo::StrAppend(&newWord, rand());
    ASSERT_TRUE(jieba.InsertUserWord(newWord));
    jieba.Cut(newWord, words);
    result  =  turbo::StrJoin(words, ", ");
    ASSERT_EQ(result, newWord);
  }

  ASSERT_TRUE(jieba.InsertUserWord("同一个世界，同一个梦想"));
  jieba.Cut("同一个世界，同一个梦想", words);
  result = turbo::StrJoin(words.begin(), words.end(), "/");
  ASSERT_EQ(result, "同一个/世界/，/同一个/梦想");

  jieba.ResetSeparators("");

  jieba.Cut("同一个世界，同一个梦想", words);
  result = turbo::StrJoin(words.begin(), words.end(), "/");
  ASSERT_EQ(result, "同一个世界，同一个梦想");

  {
    std::string s("一部iPhone6");
    std::string res;
    std::vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    jieba.extractor.Extract(s, wordweights, topN);
    res  =  turbo::StrJoin(wordweights, ", ", KeywordExtractorWordFormatter());
    ASSERT_EQ(res, "{\"word\": \"iPhone6\", \"offset\": [6], \"weight\": 11.7392}, {\"word\": \"\xE4\xB8\x80\xE9\x83\xA8\", \"offset\": [0], \"weight\": 6.47592}");
  }
}
