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

#ifndef LIBTEXT_SEGMENT_PRE_FILTER_H_
#define LIBTEXT_SEGMENT_PRE_FILTER_H_

#include "libtext/segment/trie.h"
#include <turbo/log/logging.h>
#include <turbo/container/flat_hash_set.h>

namespace libtext {

class PreFilter {
public:
  // TODO use WordRange instead of Range
  struct Range {
    RuneStrArray::const_iterator begin;
    RuneStrArray::const_iterator end;
  }; // struct Range

  PreFilter(const turbo::flat_hash_set<Rune> &symbols,
            const std::string &sentence)
      : symbols_(symbols) {
    if (!DecodeRunesInString(sentence, sentence_)) {
      TURBO_LOG(ERROR) << "decode failed. ";
    }
    cursor_ = sentence_.begin();
  }
  ~PreFilter() {}
  bool HasNext() const { return cursor_ != sentence_.end(); }
  Range Next() {
    Range range;
    range.begin = cursor_;
    while (cursor_ != sentence_.end()) {
      if (symbols_.find(cursor_->rune) != symbols_.end()) {
        if (range.begin == cursor_) {
          cursor_++;
        }
        range.end = cursor_;
        return range;
      }
      cursor_++;
    }
    range.end = sentence_.end();
    return range;
  }

private:
  RuneStrArray::const_iterator cursor_;
  RuneStrArray sentence_;
  const turbo::flat_hash_set<Rune> &symbols_;
}; // class PreFilter

} // namespace libtext

#endif // LIBTEXT_SEGMENT_PRE_FILTER_H_
