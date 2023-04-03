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


#include <gtest/gtest.h>
#include <libtext/common/prefix_matcher.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace libtext {

TEST(PrefixMatcher, PrefixMatch) {
  std::set<std::string> test_data{
      "hello world",
      "happy",
      "world",
      "hi",
      "worldhi",
  };

  PrefixMatcher pm(test_data);
  std::string data("world");
  bool found = false;
  int len = pm.PrefixMatch(data, &found);
  ASSERT_TRUE(found);
  ASSERT_EQ(len, data.size());
  pm.PrefixMatch(std::string("je"), &found);
  ASSERT_FALSE(found);
  // TODO (mxd) : more test
  data = std::string("happy") + "world";
  auto match_len = pm.PrefixSearch(data.data(), data.size());
  ASSERT_EQ(match_len, 5);
}

}  // namespace libtext