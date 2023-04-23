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

#ifndef LIBTEXT_COMMON_PREFIX_MAP_H_
#define LIBTEXT_COMMON_PREFIX_MAP_H_

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "libtext/common/cedar.h"
#include "turbo/strings/inlined_string.h"

namespace libtext {

// Given a list of strings, finds the longest string which is a
// prefix of a query.
class PrefixMap {
#if defined(USE_CEDAR_UNORDERED)
  typedef cedar::da<int, -1, -2, false> cedar_t;
#else
  typedef cedar::da<int> cedar_t;
#endif
 public:
  explicit PrefixMap(const std::map<turbo::inlined_string, int>& dic);

  // Finds the longest string in dic, which is a prefix of `w`.
  // Returns the UTF8 byte length of matched string.
  // `found` is set if a prefix match exists.
  // If no entry is found, return 0.
  size_t PrefixSearch(const char* w, size_t w_len, int* val) const;

 private:
  std::unique_ptr<cedar_t> trie_;
};

}  // namespace libtext
#endif  // LIBTEXT_COMMON_PREFIX_MAP_H_
