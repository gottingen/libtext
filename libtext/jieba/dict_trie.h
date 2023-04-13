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

#ifndef LIBTEXT_SEGMENT_DICT_TRIE_H_
#define LIBTEXT_SEGMENT_DICT_TRIE_H_

#include "libtext/jieba/trie.h"
#include "libtext/jieba/unicode.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <stdint.h>
#include <string>
#include <turbo/log/logging.h>
#include <turbo/strings/str_split.h>

namespace libtext {

const double MIN_DOUBLE = -3.14e+100;
const double MAX_DOUBLE = 3.14e+100;
const size_t DICT_COLUMN_NUM = 3;
const char *const UNKNOWN_TAG = "";

class DictTrie {
public:
  enum UserWordWeightOption {
    WordWeightMin,
    WordWeightMedian,
    WordWeightMax,
  }; // enum UserWordWeightOption

  DictTrie(const std::string &dict_path,
           const std::string &user_dict_paths = "",
           UserWordWeightOption user_word_weight_opt = WordWeightMedian) {
    Init(dict_path, user_dict_paths, user_word_weight_opt);
  }

  ~DictTrie() { delete trie_; }

  bool InsertUserWord(const std::string &word,
                      const std::string &tag = UNKNOWN_TAG) {
    DictUnit node_info;
    if (!MakeNodeInfo(node_info, word, user_word_default_weight_, tag)) {
      return false;
    }
    active_node_infos_.push_back(node_info);
    trie_->InsertNode(node_info.word, &active_node_infos_.back());
    return true;
  }

  bool InsertUserWord(const std::string &word, int freq,
                      const std::string &tag = UNKNOWN_TAG) {
    DictUnit node_info;
    double weight =
        freq ? log(1.0 * freq / freq_sum_) : user_word_default_weight_;
    if (!MakeNodeInfo(node_info, word, weight, tag)) {
      return false;
    }
    active_node_infos_.push_back(node_info);
    trie_->InsertNode(node_info.word, &active_node_infos_.back());
    return true;
  }

  bool DeleteUserWord(const std::string &word,
                      const std::string &tag = UNKNOWN_TAG) {
    DictUnit node_info;
    if (!MakeNodeInfo(node_info, word, user_word_default_weight_, tag)) {
      return false;
    }
    trie_->DeleteNode(node_info.word, &node_info);
    return true;
  }

  const DictUnit *Find(RuneStrArray::const_iterator begin,
                       RuneStrArray::const_iterator end) const {
    return trie_->Find(begin, end);
  }

  void Find(RuneStrArray::const_iterator begin,
            RuneStrArray::const_iterator end, std::vector<struct Dag> &res,
            size_t max_word_len = MAX_WORD_LENGTH) const {
    trie_->Find(begin, end, res, max_word_len);
  }

  bool Find(const std::string &word) {
    const DictUnit *tmp = NULL;
    RuneStrArray runes;
    if (!DecodeRunesInString(word, runes)) {
      TURBO_LOG(ERROR) << "Decode failed.";
    }
    tmp = Find(runes.begin(), runes.end());
    if (tmp == NULL) {
      return false;
    } else {
      return true;
    }
  }

  bool IsUserDictSingleChineseWord(const Rune &word) const {
    return user_dict_single_chinese_word_.find(word) !=
           user_dict_single_chinese_word_.end();
  }

  double GetMinWeight() const { return min_weight_; }

  void InserUserDictNode(const std::string &line) {
    std::vector<std::string> buf;
    DictUnit node_info;
    buf = turbo::StrSplit(line, " ");
    if (buf.size() == 1) {
      MakeNodeInfo(node_info, buf[0], user_word_default_weight_, UNKNOWN_TAG);
    } else if (buf.size() == 2) {
      MakeNodeInfo(node_info, buf[0], user_word_default_weight_, buf[1]);
    } else if (buf.size() == 3) {
      int freq = atoi(buf[1].c_str());
      assert(freq_sum_ > 0.0);
      double weight = log(1.0 * freq / freq_sum_);
      MakeNodeInfo(node_info, buf[0], weight, buf[2]);
    }
    static_node_infos_.push_back(node_info);
    if (node_info.word.size() == 1) {
      user_dict_single_chinese_word_.insert(node_info.word[0]);
    }
  }

  void LoadUserDict(const std::vector<std::string> &buf) {
    for (size_t i = 0; i < buf.size(); i++) {
      InserUserDictNode(buf[i]);
    }
  }

  void LoadUserDict(const std::set<std::string> &buf) {
    std::set<std::string>::const_iterator iter;
    for (iter = buf.begin(); iter != buf.end(); iter++) {
      InserUserDictNode(*iter);
    }
  }

  void LoadUserDict(const std::string &filePaths) {
    std::vector<std::string> files =
        turbo::StrSplit(filePaths, turbo::ByAnyChar("|;"));
    size_t lineno = 0;
    for (size_t i = 0; i < files.size(); i++) {
      std::ifstream ifs(files[i].c_str());
      TURBO_CHECK(ifs.is_open()) << "open " << files[i] << " failed";
      std::string line;

      for (; getline(ifs, line); lineno++) {
        if (line.size() == 0) {
          continue;
        }
        InserUserDictNode(line);
      }
    }
  }

private:
  void Init(const std::string &dict_path, const std::string &user_dict_paths,
            UserWordWeightOption user_word_weight_opt) {
    LoadDict(dict_path);
    freq_sum_ = CalcFreqSum(static_node_infos_);
    CalculateWeight(static_node_infos_, freq_sum_);
    SetStaticWordWeights(user_word_weight_opt);

    if (user_dict_paths.size()) {
      LoadUserDict(user_dict_paths);
    }
    Shrink(static_node_infos_);
    CreateTrie(static_node_infos_);
  }

  void CreateTrie(const std::vector<DictUnit> &dictUnits) {
    assert(dictUnits.size());
    std::vector<Unicode> words;
    std::vector<const DictUnit *> valuePointers;
    for (size_t i = 0; i < dictUnits.size(); i++) {
      words.push_back(dictUnits[i].word);
      valuePointers.push_back(&dictUnits[i]);
    }

    trie_ = new Trie(words, valuePointers);
  }

  bool MakeNodeInfo(DictUnit &node_info, const std::string &word, double weight,
                    const std::string &tag) {
    if (!DecodeRunesInString(word, node_info.word)) {
      TURBO_LOG(ERROR) << "Decode " << word << " failed.";
      return false;
    }
    node_info.weight = weight;
    node_info.tag = tag;
    return true;
  }

  void LoadDict(const std::string &filePath) {
    std::ifstream ifs(filePath.c_str());
    TURBO_CHECK(ifs.is_open()) << "open " << filePath << " failed.";
    std::string line;
    std::vector<std::string> buf;

    DictUnit node_info;
    for (size_t lineno = 0; getline(ifs, line); lineno++) {
      buf = turbo::StrSplit(line, " ");
      TURBO_CHECK(buf.size() == DICT_COLUMN_NUM)
          << "split result illegal, line:" << line;
      MakeNodeInfo(node_info, buf[0], atof(buf[1].c_str()), buf[2]);
      static_node_infos_.push_back(node_info);
    }
  }

  static bool WeightCompare(const DictUnit &lhs, const DictUnit &rhs) {
    return lhs.weight < rhs.weight;
  }

  void SetStaticWordWeights(UserWordWeightOption option) {
    TURBO_CHECK(!static_node_infos_.empty());
    std::vector<DictUnit> x = static_node_infos_;
    sort(x.begin(), x.end(), WeightCompare);
    min_weight_ = x[0].weight;
    max_weight_ = x[x.size() - 1].weight;
    median_weight_ = x[x.size() / 2].weight;
    switch (option) {
    case WordWeightMin:
      user_word_default_weight_ = min_weight_;
      break;
    case WordWeightMedian:
      user_word_default_weight_ = median_weight_;
      break;
    default:
      user_word_default_weight_ = max_weight_;
      break;
    }
  }

  double CalcFreqSum(const std::vector<DictUnit> &node_infos) const {
    double sum = 0.0;
    for (size_t i = 0; i < node_infos.size(); i++) {
      sum += node_infos[i].weight;
    }
    return sum;
  }

  void CalculateWeight(std::vector<DictUnit> &node_infos, double sum) const {
    assert(sum > 0.0);
    for (size_t i = 0; i < node_infos.size(); i++) {
      DictUnit &node_info = node_infos[i];
      assert(node_info.weight > 0.0);
      node_info.weight = log(double(node_info.weight) / sum);
    }
  }

  void Shrink(std::vector<DictUnit> &units) const {
    std::vector<DictUnit>(units.begin(), units.end()).swap(units);
  }

  std::vector<DictUnit> static_node_infos_;
  std::deque<DictUnit> active_node_infos_; // must not be vector
  Trie *trie_;

  double freq_sum_;
  double min_weight_;
  double max_weight_;
  double median_weight_;
  double user_word_default_weight_;
  std::unordered_set<Rune> user_dict_single_chinese_word_;
};
} // namespace libtext

#endif // LIBTEXT_SEGMENT_DICT_TRIE_H_
