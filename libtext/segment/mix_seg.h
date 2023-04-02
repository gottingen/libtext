//
// Created by 李寅斌 on 2023/4/2.
//

#ifndef LIBTEXT_SEGMENT__MIX_SEG_H_
#define LIBTEXT_SEGMENT__MIX_SEG_H_

#include "libtext/segment/hmm_seg.h"
#include "libtext/segment/mps_seg.h"
#include "libtext/segment/post_tagger.h"
#include <cassert>

namespace libtext {

class MixSegment : public SegmentTagged {
public:
  MixSegment(const std::string &mpSegDict, const std::string &hmmSegDict,
             const std::string &userDict = "")
      : mpSeg_(mpSegDict, userDict), hmmSeg_(hmmSegDict) {}
  MixSegment(const DictTrie *dictTrie, const HMMModel *model)
      : mpSeg_(dictTrie), hmmSeg_(model) {}
  ~MixSegment() {}

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
    if (!hmm) {
      mpSeg_.Cut(begin, end, res);
      return;
    }
    std::vector<WordRange> words;
    assert(end >= begin);
    words.reserve(end - begin);
    mpSeg_.Cut(begin, end, words);

    std::vector<WordRange> hmmRes;
    hmmRes.reserve(end - begin);
    for (size_t i = 0; i < words.size(); i++) {
      // if mp Get a word, it's ok, put it into result
      if (words[i].left != words[i].right ||
          (words[i].left == words[i].right &&
           mpSeg_.IsUserDictSingleChineseWord(words[i].left->rune))) {
        res.push_back(words[i]);
        continue;
      }

      // if mp Get a single one and it is not in userdict, collect it in
      // sequence
      size_t j = i;
      while (j < words.size() && words[j].left == words[j].right &&
             !mpSeg_.IsUserDictSingleChineseWord(words[j].left->rune)) {
        j++;
      }

      // Cut the sequence with hmm
      assert(j - 1 >= i);
      // TODO
      hmmSeg_.Cut(words[i].left, words[j - 1].left + 1, hmmRes);
      // put hmm result to result
      for (size_t k = 0; k < hmmRes.size(); k++) {
        res.push_back(hmmRes[k]);
      }

      // clear tmp vars
      hmmRes.clear();

      // let i jump over this piece
      i = j - 1;
    }
  }

  const DictTrie *GetDictTrie() const { return mpSeg_.GetDictTrie(); }

  bool Tag(const std::string &src,
           std::vector<std::pair<std::string, std::string>> &res) const {
    return tagger_.Tag(src, res, *this);
  }

  std::string LookupTag(const std::string &str) const {
    return tagger_.LookupTag(str, *this);
  }

private:
  MPSegment mpSeg_;
  HMMSegment hmmSeg_;
  PosTagger tagger_;

}; // class MixSegment

} // namespace libtext

#endif // LIBTEXT_SEGMENT__MIX_SEG_H_
