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

#ifndef FORTRAN_IR_BASICBLOCK_H_
#define FORTRAN_IR_BASICBLOCK_H_

namespace Fortran::IR {
class BasicBlock;
class Statement;
struct Stmt_impl;
}

#include "fir-region.h"
#include "mixin.h"
#include "statements.h"
#include "llvm/ADT/ilist.h"
#include <iostream>

namespace Fortran::IR {

struct Statement : public SumTypeMixin<std::variant<
#define HANDLE_STMT(num, opcode, name) name,
#define HANDLE_LAST_STMT(num, opcode, name) name
#include "fir-statement.def"
                       >>,
                   public ChildMixin<Statement, BasicBlock>,
                   public llvm::ilist_node<Statement> {
  template<typename A> Statement(BasicBlock *p, A &&t);
  std::string dump() const;
};

struct BasicBlock final : public llvm::ilist_node<BasicBlock>,
                          public ChildMixin<BasicBlock, Region> {
  using StatementListType = llvm::iplist<Statement>;
  using iterator = StatementListType::iterator;
  using const_iterator = StatementListType::const_iterator;
  using reverse_iterator = StatementListType::reverse_iterator;
  using const_reverse_iterator = StatementListType::const_reverse_iterator;

  BasicBlock(const BasicBlock &) = delete;
  BasicBlock &operator=(const BasicBlock &) = delete;
  ~BasicBlock();
  StatementListType &getSublist(Statement *);
  void insertBefore(Statement *stmt, Statement *before = nullptr);
  static BasicBlock *Create(
      Region *parentRegion, BasicBlock *insertBefore = nullptr);
  const Statement *getTerminator() const;
  Statement *getTerminator();
  void SetRegion(Region *region);
  void addPred(BasicBlock *bb);

private:
  StatementListType statementList;
  std::vector<BasicBlock *> predecessors;
  explicit BasicBlock(Region *parentRegion, BasicBlock *insertBefore);
};

}

#endif
