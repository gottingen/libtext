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

#ifndef LIBTEXT_SEGMENT_MPS_SEG_H_
#define LIBTEXT_SEGMENT_MPS_SEG_H_

#include "libtext/segment/dict_trie.h"
#include "libtext/segment/post_tagger.h"
#include "libtext/segment/pre_filter.h"
#include "libtext/segment/seg_tagged.h"
#include <algorithm>
#include <cassert>
#include <set>

namespace libtext {

class MPSegment : public SegmentTagged {
public:
  MPSegment(const std::string &dictPath, const std::string &userDictPath = "")
      : dictTrie_(new DictTrie(dictPath, userDictPath)), isNeedDestroy_(true) {}
  MPSegment(const DictTrie *dictTrie)
      : dictTrie_(dictTrie), isNeedDestroy_(false) {
    assert(dictTrie_);
  }
  ~MPSegment() {
    if (isNeedDestroy_) {
      delete dictTrie_;
    }
  }

  void Cut(const std::string &sentence, std::vector<std::string> &words) const {
    Cut(sentence, words, MAX_WORD_LENGTH);
  }

  void Cut(const std::string &sentence, std::vector<std::string> &words,
           size_t max_word_len) const {
    std::vector<Word> tmp;
    Cut(sentence, tmp, max_word_len);
    GetStringsFromWords(tmp, words);
  }
  void Cut(const std::string &sentence, std::vector<Word> &words,
           size_t max_word_len = MAX_WORD_LENGTH) const {
    PreFilter pre_filter(symbols_, sentence);
    PreFilter::Range range;
    std::vector<WordRange> wrs;
    wrs.reserve(sentence.size() / 2);
    while (pre_filter.HasNext()) {
      range = pre_filter.Next();
      Cut(range.begin, range.end, wrs, max_word_len);
    }
    words.clear();
    words.reserve(wrs.size());
    GetWordsFromWordRanges(sentence, wrs, words);
  }
  void Cut(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end,
           std::vector<WordRange> &words,
           size_t max_word_len = MAX_WORD_LENGTH) const {
    std::vector<Dag> dags;
    dictTrie_->Find(begin, end, dags, max_word_len);
    CalcDP(dags);
    CutByDag(begin, end, dags, words);
  }

  const DictTrie *GetDictTrie() const { return dictTrie_; }

  bool Tag(const std::string &src,
           std::vector<std::pair<std::string, std::string>> &res) const {
    return tagger_.Tag(src, res, *this);
  }

  bool IsUserDictSingleChineseWord(const Rune &value) const {
    return dictTrie_->IsUserDictSingleChineseWord(value);
  }

private:
  void CalcDP(std::vector<Dag> &dags) const {
    size_t nextPos;
    const DictUnit *p;
    double val;

    for (std::vector<Dag>::reverse_iterator rit = dags.rbegin();
         rit != dags.rend(); rit++) {
      rit->pInfo = NULL;
      rit->weight = MIN_DOUBLE;
      assert(!rit->nexts.empty());
      for (turbo::InlinedVector<std::pair<size_t, const DictUnit *>,
                                8>::const_iterator it = rit->nexts.begin();
           it != rit->nexts.end(); it++) {
        nextPos = it->first;
        p = it->second;
        val = 0.0;
        if (nextPos + 1 < dags.size()) {
          val += dags[nextPos + 1].weight;
        }

        if (p) {
          val += p->weight;
        } else {
          val += dictTrie_->GetMinWeight();
        }
        if (val > rit->weight) {
          rit->pInfo = p;
          rit->weight = val;
        }
      }
    }
  }
  void CutByDag(RuneStrArray::const_iterator begin,
                RuneStrArray::const_iterator end, const std::vector<Dag> &dags,
                std::vector<WordRange> &words) const {
    size_t i = 0;
    while (i < dags.size()) {
      const DictUnit *p = dags[i].pInfo;
      if (p) {
        assert(p->word.size() >= 1);
        WordRange wr(begin + i, begin + i + p->word.size() - 1);
        words.push_back(wr);
        i += p->word.size();
      } else { // single chinese word
        WordRange wr(begin + i, begin + i);
        words.push_back(wr);
        i++;
      }
    }
  }

  const DictTrie *dictTrie_;
  bool isNeedDestroy_;
  PosTagger tagger_;

}; // class MPSegment

} // namespace libtext

#endif // LIBTEXT_SEGMENT_MPS_SEG_H_
