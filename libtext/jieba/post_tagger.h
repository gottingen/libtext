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

#ifndef LIBTEXT_SEGMENT_POST_TAGGER_H_
#define LIBTEXT_SEGMENT_POST_TAGGER_H_

#include "libtext/jieba/dict_trie.h"
#include "libtext/jieba/seg_tagged.h"

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
