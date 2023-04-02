//
// Created by 李寅斌 on 2023/4/2.
//

#ifndef LIBTEXT_SEGMENT_SEG_TAGGED_H_
#define LIBTEXT_SEGMENT_SEG_TAGGED_H_

#include "libtext/segment/seg_base.h"

namespace libtext {

class SegmentTagged : public SegmentBase {
public:
  SegmentTagged()  = default;
  virtual  ~SegmentTagged() = default;

  virtual bool
  Tag(const std::string &src,
      std::vector<std::pair<std::string, std::string>> &res) const = 0;

  virtual const DictTrie *GetDictTrie() const = 0;

}; // class SegmentTagged

} // namespace libtext
#endif // LIBTEXT_SEGMENT_SEG_TAGGED_H_
