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
#include "libtext/common/prefix_matcher.h"

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <turbo/log/logging.h>

namespace libtext {

namespace {
static constexpr const size_t UTF8_BYTE_LEN[16] = {1, 1, 1, 1, 1, 1, 1, 1,
                                                   1, 1, 1, 1, 2, 2, 3, 4};
static inline constexpr size_t OneCharLen(const char *src) noexcept {
  return UTF8_BYTE_LEN[(*((const unsigned char *)src)) >> 4];
}

}

PrefixMatcher::PrefixMatcher(const std::set<std::string> &dic) {
  if (dic.empty())
    return;
  std::vector<const char *> key;
  key.reserve(dic.size());
  for (const auto &it : dic) {
    key.push_back(it.data());
  }
  trie_ = std::make_unique<cedar_t>();
  TURBO_CHECK_EQ(0, trie_->build(key.size(), const_cast<const char **>(&key[0]),
                             nullptr, nullptr));
}

size_t PrefixMatcher::PrefixMatch(const char *w, size_t w_len, bool *found) const {
  if (trie_ == nullptr) {
    if (found) {
      *found = false;
    }
    return std::min<size_t>(w_len, OneCharLen(w));
  }
  constexpr size_t kResultSize = 64;
  cedar_t::result_pair_type trie_results[kResultSize];
  const size_t num_nodes =
      trie_->commonPrefixSearch(w, trie_results, kResultSize, w_len);

  if (found) {
    *found = (num_nodes > 0);
  }
  if (num_nodes == 0) {
    return std::min<size_t>(w_len, OneCharLen(w));
  }
  size_t mblen = 0;
  for (size_t i = 0; i < num_nodes; ++i) {
    mblen = std::max<size_t>(trie_results[i].length, mblen);
  }
  return mblen;
}

size_t PrefixMatcher::PrefixMatch(const std::string &w, bool *found) const {
  return PrefixMatch(w.data(), w.length(), found);
}

size_t PrefixMatcher::PrefixSearch(const char *w, size_t w_len) const {
  if (trie_ == nullptr) {
    return 0;
  }
  constexpr int kResultSize = 64;
  cedar_t::result_pair_type trie_results[kResultSize];
  const size_t num_nodes =
      trie_->commonPrefixSearch(w, trie_results, kResultSize, w_len);

  if (num_nodes == 0) {
    return 0;
  }
  size_t mblen = 0;
  for (size_t i = 0; i < num_nodes; ++i) {
    mblen = std::max<size_t>(trie_results[i].length, mblen);
  }
  return mblen;
}

} // namespace libtext
