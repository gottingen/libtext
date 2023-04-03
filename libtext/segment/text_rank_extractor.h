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

#ifndef LIBTEXT_SEGMENT_TEXT_RANK_EXTRACTOR_H_
#define LIBTEXT_SEGMENT_TEXT_RANK_EXTRACTOR_H_

#include "libtext/segment/seg.h"
#include <cmath>

namespace libtext {

class TextRankExtractor {
public:
  typedef struct _Word {
    std::string word;
    std::vector<size_t> offsets;
    double weight;
  } Word; // struct Word
private:
  typedef std::map<std::string, Word> WordMap;

  class WordGraph {
  private:
    typedef double Score;
    typedef std::string Node;
    typedef std::set<Node> NodeSet;

    typedef std::map<Node, double> Edges;
    typedef std::map<Node, Edges> Graph;

    double d;
    Graph graph;
    NodeSet nodeSet;

  public:
    WordGraph() : d(0.85){};
    WordGraph(double in_d) : d(in_d){};

    void addEdge(Node start, Node end, double weight) {
      Edges temp;
      Edges::iterator gotEdges;
      nodeSet.insert(start);
      nodeSet.insert(end);
      graph[start][end] += weight;
      graph[end][start] += weight;
    }

    void rank(WordMap &ws, size_t rankTime = 10) {
      WordMap outSum;
      Score wsdef, min_rank, max_rank;

      if (graph.size() == 0)
        return;

      wsdef = 1.0 / graph.size();

      for (Graph::iterator edges = graph.begin(); edges != graph.end();
           ++edges) {
        // edges->first start节点；edge->first end节点；edge->second 权重
        ws[edges->first].word = edges->first;
        ws[edges->first].weight = wsdef;
        outSum[edges->first].weight = 0;
        for (Edges::iterator edge = edges->second.begin();
             edge != edges->second.end(); ++edge) {
          outSum[edges->first].weight += edge->second;
        }
      }
      // sort(nodeSet.begin(),nodeSet.end()); 是否需要排序?
      for (size_t i = 0; i < rankTime; i++) {
        for (NodeSet::iterator node = nodeSet.begin(); node != nodeSet.end();
             node++) {
          double s = 0;
          for (Edges::iterator edge = graph[*node].begin();
               edge != graph[*node].end(); edge++)
            // edge->first end节点；edge->second 权重
            s += edge->second / outSum[edge->first].weight *
                 ws[edge->first].weight;
          ws[*node].weight = (1 - d) + d * s;
        }
      }

      min_rank = max_rank = ws.begin()->second.weight;
      for (WordMap::iterator i = ws.begin(); i != ws.end(); i++) {
        if (i->second.weight < min_rank) {
          min_rank = i->second.weight;
        }
        if (i->second.weight > max_rank) {
          max_rank = i->second.weight;
        }
      }
      for (WordMap::iterator i = ws.begin(); i != ws.end(); i++) {
        ws[i->first].weight =
            (i->second.weight - min_rank / 10.0) / (max_rank - min_rank / 10.0);
      }
    }
  };

public:
  TextRankExtractor(const std::string &dictPath, const std::string &hmmFilePath,
                    const std::string &stopWordPath,
                    const std::string &userDict = "")
      : segment_(dictPath, hmmFilePath, userDict) {
    LoadStopWordDict(stopWordPath);
  }
  TextRankExtractor(const DictTrie *dictTrie, const HMMModel *model,
                    const std::string &stopWordPath)
      : segment_(dictTrie, model) {
    LoadStopWordDict(stopWordPath);
  }
  TextRankExtractor(const Segmentor &segmentor, const std::string &stopWordPath)
      : segment_(segmentor.GetDictTrie(), segmentor.GetHMMModel()) {
    LoadStopWordDict(stopWordPath);
  }
  ~TextRankExtractor() {}

  void Extract(const std::string &sentence, std::vector<std::string> &keywords,
               size_t topN) const {
    std::vector<Word> topWords;
    Extract(sentence, topWords, topN);
    for (size_t i = 0; i < topWords.size(); i++) {
      keywords.push_back(topWords[i].word);
    }
  }

  void Extract(const std::string &sentence,
               std::vector<std::pair<std::string, double>> &keywords,
               size_t topN) const {
    std::vector<Word> topWords;
    Extract(sentence, topWords, topN);
    for (size_t i = 0; i < topWords.size(); i++) {
      keywords.push_back(
          std::pair<std::string, double>(topWords[i].word, topWords[i].weight));
    }
  }

  void Extract(const std::string &sentence, std::vector<Word> &keywords,
               size_t topN, size_t span = 5, size_t rankTime = 10) const {
    std::vector<std::string> words;
    segment_.Cut(sentence, words);

    TextRankExtractor::WordGraph graph;
    WordMap wordmap;
    size_t offset = 0;

    for (size_t i = 0; i < words.size(); i++) {
      size_t t = offset;
      offset += words[i].size();
      if (IsSingleWord(words[i]) ||
          stopWords_.find(words[i]) != stopWords_.end()) {
        continue;
      }
      for (size_t j = i + 1, skip = 0; j < i + span + skip && j < words.size();
           j++) {
        if (IsSingleWord(words[j]) ||
            stopWords_.find(words[j]) != stopWords_.end()) {
          skip++;
          continue;
        }
        graph.addEdge(words[i], words[j], 1);
      }
      wordmap[words[i]].offsets.push_back(t);
    }
    if (offset != sentence.size()) {
      TURBO_LOG(ERROR) << "words illegal";
      return;
    }

    graph.rank(wordmap, rankTime);

    keywords.clear();
    keywords.reserve(wordmap.size());
    for (WordMap::iterator itr = wordmap.begin(); itr != wordmap.end(); ++itr) {
      keywords.push_back(itr->second);
    }

    topN = std::min(topN, keywords.size());
    partial_sort(keywords.begin(), keywords.begin() + topN, keywords.end(),
                 Compare);
    keywords.resize(topN);
  }

private:
  void LoadStopWordDict(const std::string &filePath) {
    std::ifstream ifs(filePath.c_str());
    TURBO_CHECK(ifs.is_open()) << "open " << filePath << " failed";
    std::string line;
    while (getline(ifs, line)) {
      stopWords_.insert(line);
    }
    assert(stopWords_.size());
  }

  static bool Compare(const Word &x, const Word &y) {
    return x.weight > y.weight;
  }

  MixSegment segment_;
  std::unordered_set<std::string> stopWords_;
}; // class TextRankExtractor

inline std::ostream &operator<<(std::ostream &os,
                                const TextRankExtractor::Word &word) {
  return os << "{\"word\": \"" << word.word << "\", \"offset\": {"
            << turbo::StrJoin(word.offsets, ",")
            << "}, \"weight\": " << word.weight << "}";
}

struct TextRankExtractorWordFormatter {
  void operator()(std::string *s, const TextRankExtractor::Word &word) {
    turbo::StrAppend(s, "{word: ",  word.word,
                     ", offset: [",turbo::StrJoin(word.offsets, ", "),
                     "], weight: ", word.weight, "}");
  }
};
} // namespace libtext

#endif // LIBTEXT_SEGMENT_TEXT_RANK_EXTRACTOR_H_
