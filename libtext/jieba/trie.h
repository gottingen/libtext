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

#ifndef LIBTEXT_SEGMENT_TRIE_H_
#define LIBTEXT_SEGMENT_TRIE_H_

#include "libtext/jieba/unicode.h"
#include <queue>
#include <turbo/container/flat_hash_map.h>
#include <vector>

namespace libtext {

const size_t MAX_WORD_LENGTH = 512;

struct DictUnit {
  Unicode word;
  double weight;
  std::string tag;
}; // struct DictUnit

// for debugging
// inline ostream & operator << (ostream& os, const DictUnit& unit) {
//   string s;
//   s << unit.word;
//   return os << StringFormat("%s %s %.3lf", s.c_str(), unit.tag.c_str(),
//   unit.weight);
// }

struct Dag {
  RuneStr runestr;
  // [offset, nexts.first]
  turbo::InlinedVector<std::pair<size_t, const DictUnit *>, 8> nexts;
  const DictUnit *pInfo;
  double weight;
  size_t nextPos; // TODO
  Dag() : runestr(), pInfo(nullptr), weight(0.0), nextPos(0) {}
}; // struct Dag

typedef Rune TrieKey;

class TrieNode {
public:
  TrieNode() : next(nullptr), ptValue(nullptr) {}

public:
  typedef turbo::flat_hash_map<TrieKey, TrieNode *> NextMap;
  NextMap *next;
  const DictUnit *ptValue;
};

class Trie {
public:
  Trie(const std::vector<Unicode> &keys,
       const std::vector<const DictUnit *> &valuePointers)
      : root_(new TrieNode) {
    CreateTrie(keys, valuePointers);
  }
  ~Trie() { DeleteNode(root_); }

  const DictUnit *Find(RuneStrArray::const_iterator begin,
                       RuneStrArray::const_iterator end) const {
    if (begin == end) {
      return nullptr;
    }

    const TrieNode *ptNode = root_;
    TrieNode::NextMap::const_iterator citer;
    for (RuneStrArray::const_iterator it = begin; it != end; it++) {
      if (nullptr == ptNode->next) {
        return nullptr;
      }
      citer = ptNode->next->find(it->rune);
      if (ptNode->next->end() == citer) {
        return nullptr;
      }
      ptNode = citer->second;
    }
    return ptNode->ptValue;
  }

  void Find(RuneStrArray::const_iterator begin,
            RuneStrArray::const_iterator end, std::vector<struct Dag> &res,
            size_t max_word_len = MAX_WORD_LENGTH) const {
    assert(root_ != nullptr);
    res.resize(static_cast<size_t>(end - begin));

    const TrieNode *ptNode = nullptr;
    TrieNode::NextMap::const_iterator citer;
    for (size_t i = 0; i < size_t(end - begin); i++) {
      res[i].runestr = *(begin + i);

      if (root_->next != nullptr &&
          root_->next->end() !=
              (citer = root_->next->find(res[i].runestr.rune))) {
        ptNode = citer->second;
      } else {
        ptNode = nullptr;
      }
      if (ptNode != nullptr) {
        res[i].nexts.push_back(
            std::pair<size_t, const DictUnit *>(i, ptNode->ptValue));
      } else {
        res[i].nexts.push_back(std::pair<size_t, const DictUnit *>(
            i, static_cast<const DictUnit *>(nullptr)));
      }

      for (size_t j = i + 1;
           j < size_t(end - begin) && (j - i + 1) <= max_word_len; j++) {
        if (ptNode == nullptr || ptNode->next == nullptr) {
          break;
        }
        citer = ptNode->next->find((begin + j)->rune);
        if (ptNode->next->end() == citer) {
          break;
        }
        ptNode = citer->second;
        if (nullptr != ptNode->ptValue) {
          res[i].nexts.push_back(
              std::pair<size_t, const DictUnit *>(j, ptNode->ptValue));
        }
      }
    }
  }

  void InsertNode(const Unicode &key, const DictUnit *ptValue) {
    if (key.begin() == key.end()) {
      return;
    }

    TrieNode::NextMap::const_iterator kmIter;
    TrieNode *ptNode = root_;
    for (Unicode::const_iterator citer = key.begin(); citer != key.end();
         ++citer) {
      if (nullptr == ptNode->next) {
        ptNode->next = new TrieNode::NextMap;
      }
      kmIter = ptNode->next->find(*citer);
      if (ptNode->next->end() == kmIter) {
        TrieNode *nextNode = new TrieNode;

        ptNode->next->insert(std::make_pair(*citer, nextNode));
        ptNode = nextNode;
      } else {
        ptNode = kmIter->second;
      }
    }
    assert(ptNode != nullptr);
    ptNode->ptValue = ptValue;
  }
  void DeleteNode(const Unicode &key, const DictUnit *ptValue) {
    if (key.begin() == key.end()) {
      return;
    }
    // 定义一个NextMap迭代器
    TrieNode::NextMap::const_iterator kmIter;
    // 定义一个指向root的TrieNode指针
    TrieNode *ptNode = root_;
    for (Unicode::const_iterator citer = key.begin(); citer != key.end();
         ++citer) {
      // 链表不存在元素
      if (nullptr == ptNode->next) {
        return;
      }
      kmIter = ptNode->next->find(*citer);
      // if not exist, break loop
      if (ptNode->next->end() == kmIter) {
        break;
      }
      // erase from flat_hash_map
      ptNode->next->erase(*citer);
      // erase node
      ptNode = kmIter->second;
      delete ptNode;
      break;
    }
    return;
  }

private:
  void CreateTrie(const std::vector<Unicode> &keys,
                  const std::vector<const DictUnit *> &valuePointers) {
    if (valuePointers.empty() || keys.empty()) {
      return;
    }
    assert(keys.size() == valuePointers.size());

    for (size_t i = 0; i < keys.size(); i++) {
      InsertNode(keys[i], valuePointers[i]);
    }
  }

  void DeleteNode(TrieNode *node) {
    if (nullptr == node) {
      return;
    }
    if (nullptr != node->next) {
      for (TrieNode::NextMap::iterator it = node->next->begin();
           it != node->next->end(); ++it) {
        DeleteNode(it->second);
      }
      delete node->next;
    }
    delete node;
  }

  TrieNode *root_;
}; // class Trie
} // namespace libtext

#endif // LIBTEXT_SEGMENT_TRIE_H_
