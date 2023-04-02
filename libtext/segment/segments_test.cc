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

#include "libtext/segment/full_seg.h"
#include "libtext/segment/hmm_seg.h"
#include "libtext/segment/mix_seg.h"
#include "libtext/segment/mps_seg.h"
#include "libtext/segment/query_seg.h"
#include "libtext/segment/seg_base.h"
#include "gtest/gtest.h"
#include <turbo/strings/str_join.h>

using namespace libtext;

TEST(MixSegmentTest, Test1) {
  MixSegment segment("../dict/jieba.dict.utf8", "../dict/hmm_model.utf8");
  ;
  std::string sentence;
  std::vector<std::string> words;
  std::string actual;
  std::string expected;

  {
    sentence = "我来自北京邮电大学。。。学号123456，用AK47";
    expected = "我/来自/北京邮电大学/。/。/。/学号/123456/，/用/AK47";
    segment.Cut(sentence, words);
    actual = turbo::StrJoin(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    sentence = "B超 T恤";
    expected = "B超/ /T恤";
    segment.Cut(sentence, words);
    actual = turbo::StrJoin(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    sentence = "他来到了网易杭研大厦";
    expected = "他/来到/了/网易/杭/研/大厦";
    segment.Cut(sentence, words, false);
    actual = turbo::StrJoin(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    sentence = "他来到了网易杭研大厦";
    expected = "他/来到/了/网易/杭研/大厦";
    segment.Cut(sentence, words);
    actual = turbo::StrJoin(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }
}

TEST(MixSegmentTest, NoUserDict) {
  MixSegment segment("../test/testdata/extra_dict/jieba.dict.small.utf8",
                     "../dict/hmm_model.utf8");
  const char *str = "令狐冲是云计算方面的专家";
  std::vector<std::string> words;
  segment.Cut(str, words);
  std::string res;
  res = turbo::StrJoin(words, ", ");
  ASSERT_EQ(
      "令狐冲, 是, 云, 计算, 方面, 的, 专家",
      res);
}
TEST(MixSegmentTest, UserDict) {
  MixSegment segment("../test/testdata/extra_dict/jieba.dict.small.utf8",
                     "../dict/hmm_model.utf8", "../dict/user.dict.utf8");
  {
    const char *str = "令狐冲是云计算方面的专家";
    std::vector<std::string> words;
    segment.Cut(str, words);
    std::string res;
    res = turbo::StrJoin(words, ", ");
    ASSERT_EQ("令狐冲, 是, 云计算, 方面, 的, 专家",
              res);
  }
  {
    const char *str = "小明先就职于IBM,后在日本京都大学深造";
    std::vector<std::string> words;
    segment.Cut(str, words);
    std::string res;
    res = turbo::StrJoin(words, ", ");
    ASSERT_EQ("小明, 先, 就职, 于, IBM, ,, 后, "
              "在, 日本, 京都大学, 深造",
              res);
  }
  {
    const char *str = "IBM,3.14";
    std::vector<std::string> words;
    segment.Cut(str, words);
    std::string res;
    res = turbo::StrJoin(words, ", ");
    ASSERT_EQ("IBM, ,, 3.14", res);
  }
}
TEST(MixSegmentTest, TestUserDict) {
  MixSegment segment("../test/testdata/extra_dict/jieba.dict.small.utf8",
                     "../dict/hmm_model.utf8",
                     "../test/testdata/userdict.utf8");
  std::vector<std::string> words;
  std::string res;

  segment.Cut("令狐冲是云计算方面的专家", words);
  res = turbo::StrJoin(words, ", ");
  ASSERT_EQ("令狐冲, 是, 云计算, 方面, 的, 专家",
            res);

  segment.Cut("小明先就职于IBM,后在日本京都大学深造", words);
  res = turbo::StrJoin(words, ", ");
  ASSERT_EQ("小明, 先, 就职, 于, I, B, M, ,, "
            "后, 在, 日本, 京都大学, 深造",
            res);

  segment.Cut("IBM,3.14", words);
  res = turbo::StrJoin(words, ", ");
  ASSERT_EQ("I, B, M, ,, 3.14", res);

  segment.Cut("忽如一夜春风来，千树万树梨花开", words);
  res = turbo::StrJoin(words.begin(), words.end(), "/");
  ASSERT_EQ("忽如一夜春风来/，/千树/万树/梨花/开", res);

  // rand input
  {
    const size_t ITERATION = 16;
    const size_t MAX_LEN = 256;
    std::string s;
    srand(time(NULL));

    for (size_t i = 0; i < ITERATION; i++) {
      size_t len = rand() % MAX_LEN;
      s.resize(len);
      for (size_t j = 0; j < len; j++) {
        s[rand() % len] = rand();
      }
      segment.Cut(s, words);
    }
  }
}

TEST(MixSegmentTest, TestMultiUserDict) {
  MixSegment segment(
      "../test/testdata/extra_dict/jieba.dict.small.utf8",
      "../dict/hmm_model.utf8",
      "../test/testdata/userdict.utf8;../test/testdata/userdict.2.utf8");
  std::vector<std::string> words;
  std::string res;

  segment.Cut("忽如一夜春风来，千树万树梨花开", words);
  res = turbo::StrJoin(words.begin(), words.end(), "/");
  ASSERT_EQ("忽如一夜春风来/，/千树万树梨花开", res);
}

TEST(MPSegmentTest, Test1) {
  MPSegment segment("../dict/jieba.dict.utf8");
  ;
  std::string s;
  std::vector<std::string> words;
  segment.Cut("我来自北京邮电大学。", words);
  s = turbo::StrJoin(words, ", ");
  ASSERT_EQ("我, 来自, 北京邮电大学, 。", s);

  segment.Cut("B超 T恤", words);
  s = turbo::StrJoin(words, ", ");
  ASSERT_EQ(s, "B超,  , T恤");

  segment.Cut("南京市长江大桥", words);
  s = turbo::StrJoin(words, ", ");
  ASSERT_EQ("南京市, 长江大桥", s);

  // MaxWordLen
  segment.Cut("南京市长江大桥", words, 3);
  s = turbo::StrJoin(words, ", ");
  ASSERT_EQ("南京市, 长江, 大桥", s);

  segment.Cut("南京市长江大桥", words, 0);
  s = turbo::StrJoin(words, ", ");
  ASSERT_EQ("南, 京, 市, 长, 江, 大, 桥", s);

  segment.Cut("湖南长沙市天心区", words);
  s = turbo::StrJoin(words.begin(), words.end(), "/");
  ASSERT_EQ("湖南长沙市/天心区", s);

  segment.Cut("湖南长沙市天心区", words, 3);
  s = turbo::StrJoin(words.begin(), words.end(), "/");
  ASSERT_EQ("湖南/长沙市/天心区", s);
}

TEST(HMMSegmentTest, Test1) {
  HMMSegment segment("../dict/hmm_model.utf8");
  ;
  {
    const char *str = "我来自北京邮电大学。。。学号123456";
    const char *res[] = {"我来", "自北京", "邮电大学", "。",
                         "。",   "。",     "学号",     "123456"};
    std::vector<std::string> words;
    segment.Cut(str, words);
    ASSERT_EQ(words, std::vector<std::string>(res, res + sizeof(res) /
                                                             sizeof(res[0])));
  }

  {
    const char *str = "IBM,1.2,123";
    const char *res[] = {"IBM", ",", "1.2", ",", "123"};
    std::vector<std::string> words;
    segment.Cut(str, words);
    ASSERT_EQ(words, std::vector<std::string>(res, res + sizeof(res) /
                                                             sizeof(res[0])));
  }
}

TEST(FullSegment, Test1) {
  FullSegment segment("../test/testdata/extra_dict/jieba.dict.small.utf8");
  std::vector<std::string> words;
  std::string s;

  segment.Cut("我来自北京邮电大学", words);
  s = turbo::StrJoin(words, ", ");
  ASSERT_EQ(s, "我, 来自, 北京, 北京邮电大学, 邮电, "
               "电大, 大学");

  segment.Cut("上市公司CEO", words);
  s = turbo::StrJoin(words, ", ");
  ASSERT_EQ(s, "上市, 公司, C, E, O");
}

TEST(QuerySegment, Test1) {
  QuerySegment segment("../dict/jieba.dict.utf8", "../dict/hmm_model.utf8", "");
  std::vector<std::string> words;
  std::string s1, s2;

  segment.Cut("小明硕士毕业于中国科学院计算所，后在日本京都大学深造", words);
  s1 = turbo::StrJoin(words.begin(), words.end(), "/");
  s2 = "小明/硕士/毕业/于/中国/科学/学院/科学院/中国科学院/计算/计算所/，/后/"
       "在/日本/京都/大学/日本京都大学/深造";
  ASSERT_EQ(s1, s2);

  segment.Cut("亲口交代", words);
  s1 = turbo::StrJoin(words.begin(), words.end(), "/");
  s2 = "亲口/交代";
  ASSERT_EQ(s1, s2);

  segment.Cut("他心理健康", words);
  s1 = turbo::StrJoin(words.begin(), words.end(), "/");
  s2 = "他/心理/健康/心理健康";
  ASSERT_EQ(s1, s2);
}

TEST(QuerySegment, Test2) {
  QuerySegment segment(
      "../test/testdata/extra_dict/jieba.dict.small.utf8",
      "../dict/hmm_model.utf8",
      "../test/testdata/userdict.utf8|../test/testdata/userdict.english");
  std::vector<std::string> words;
  std::string s1, s2;

  {
    segment.Cut("小明硕士毕业于中国科学院计算所，后在日本京都大学深造", words);
    s1 = turbo::StrJoin(words.begin(), words.end(), "/");
    s2 = "小明/硕士/毕业/于/中国/科学/学院/科学院/中国科学院/计算/计算所/，/后/"
         "在/日本/京都/大学/京都大学/深造";
    ASSERT_EQ(s1, s2);
  }

  {
    segment.Cut("小明硕士毕业于中国科学院计算所iPhone6", words);
    s1 = turbo::StrJoin(words.begin(), words.end(), "/");
    s2 = "小明/硕士/毕业/于/中国/科学/学院/科学院/中国科学院/计算/计算所/"
         "iPhone6";
    ASSERT_EQ(s1, s2);
  }

  {
    segment.Cut("中国科学院", words);
    s1 = turbo::StrJoin(words.begin(), words.end(), "/");
    s2 = "中国/科学/学院/科学院/中国科学院";
    ASSERT_EQ(s1, s2);
  }
}

TEST(MPSegmentTest, Unicode32) {
  std::string s("天气很好，🙋 我们去郊游。");
  std::vector<std::string> words;

  MPSegment segment("../dict/jieba.dict.utf8");
  ;
  segment.Cut(s, words);

  ASSERT_EQ(turbo::StrJoin(words.begin(), words.end(), "/"),
            "天气/很/好/，/🙋/ /我们/去/郊游/。");
}
