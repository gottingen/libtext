//
// Created by 李寅斌 on 2023/4/2.
//

#ifndef LIBTEXT_SEGMENT_SEG_H_
#define LIBTEXT_SEGMENT_SEG_H_

#include "libtext/segment/keyword_extrator.h"
#include "libtext/segment/query_seg.h"

namespace libtext {

class Segmentor {
public:
  Segmentor(const std::string &dict_path, const std::string &model_path,
            const std::string &user_dict_path, const std::string &idfPath,
            const std::string &stopWordPath)
      : dict_trie_(dict_path, user_dict_path), model_(model_path),
        mp_seg_(&dict_trie_), hmm_seg_(&model_), mix_seg_(&dict_trie_, &model_),
        full_seg_(&dict_trie_), query_seg_(&dict_trie_, &model_),
        extractor(&dict_trie_, &model_, idfPath, stopWordPath) {}
  ~Segmentor() {}

  struct LocWord {
    std::string word;
    size_t begin;
    size_t end;
  }; // struct LocWord

  void Cut(const std::string &sentence, std::vector<std::string> &words,
           bool hmm = true) const {
    mix_seg_.Cut(sentence, words, hmm);
  }
  void Cut(const std::string &sentence, std::vector<Word> &words,
           bool hmm = true) const {
    mix_seg_.Cut(sentence, words, hmm);
  }
  void CutAll(const std::string &sentence,
              std::vector<std::string> &words) const {
    full_seg_.Cut(sentence, words);
  }
  void CutAll(const std::string &sentence, std::vector<Word> &words) const {
    full_seg_.Cut(sentence, words);
  }
  void CutForSearch(const std::string &sentence,
                    std::vector<std::string> &words, bool hmm = true) const {
    query_seg_.Cut(sentence, words, hmm);
  }
  void CutForSearch(const std::string &sentence, std::vector<Word> &words,
                    bool hmm = true) const {
    query_seg_.Cut(sentence, words, hmm);
  }
  void CutHMM(const std::string &sentence,
              std::vector<std::string> &words) const {
    hmm_seg_.Cut(sentence, words);
  }
  void CutHMM(const std::string &sentence, std::vector<Word> &words) const {
    hmm_seg_.Cut(sentence, words);
  }
  void CutSmall(const std::string &sentence, std::vector<std::string> &words,
                size_t max_word_len) const {
    mp_seg_.Cut(sentence, words, max_word_len);
  }
  void CutSmall(const std::string &sentence, std::vector<Word> &words,
                size_t max_word_len) const {
    mp_seg_.Cut(sentence, words, max_word_len);
  }

  void Tag(const std::string &sentence,
           std::vector<std::pair<std::string, std::string>> &words) const {
    mix_seg_.Tag(sentence, words);
  }
  std::string LookupTag(const std::string &str) const {
    return mix_seg_.LookupTag(str);
  }
  bool InsertUserWord(const std::string &word,
                      const std::string &tag = UNKNOWN_TAG) {
    return dict_trie_.InsertUserWord(word, tag);
  }

  bool InsertUserWord(const std::string &word, int freq,
                      const std::string &tag = UNKNOWN_TAG) {
    return dict_trie_.InsertUserWord(word, freq, tag);
  }

  bool DeleteUserWord(const std::string &word,
                      const std::string &tag = UNKNOWN_TAG) {
    return dict_trie_.DeleteUserWord(word, tag);
  }

  bool Find(const std::string &word) { return dict_trie_.Find(word); }

  void ResetSeparators(const std::string &s) {
    // TODO
    mp_seg_.ResetSeparators(s);
    hmm_seg_.ResetSeparators(s);
    mix_seg_.ResetSeparators(s);
    full_seg_.ResetSeparators(s);
    query_seg_.ResetSeparators(s);
  }

  const DictTrie *GetDictTrie() const { return &dict_trie_; }

  const HMMModel *GetHMMModel() const { return &model_; }

  void LoadUserDict(const std::vector<std::string> &buf) {
    dict_trie_.LoadUserDict(buf);
  }

  void LoadUserDict(const std::set<std::string> &buf) {
    dict_trie_.LoadUserDict(buf);
  }

  void LoadUserDict(const std::string &path) { dict_trie_.LoadUserDict(path); }

private:
  DictTrie dict_trie_;
  HMMModel model_;

  // They share the same dict trie and model
  MPSegment mp_seg_;
  HMMSegment hmm_seg_;
  MixSegment mix_seg_;
  FullSegment full_seg_;
  QuerySegment query_seg_;

public:
  KeywordExtractor extractor;
}; // class Segmentor

} // namespace libtext

#endif // LIBTEXT_SEGMENT_SEG_H_
