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

#include "libtext/normalize/emoji_filter.h"


namespace libtext {

// declare resource
extern const char* const UNICODE_EMOJI_EN[][2];
extern const char* const UNICODE_EMOJI_ALIAS_EN[][2];

template <typename DictContainerType>
static void AppendResource(const char* const resource[], DictContainerType& dic) {
  uint32_t len = 0;
  while (resource[len] != nullptr) {
    dic[turbo::inlined_string(resource[len])] = 0;
    ++len;
  }
}

template <typename DictContainerType>
static void AppendResource(const char* const resource[][2], int idx, DictContainerType& dic) {
  uint32_t len = 0;
  while (resource[len][idx] != nullptr) {
    dic[turbo::inlined_string(resource[len][idx])] = 0;
    ++len;
  }
}


// just for static link
EmojiFilter::EmojiFilter(Options opt) {
  opt_ = opt;
  std::map<turbo::inlined_string , int> dic;
  if (opt_.unicode) {
    AppendResource(UNICODE_EMOJI_EN, 1, dic);
  }
  if (opt_.unicode_trans) {
    AppendResource(UNICODE_EMOJI_EN, 0, dic);
  }
  if (opt_.unicode_trans_alias) {
    AppendResource(UNICODE_EMOJI_ALIAS_EN, 0, dic);
  }
  emoji_codes_ = std::make_shared<PrefixMap>(dic);
}

turbo::inlined_string EmojiFilter::Replace(const std::string_view& str, const std::string_view& repl, bool keep_all) const {
  if (repl.empty()) {
    return Filter(str);
  } else {
    turbo::inlined_string result;
    auto ptr = str.data();
    auto len = str.size();
    result.reserve(len);
    bool last_is_emoji = false;
    while (len > 0) {
      auto match_len = emoji_codes_->PrefixSearch(ptr, len, nullptr);
      if (match_len <= 0) {
        result.push_back(ptr[0]);
        ++ptr;
        --len;
        last_is_emoji = false;
      } else {
        if (keep_all || !last_is_emoji) {
          result.append(repl.data(), repl.size());
        }
        ptr += match_len;
        len -= match_len;
        last_is_emoji = true;
      }
    }
    return result;
  }
}

turbo::inlined_string EmojiFilter::Filter(const std::string_view& str) const {
  turbo::inlined_string result;
  auto ptr = str.data();
  auto len = str.size();
  result.reserve(len);

  while (len > 0) {
    auto match_len = emoji_codes_->PrefixSearch(ptr, len, nullptr);
    if (match_len <= 0) {
      result.push_back(ptr[0]);
      ++ptr;
      --len;
    } else {
      ptr += match_len;
      len -= match_len;
    }
  }

  return result;
}

}  // namespace libtext
