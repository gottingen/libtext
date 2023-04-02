//
// Created by 李寅斌 on 2023/4/2.
//

#ifndef LIBTEXT_SEGMENT_POST_TAGGER_H_
#define LIBTEXT_SEGMENT_POST_TAGGER_H_

#include "libtext/segment/dict_trie.h"
#include "libtext/segment/seg_tagged.h"

namespace libtext {

static const char *const POS_M = "m";
static const char *const POS_ENG = "eng";
static const char *const POS_X = "x";

class PosTagger {
public:
  PosTagger() {}
  ~PosTagger() {}

  bool Tag(const std::string &src,
           std::vector<std::pair<std::string, std::string>> &res,
           const SegmentTagged &segment) const {
    std::vector<std::string> CutRes;
    segment.Cut(src, CutRes);

    for (std::vector<std::string>::iterator itr = CutRes.begin();
         itr != CutRes.end(); ++itr) {
      res.push_back(make_pair(*itr, LookupTag(*itr, segment)));
    }
    return !res.empty();
  }

  std::string LookupTag(const std::string &str,
                        const SegmentTagged &segment) const {
    const DictUnit *tmp = nullptr;
    RuneStrArray runes;
    const DictTrie *dict = segment.GetDictTrie();
    assert(dict != nullptr);
    if (!DecodeRunesInString(str, runes)) {
      TURBO_LOG(ERROR) << "Decode failed.";
      return POS_X;
    }
    tmp = dict->Find(runes.begin(), runes.end());
    if (tmp == nullptr || tmp->tag.empty()) {
      return SpecialRule(runes);
    } else {
      return tmp->tag;
    }
  }

private:
  const char *SpecialRule(const RuneStrArray &unicode) const {
    size_t m = 0;
    size_t eng = 0;
    for (size_t i = 0; i < unicode.size() && eng < unicode.size() / 2; i++) {
      if (unicode[i].rune < 0x80) {
        eng++;
        if ('0' <= unicode[i].rune && unicode[i].rune <= '9') {
          m++;
        }
      }
    }
    // ascii char is not found
    if (eng == 0) {
      return POS_X;
    }
    // all the ascii is number char
    if (m == eng) {
      return POS_M;
    }
    // the ascii chars contain english letter
    return POS_ENG;
  }

}; // class PosTagger

} // namespace libtext

#endif // LIBTEXT_SEGMENT_POST_TAGGER_H_
