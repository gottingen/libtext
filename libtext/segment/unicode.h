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
#ifndef LIBTEXT_SEGMENT_UNICODE_H_
#define LIBTEXT_SEGMENT_UNICODE_H_

#include <ostream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <turbo/container/inlined_vector.h>
#include <turbo/strings/string_view.h>
#include <turbo/strings/str_join.h>
#include <vector>

namespace libtext {

typedef uint32_t Rune;

struct Word {
  std::string word;
  uint32_t offset;
  uint32_t unicode_offset;
  uint32_t unicode_length;
  Word(const std::string &w, uint32_t o) : word(w), offset(o) {}
  Word(const std::string &w, uint32_t o, uint32_t unicode_offset,
       uint32_t unicode_length)
      : word(w), offset(o), unicode_offset(unicode_offset),
        unicode_length(unicode_length) {}
}; // struct Word

inline std::ostream &operator<<(std::ostream &os, const Word &w) {
  return os << "{\"word\": \"" << w.word << "\", \"offset\": " << w.offset
            << "}";
}

struct WordFormatter {
  void operator()(std::string *s, const Word &w) {
    turbo::StrAppend(s, "{word: ", w.word, ", offset: ", w.offset, "}");
  }
};
struct RuneStr {
  Rune rune;
  uint32_t offset;
  uint32_t len;
  uint32_t unicode_offset;
  uint32_t unicode_length;
  RuneStr()
      : rune(0), offset(0), len(0), unicode_offset(0), unicode_length(0) {}
  RuneStr(Rune r, uint32_t o, uint32_t l)
      : rune(r), offset(o), len(l), unicode_offset(0), unicode_length(0) {}
  RuneStr(Rune r, uint32_t o, uint32_t l, uint32_t unicode_offset,
          uint32_t unicode_length)
      : rune(r), offset(o), len(l), unicode_offset(unicode_offset),
        unicode_length(unicode_length) {}
}; // struct RuneStr

inline std::ostream &operator<<(std::ostream &os, const RuneStr &r) {
  return os << "{\"rune\": \"" << r.rune << "\", \"offset\": " << r.offset
            << ", \"len\": " << r.len << "}";
}

typedef turbo::InlinedVector<Rune, 8> Unicode;
typedef turbo::InlinedVector<struct RuneStr, 8> RuneStrArray;

// [left, right]
struct WordRange {
  RuneStrArray::const_iterator left;
  RuneStrArray::const_iterator right;
  WordRange(RuneStrArray::const_iterator l, RuneStrArray::const_iterator r)
      : left(l), right(r) {}
  size_t Length() const { return right - left + 1; }
  bool IsAllAscii() const {
    for (RuneStrArray::const_iterator iter = left; iter <= right; ++iter) {
      if (iter->rune >= 0x80) {
        return false;
      }
    }
    return true;
  }
}; // struct WordRange

struct RuneStrLite {
  uint32_t rune;
  uint32_t len;
  RuneStrLite() : rune(0), len(0) {}
  RuneStrLite(uint32_t r, uint32_t l) : rune(r), len(l) {}
}; // struct RuneStrLite

inline RuneStrLite DecodeRuneInString(const char *str, size_t len) {
  RuneStrLite rp(0, 0);
  if (str == NULL || len == 0) {
    return rp;
  }
  if (!(str[0] & 0x80)) { // 0xxxxxxx
    // 7bit, total 7bit
    rp.rune = (uint8_t)(str[0]) & 0x7f;
    rp.len = 1;
  } else if ((uint8_t)str[0] <= 0xdf && 1 < len) {
    // 110xxxxxx
    // 5bit, total 5bit
    rp.rune = (uint8_t)(str[0]) & 0x1f;

    // 6bit, total 11bit
    rp.rune <<= 6;
    rp.rune |= (uint8_t)(str[1]) & 0x3f;
    rp.len = 2;
  } else if ((uint8_t)str[0] <= 0xef && 2 < len) { // 1110xxxxxx
    // 4bit, total 4bit
    rp.rune = (uint8_t)(str[0]) & 0x0f;

    // 6bit, total 10bit
    rp.rune <<= 6;
    rp.rune |= (uint8_t)(str[1]) & 0x3f;

    // 6bit, total 16bit
    rp.rune <<= 6;
    rp.rune |= (uint8_t)(str[2]) & 0x3f;

    rp.len = 3;
  } else if ((uint8_t)str[0] <= 0xf7 && 3 < len) { // 11110xxxx
    // 3bit, total 3bit
    rp.rune = (uint8_t)(str[0]) & 0x07;

    // 6bit, total 9bit
    rp.rune <<= 6;
    rp.rune |= (uint8_t)(str[1]) & 0x3f;

    // 6bit, total 15bit
    rp.rune <<= 6;
    rp.rune |= (uint8_t)(str[2]) & 0x3f;

    // 6bit, total 21bit
    rp.rune <<= 6;
    rp.rune |= (uint8_t)(str[3]) & 0x3f;

    rp.len = 4;
  } else {
    rp.rune = 0;
    rp.len = 0;
  }
  return rp;
}

inline bool DecodeRunesInString(const char *s, size_t len,
                                RuneStrArray &runes) {
  runes.clear();
  runes.reserve(len / 2);
  for (uint32_t i = 0, j = 0; i < len;) {
    RuneStrLite rp = DecodeRuneInString(s + i, len - i);
    if (rp.len == 0) {
      runes.clear();
      return false;
    }
    RuneStr x(rp.rune, i, rp.len, j, 1);
    runes.push_back(x);
    i += rp.len;
    ++j;
  }
  return true;
}

inline bool DecodeRunesInString(const std::string &s, RuneStrArray &runes) {
  return DecodeRunesInString(s.c_str(), s.size(), runes);
}

inline bool DecodeRunesInString(const char *s, size_t len, Unicode &unicode) {
  unicode.clear();
  RuneStrArray runes;
  if (!DecodeRunesInString(s, len, runes)) {
    return false;
  }
  unicode.reserve(runes.size());
  for (size_t i = 0; i < runes.size(); i++) {
    unicode.push_back(runes[i].rune);
  }
  return true;
}

inline bool IsSingleWord(const std::string &str) {
  RuneStrLite rp = DecodeRuneInString(str.c_str(), str.size());
  return rp.len == str.size();
}

inline bool DecodeRunesInString(const std::string &s, Unicode &unicode) {
  return DecodeRunesInString(s.c_str(), s.size(), unicode);
}

inline Unicode DecodeRunesInString(const std::string &s) {
  Unicode result;
  DecodeRunesInString(s, result);
  return result;
}

// [left, right]
inline Word GetWordFromRunes(const std::string &s,
                             RuneStrArray::const_iterator left,
                             RuneStrArray::const_iterator right) {
  assert(right->offset >= left->offset);
  uint32_t len = right->offset - left->offset + right->len;
  uint32_t unicode_length =
      right->unicode_offset - left->unicode_offset + right->unicode_length;
  return Word(s.substr(left->offset, len), left->offset, left->unicode_offset,
              unicode_length);
}

inline std::string GetStringFromRunes(const std::string &s,
                                      RuneStrArray::const_iterator left,
                                      RuneStrArray::const_iterator right) {
  assert(right->offset >= left->offset);
  uint32_t len = right->offset - left->offset + right->len;
  return s.substr(left->offset, len);
}

inline void GetWordsFromWordRanges(const std::string &s,
                                   const std::vector<WordRange> &wrs,
                                   std::vector<Word> &words) {
  for (size_t i = 0; i < wrs.size(); i++) {
    words.push_back(GetWordFromRunes(s, wrs[i].left, wrs[i].right));
  }
}

inline std::vector<Word>
GetWordsFromWordRanges(const std::string &s,
                       const std::vector<WordRange> &wrs) {
  std::vector<Word> result;
  GetWordsFromWordRanges(s, wrs, result);
  return result;
}

inline void GetStringsFromWords(const std::vector<Word> &words,
                                std::vector<std::string> &strs) {
  strs.resize(words.size());
  for (size_t i = 0; i < words.size(); ++i) {
    strs[i] = words[i].word;
  }
}

} // namespace libtext

#endif // LIBTEXT_SEGMENT_UNICODE_H_
