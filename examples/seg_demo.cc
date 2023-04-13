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

#include <libtext/jieba/seg.h>
#include <turbo/strings/str_join.h>

using namespace std;

const char* const DICT_PATH = "../dict/jieba.dict.utf8";
const char* const HMM_PATH = "../dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../dict/user.dict.utf8";
const char* const IDF_PATH = "../dict/idf.utf8";
const char* const STOP_WORD_PATH = "../dict/stop_words.utf8";

int main(int argc, char** argv) {
  libtext::Segmentor segmentor(DICT_PATH,
                        HMM_PATH,
                        USER_DICT_PATH,
                        IDF_PATH,
                        STOP_WORD_PATH);
  vector<string> words;
  vector<libtext::Word> jiebawords;
  string s;
  string result;

  s = "他来到了网易杭研大厦";
  cout << s << endl;
  cout << "[demo] Cut With HMM" << endl;
  segmentor.Cut(s, words, true);
  cout << turbo::StrJoin(words.begin(), words.end(), "/") << endl;

  cout << "[demo] Cut Without HMM " << endl;
  segmentor.Cut(s, words, false);
  cout << turbo::StrJoin(words.begin(), words.end(), "/") << endl;

  s = "我来到北京清华大学";
  cout << s << endl;
  cout << "[demo] CutAll" << endl;
  segmentor.CutAll(s, words);
  cout << turbo::StrJoin(words.begin(), words.end(), "/") << endl;

  s = "小明硕士毕业于中国科学院计算所，后在日本京都大学深造";
  cout << s << endl;
  cout << "[demo] CutForSearch" << endl;
  segmentor.CutForSearch(s, words);
  cout << turbo::StrJoin(words.begin(), words.end(), "/") << endl;

  cout << "[demo] Insert User Word" << endl;
  segmentor.Cut("男默女泪", words);
  cout << turbo::StrJoin(words.begin(), words.end(), "/") << endl;
  segmentor.InsertUserWord("男默女泪");
  segmentor.Cut("男默女泪", words);
  cout << turbo::StrJoin(words.begin(), words.end(), "/") << endl;

  cout << "[demo] CutForSearch Word With Offset" << endl;
  segmentor.CutForSearch(s, jiebawords, true);
  cout << turbo::StrJoin(jiebawords,", ", libtext::WordFormatter()) << endl;

  cout << "[demo] Lookup Tag for Single Token" << endl;
  const int DemoTokenMaxLen = 32;
  char DemoTokens[][DemoTokenMaxLen] = {"拖拉机", "CEO", "123", "。"};
  vector<pair<string, string> > LookupTagres(sizeof(DemoTokens) / DemoTokenMaxLen);
  vector<pair<string, string> >::iterator it;
  for (it = LookupTagres.begin(); it != LookupTagres.end(); it++) {
    it->first = DemoTokens[it - LookupTagres.begin()];
    it->second = segmentor.LookupTag(it->first);
  }
  cout << turbo::StrJoin(LookupTagres, ",", turbo::PairFormatter(":")) << endl;

  cout << "[demo] Tagging" << endl;
  vector<pair<string, string> > tagres;
  s = "我是拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上CEO，走上人生巅峰。";
  segmentor.Tag(s, tagres);
  cout << s << endl;
  cout << turbo::StrJoin(tagres, ",", turbo::PairFormatter(":")) << endl;

  cout << "[demo] Keyword Extraction" << endl;
  const size_t topk = 5;
  vector<libtext::KeywordExtractor::Word> keywordres;
  segmentor.extractor.Extract(s, keywordres, topk);
  cout << s << endl;
  cout << turbo::StrJoin(keywordres, ",", libtext::KeywordExtractorWordFormatter()) << endl;
  return EXIT_SUCCESS;
}

