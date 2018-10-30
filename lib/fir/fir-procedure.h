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

#ifndef FORTRAN_IR_PROCEDURE_H_
#define FORTRAN_IR_PROCEDURE_H_

namespace Fortran::IR {
class Procedure;
struct GraphWriter;
}

#include "fir-program.h"
#include "fir-region.h"
#include "mixin.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/ilist.h"

namespace Fortran::IR {

// FIXME
using FunctionType = int;
using LinkageTypes = int;

struct Procedure final : public llvm::ilist_node<Procedure>,
                         public ChildMixin<Procedure, Program> {
  friend GraphWriter;
  friend Program;
  friend Region;
  using BasicBlockListType = llvm::iplist<BasicBlock>;
  using RegionListType = llvm::iplist<Region>;
  using iterator = BasicBlockListType::iterator;
  using const_iterator = BasicBlockListType::const_iterator;
  using reverse_iterator = BasicBlockListType::reverse_iterator;
  using const_reverse_iterator = BasicBlockListType::const_reverse_iterator;

  Procedure(const Procedure &) = delete;
  Procedure &operator=(const Procedure &) = delete;
  ~Procedure();
  BasicBlockListType &getBlocks();
  BasicBlockListType &getSublist(BasicBlock *);
  RegionListType &getSublist(Region *);
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  Region *getLastRegion();
  BasicBlock *StartBlock();
  static Procedure *Create(Program *prog, FunctionType *ty,
      LinkageTypes linkage, unsigned addrSpace = 0u,
      const llvm::Twine &name = "", Procedure *before = nullptr);
  void setParent(Program *p);
  bool hasName() const;
  llvm::StringRef getName() const;
  void FlattenRegions();

private:
  RegionListType regionList;
  BasicBlockListType basicBlockList;
  FunctionType *procType;
  LinkageTypes linkage;
  unsigned addressSpace;
  const std::string name;

  explicit Procedure(Program *program, FunctionType *ty, LinkageTypes lt,
      unsigned addrSpace, const llvm::Twine &n, Procedure *before);
  Region *insertBefore(Region *region, Region *before = nullptr);
};

}

#endif
