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

#ifndef LIBTEXT_SEGMENT_QUERY_SEG_H_
#define LIBTEXT_SEGMENT_QUERY_SEG_H_

#include "libtext/segment/dict_trie.h"
#include "libtext/segment/full_seg.h"
#include "libtext/segment/mix_seg.h"
#include "libtext/segment/seg_base.h"
#include "libtext/segment/unicode.h"
#include <algorithm>
#include <cassert>
#include <set>

namespace libtext {

class QuerySegment : public SegmentBase {
public:
  QuerySegment(const std::string &dict, const std::string &model,
               const std::string &userDict = "")
      : mixSeg_(dict, model, userDict), trie_(mixSeg_.GetDictTrie()) {}
  QuerySegment(const DictTrie *dictTrie, const HMMModel *model)
      : mixSeg_(dictTrie, model), trie_(dictTrie) {}
  ~QuerySegment() {}

  void Cut(const std::string &sentence, std::vector<std::string> &words) const {
    Cut(sentence, words, true);
  }
  void Cut(const std::string &sentence, std::vector<std::string> &words,
           bool hmm) const {
    std::vector<Word> tmp;
    Cut(sentence, tmp, hmm);
    GetStringsFromWords(tmp, words);
  }
  void Cut(const std::string &sentence, std::vector<Word> &words,
           bool hmm = true) const {
    PreFilter pre_filter(symbols_, sentence);
    PreFilter::Range range;
    std::vector<WordRange> wrs;
    wrs.reserve(sentence.size() / 2);
    while (pre_filter.HasNext()) {
      range = pre_filter.Next();
      Cut(range.begin, range.end, wrs, hmm);
    }
    words.clear();
    words.reserve(wrs.size());
    GetWordsFromWordRanges(sentence, wrs, words);
  }
  void Cut(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end,
           std::vector<WordRange> &res, bool hmm) const {
    // use mix Cut first
    std::vector<WordRange> mixRes;
    mixSeg_.Cut(begin, end, mixRes, hmm);

    std::vector<WordRange> fullRes;
    for (std::vector<WordRange>::const_iterator mixResItr = mixRes.begin();
         mixResItr != mixRes.end(); mixResItr++) {
      if (mixResItr->Length() > 2) {
        for (size_t i = 0; i + 1 < mixResItr->Length(); i++) {
          WordRange wr(mixResItr->left + i, mixResItr->left + i + 1);
          if (trie_->Find(wr.left, wr.right + 1) != NULL) {
            res.push_back(wr);
          }
        }
      }
      if (mixResItr->Length() > 3) {
        for (size_t i = 0; i + 2 < mixResItr->Length(); i++) {
          WordRange wr(mixResItr->left + i, mixResItr->left + i + 2);
          if (trie_->Find(wr.left, wr.right + 1) != NULL) {
            res.push_back(wr);
          }
        }
      }
      res.push_back(*mixResItr);
    }
  }

private:
  bool IsAllAscii(const Unicode &s) const {
    for (size_t i = 0; i < s.size(); i++) {
      if (s[i] >= 0x80) {
        return false;
      }
    }
    return true;
  }
  MixSegment mixSeg_;
  const DictTrie *trie_;
}; // QuerySegment

} // namespace libtext

#endif // LIBTEXT_SEGMENT_QUERY_SEG_H_
