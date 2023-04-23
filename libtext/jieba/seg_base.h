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

#ifndef LIBTEXT_SEGMENT_SEG_BASE_H_
#define LIBTEXT_SEGMENT_SEG_BASE_H_

#include "libtext/jieba/pre_filter.h"
#include "turbo/log/logging.h"
#include <cassert>

namespace libtext {

const char *const SPECIAL_SEPARATORS = " \t\n\xEF\xBC\x8C\xE3\x80\x82";

class SegmentBase {
public:
  SegmentBase() { TURBO_CHECK(ResetSeparators(SPECIAL_SEPARATORS)); }
  virtual ~SegmentBase() {}

  virtual void Cut(const std::string &sentence,
                   std::vector<std::string> &words) const = 0;

  bool ResetSeparators(const std::string &s) {
    symbols_.clear();
    RuneStrArray runes;
    if (!DecodeRunesInString(s, runes)) {
      TURBO_LOG(ERROR) << "decode " << s << " failed";
      return false;
    }
    for (size_t i = 0; i < runes.size(); i++) {
      if (!symbols_.insert(runes[i].rune).second) {
        TURBO_LOG(ERROR) << s.substr(runes[i].offset, runes[i].len)
                         << " already exists";
        return false;
      }
    }
    return true;
  }

protected:
  turbo::flat_hash_set<Rune> symbols_;
}; // class SegmentBase

} // namespace libtext

#endif // LIBTEXT_SEGMENT_SEG_BASE_H_
