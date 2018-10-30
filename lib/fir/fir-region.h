// Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef FORTRAN_IR_REGION_H_
#define FORTRAN_IR_REGION_H_

namespace Fortran::IR {
class Region;
struct GraphWriter;
}

#include "fir-basicblock.h"
#include "fir-procedure.h"
#include "mixin.h"
#include "../parser/char-block.h"
#include "llvm/ADT/ilist.h"

namespace Fortran::IR {

struct Region final : public llvm::ilist_node<Region>,
                      public ChildMixin<Region, Procedure> {
  friend Procedure;
  friend BasicBlock;
  using BasicBlockListType = llvm::iplist<BasicBlock>;
  using AllocatableListType = std::list<Fortran::parser::CharBlock>;  // FIXME
  using SubregionListType = llvm::iplist<Region>;
  using iterator = SubregionListType::iterator;
  using const_iterator = SubregionListType::const_iterator;
  using reverse_iterator = SubregionListType::reverse_iterator;
  using const_reverse_iterator = SubregionListType::const_reverse_iterator;

  Region(const Region &) = delete;
  Region &operator=(const Region &) = delete;
  ~Region();
  std::vector<BasicBlock *> getBlocks();
  std::vector<BasicBlock *> getSublist(BasicBlock *);
  SubregionListType &getSublist(Region *);
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  Region *getEnclosing() const;
  bool isOutermost() const;
  static Region *Create(Procedure *procedure, Region *inRegion = nullptr,
      Region *insertBefore = nullptr);

private:
  BasicBlockListType &basicBlockList;
  AllocatableListType allocatableList;
  SubregionListType subregionList;  // direct descendants
  Region *enclosingRegion;  // parent in nesting tree

  explicit Region(Procedure *procedure, Region *inRegion, Region *insertBefore);
  void insertBefore(BasicBlock *block, BasicBlock *before);
};
}

#endif
