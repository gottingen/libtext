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

#ifndef LIBTEXT_SEGMENT_HMM_MODEL_H_
#define LIBTEXT_SEGMENT_HMM_MODEL_H_

#include <fstream>
#include "libtext/jieba/trie.h"
#include <turbo/strings/ascii.h>
#include <turbo/strings/match.h>
#include <turbo/log/logging.h>
#include <turbo/strings/str_split.h>

namespace libtext {

typedef turbo::flat_hash_map<Rune, double> EmitProbMap;

struct HMMModel {
  /*
   * STATUS:
   * 0: HMMModel::B, 1: HMMModel::E, 2: HMMModel::M, 3:HMMModel::S
   * */
  enum { B = 0, E = 1, M = 2, S = 3, STATUS_SUM = 4 };

  HMMModel(const std::string &modelPath) {
    memset(startProb, 0, sizeof(startProb));
    memset(transProb, 0, sizeof(transProb));
    statMap[0] = 'B';
    statMap[1] = 'E';
    statMap[2] = 'M';
    statMap[3] = 'S';
    emitProbVec.push_back(&emitProbB);
    emitProbVec.push_back(&emitProbE);
    emitProbVec.push_back(&emitProbM);
    emitProbVec.push_back(&emitProbS);
    LoadModel(modelPath);
  }
  ~HMMModel() {}
  void LoadModel(const std::string &filePath) {
    std::ifstream ifile(filePath.c_str());
    TURBO_CHECK(ifile.is_open()) << "open " << filePath << " failed";
    std::string line;
    std::vector<std::string> tmp;
    std::vector<std::string> tmp2;
    // Load startProb
    TURBO_CHECK(GetLine(ifile, line));
    tmp = turbo::StrSplit(line, " ");
    TURBO_CHECK(tmp.size() == STATUS_SUM);
    for (size_t j = 0; j < tmp.size(); j++) {
      startProb[j] = atof(tmp[j].c_str());
    }

    // Load transProb
    for (size_t i = 0; i < STATUS_SUM; i++) {
      TURBO_CHECK(GetLine(ifile, line));
      tmp = turbo::StrSplit(line, " ");
      TURBO_CHECK(tmp.size() == STATUS_SUM);
      for (size_t j = 0; j < STATUS_SUM; j++) {
        transProb[i][j] = atof(tmp[j].c_str());
      }
    }

    // Load emitProbB
    TURBO_CHECK(GetLine(ifile, line));
    TURBO_CHECK(LoadEmitProb(line, emitProbB));

    // Load emitProbE
    TURBO_CHECK(GetLine(ifile, line));
    TURBO_CHECK(LoadEmitProb(line, emitProbE));

    // Load emitProbM
    TURBO_CHECK(GetLine(ifile, line));
    TURBO_CHECK(LoadEmitProb(line, emitProbM));

    // Load emitProbS
    TURBO_CHECK(GetLine(ifile, line));
    TURBO_CHECK(LoadEmitProb(line, emitProbS));
  }
  double GetEmitProb(const EmitProbMap *ptMp, Rune key, double defVal) const {
    EmitProbMap::const_iterator cit = ptMp->find(key);
    if (cit == ptMp->end()) {
      return defVal;
    }
    return cit->second;
  }
  bool GetLine(std::ifstream &ifile, std::string &line) {
    while (getline(ifile, line)) {
      turbo::StripAsciiWhitespace(&line);
      if (line.empty()) {
        continue;
      }
      if (turbo::StartsWith(line, "#")) {
        continue;
      }
      return true;
    }
    return false;
  }
  bool LoadEmitProb(const std::string &line, EmitProbMap &mp) {
    if (line.empty()) {
      return false;
    }
    std::vector<std::string> tmp, tmp2;
    Unicode unicode;
    tmp = turbo::StrSplit(line, ",");
    for (size_t i = 0; i < tmp.size(); i++) {
      tmp2 = turbo::StrSplit(tmp[i], ":");
      if (2 != tmp2.size()) {
        TURBO_LOG(ERROR) << "emitProb illegal.";
        return false;
      }
      if (!DecodeRunesInString(tmp2[0], unicode) || unicode.size() != 1) {
        TURBO_LOG(ERROR) << "TransCode failed.";
        return false;
      }
      mp[unicode[0]] = atof(tmp2[1].c_str());
    }
    return true;
  }

  char statMap[STATUS_SUM];
  double startProb[STATUS_SUM];
  double transProb[STATUS_SUM][STATUS_SUM];
  EmitProbMap emitProbB;
  EmitProbMap emitProbE;
  EmitProbMap emitProbM;
  EmitProbMap emitProbS;
  std::vector<EmitProbMap *> emitProbVec;
}; // struct HMMModel

} // namespace libtext

#endif // LIBTEXT_SEGMENT_HMM_MODEL_H_
