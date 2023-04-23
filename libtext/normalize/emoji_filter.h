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

#ifndef LIBTEXT_NORMALIZE_EMOJI_FILTER_H_
#define LIBTEXT_NORMALIZE_EMOJI_FILTER_H_

#include <memory>
#include <cstddef>
#include <cstdint>
#include <string>
#include <map>
#include "turbo/strings/string_view.h"
#include "turbo/strings/inlined_string.h"
#include "libtext/common/prefix_map.h"

namespace libtext {

class EmojiFilter {
public:
  struct Options {
    bool unicode;             // general unicode emoji
    bool unicode_trans;       // general unicode emoji transcription
    bool unicode_trans_alias; // general unicode emoji transcription alias

    Options()
        : unicode(true), unicode_trans(false), unicode_trans_alias(false){}
  };

  explicit EmojiFilter(Options opt);
  virtual ~EmojiFilter() = default;

  // check ptr[pos: return_len] is emoji
  inline size_t CheckPos(const char *ptr, size_t len, size_t pos = 0) const {
    if (pos > len) {
      return 0;
    }
    return emoji_codes_->PrefixSearch(ptr + pos, len - pos, nullptr);
  }

  turbo::inlined_string Replace(const std::string_view &str, const std::string_view &repl,
                 bool keep_all = true) const;

  turbo::inlined_string Filter(const std::string_view &str) const;

private:
  Options opt_;
  std::shared_ptr<PrefixMap> emoji_codes_;
};

} // namespace libtext

#endif // LIBTEXT_NORMALIZE_EMOJI_FILTER_H_
