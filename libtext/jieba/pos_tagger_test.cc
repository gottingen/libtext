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

#include "libtext/jieba/mix_seg.h"
#include <turbo/strings/str_join.h>
#include "gtest/gtest.h"

using namespace libtext;

static const char * const QUERY_TEST1 = "我是蓝翔技工拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上总经理，出任CEO，迎娶白富美，走上人生巅峰。";
static const char * const ANS_TEST1 = "我:r, 是:v, 蓝翔:x, 技工:n, 拖拉机:n, 学院:n, 手扶拖拉机:n, 专业:n, 的:uj, 。:x, 不用:v, 多久:m, ，:x, 我:r, 就:d, 会:v, 升职:v, 加薪:nr, ，:x, 当上:t, 总经理:n, ，:x, 出任:v, CEO:eng, ，:x, 迎娶:v, 白富:x, 美:ns, ，:x, 走上:v, 人生:n, 巅峰:n, 。:x";
static const char * const QUERY_TEST2 = "我是蓝翔技工拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上总经理，出任CEO，迎娶白富美，走上人生巅峰。";
static const char * const ANS_TEST2 = "我:r, 是:v, 蓝翔:nz, 技工:n, 拖拉机:n, 学院:n, 手扶拖拉机:n, 专业:n, 的:uj, 。:x, 不用:v, 多久:m, ，:x, 我:r, 就:d, 会:v, 升职:v, 加薪:nr, ，:x, 当上:t, 总经理:n, ，:x, 出任:v, CEO:eng, ，:x, 迎娶:v, 白富:x, 美:ns, ，:x, 走上:v, 人生:n, 巅峰:n, 。:x";

static const char * const QUERY_TEST3 = "iPhone6手机的最大特点是很容易弯曲。";
static const char * const ANS_TEST3 = "iPhone6:eng, 手机:n, 的:uj, 最大:a, 特点:n, 是:v, 很:zg, 容易:a, 弯曲:v, 。:x";
//static const char * const ANS_TEST3 = "";

TEST(PosTaggerTest, Test) {
  MixSegment tagger("../dict/jieba.dict.utf8", "../dict/hmm_model.utf8");
  {
    std::vector<std::pair<std::string, std::string> > res;
    tagger.Tag(QUERY_TEST1, res);
    std::string s;
    s = turbo::StrJoin(res, ", ", turbo::PairFormatter(":"));
    ASSERT_TRUE(s == ANS_TEST1);
  }
}
TEST(PosTagger, TestUserDict) {
  MixSegment tagger("../dict/jieba.dict.utf8", "../dict/hmm_model.utf8", "../test/testdata/userdict.utf8");
  {
    std::vector<std::pair<std::string, std::string> > res;
    tagger.Tag(QUERY_TEST2, res);
    std::string s;
    s = turbo::StrJoin(res, ", ", turbo::PairFormatter(":"));
    ASSERT_EQ(s, ANS_TEST2);
  }
  {
    std::vector<std::pair<std::string, std::string> > res;
    tagger.Tag(QUERY_TEST3, res);
    std::string s;
    s = turbo::StrJoin(res, ", ", turbo::PairFormatter(":"));
    ASSERT_EQ(s, ANS_TEST3);
  }
}
