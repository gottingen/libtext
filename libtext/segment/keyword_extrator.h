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

#ifndef LIBTEXT_SEGMENT_KEYWORD_EXTRATOR_H_
#define LIBTEXT_SEGMENT_KEYWORD_EXTRATOR_H_

#include "libtext/segment/mix_seg.h"
#include "turbo/strings/str_join.h"
#include <cmath>
#include <set>

namespace libtext {

/*utf8*/
class KeywordExtractor {
public:
  struct Word {
    std::string word;
    std::vector<size_t> offsets;
    double weight;
  }; // struct Word

  KeywordExtractor(const std::string &dictPath, const std::string &hmmFilePath,
                   const std::string &idfPath, const std::string &stopWordPath,
                   const std::string &userDict = "")
      : segment_(dictPath, hmmFilePath, userDict) {
    LoadIdfDict(idfPath);
    LoadStopWordDict(stopWordPath);
  }
  KeywordExtractor(const DictTrie *dictTrie, const HMMModel *model,
                   const std::string &idfPath, const std::string &stopWordPath)
      : segment_(dictTrie, model) {
    LoadIdfDict(idfPath);
    LoadStopWordDict(stopWordPath);
  }
  ~KeywordExtractor() {}

  void Extract(const std::string &sentence, std::vector<std::string> &keywords,
               size_t topN) const {
    std::vector<Word> topWords;
    Extract(sentence, topWords, topN);
    for (size_t i = 0; i < topWords.size(); i++) {
      keywords.push_back(topWords[i].word);
    }
  }

  void Extract(const std::string &sentence,
               std::vector<std::pair<std::string, double>> &keywords,
               size_t topN) const {
    std::vector<Word> topWords;
    Extract(sentence, topWords, topN);
    for (size_t i = 0; i < topWords.size(); i++) {
      keywords.push_back(
          std::pair<std::string, double>(topWords[i].word, topWords[i].weight));
    }
  }

  void Extract(const std::string &sentence, std::vector<Word> &keywords,
               size_t topN) const {
    std::vector<std::string> words;
    segment_.Cut(sentence, words);

    std::map<std::string, Word> wordmap;
    size_t offset = 0;
    for (size_t i = 0; i < words.size(); ++i) {
      size_t t = offset;
      offset += words[i].size();
      if (IsSingleWord(words[i]) ||
          stopWords_.find(words[i]) != stopWords_.end()) {
        continue;
      }
      wordmap[words[i]].offsets.push_back(t);
      wordmap[words[i]].weight += 1.0;
    }
    if (offset != sentence.size()) {
      TURBO_LOG(ERROR) << "words illegal";
      return;
    }

    keywords.clear();
    keywords.reserve(wordmap.size());
    for (std::map<std::string, Word>::iterator itr = wordmap.begin();
         itr != wordmap.end(); ++itr) {
      std::unordered_map<std::string, double>::const_iterator cit =
          idfMap_.find(itr->first);
      if (cit != idfMap_.end()) {
        itr->second.weight *= cit->second;
      } else {
        itr->second.weight *= idfAverage_;
      }
      itr->second.word = itr->first;
      keywords.push_back(itr->second);
    }
    topN = std::min(topN, keywords.size());
    partial_sort(keywords.begin(), keywords.begin() + topN, keywords.end(),
                 Compare);
    keywords.resize(topN);
  }

private:
  void LoadIdfDict(const std::string &idfPath) {
    std::ifstream ifs(idfPath.c_str());
    TURBO_CHECK(ifs.is_open()) << "open " << idfPath << " failed";
    std::string line;
    std::vector<std::string> buf;
    double idf = 0.0;
    double idfSum = 0.0;
    size_t lineno = 0;
    for (; getline(ifs, line); lineno++) {
      buf.clear();
      if (line.empty()) {
        TURBO_LOG(ERROR) << "lineno: " << lineno << " empty. skipped.";
        continue;
      }
      buf = turbo::StrSplit(line, " ");
      if (buf.size() != 2) {
        TURBO_LOG(ERROR) << "line: " << line << ", lineno: " << lineno
                         << " empty. skipped.";
        continue;
      }
      idf = atof(buf[1].c_str());
      idfMap_[buf[0]] = idf;
      idfSum += idf;
    }

    assert(lineno);
    idfAverage_ = idfSum / lineno;
    assert(idfAverage_ > 0.0);
  }
  void LoadStopWordDict(const std::string &filePath) {
    std::ifstream ifs(filePath.c_str());
    TURBO_CHECK(ifs.is_open()) << "open " << filePath << " failed";
    std::string line;
    while (getline(ifs, line)) {
      stopWords_.insert(line);
    }
    assert(stopWords_.size());
  }

  static bool Compare(const Word &lhs, const Word &rhs) {
    return lhs.weight > rhs.weight;
  }

  MixSegment segment_;
  std::unordered_map<std::string, double> idfMap_;
  double idfAverage_;

  std::unordered_set<std::string> stopWords_;
}; // class KeywordExtractor

inline std::ostream &operator<<(std::ostream &os,
                                const KeywordExtractor::Word &word) {
  return os << "{\"word\": \"" << word.word << "\", \"offset\": {"
            << turbo::StrJoin(word.offsets, ",")
            << "}, \"weight\": " << word.weight << "}";
}

struct KeywordExtractorWordFormatter {
  void operator()(std::string* s, const KeywordExtractor::Word &word) {
    turbo::StrAppend(s, "{\"word\": \"", word.word,
                     "\", \"offset\": [", turbo::StrJoin(word.offsets, ", "),
                     "], \"weight\": ", word.weight, "}");
  }
};


} // namespace libtext

#endif // LIBTEXT_SEGMENT_KEYWORD_EXTRATOR_H_
