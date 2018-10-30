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

#ifndef FORTRAN_IR_BUILDER_H_
#define FORTRAN_IR_BUILDER_H_

#include "fir-basicblock.h"
#include "statements.h"
#include <initializer_list>

namespace Fortran::IR {

struct FIRBuilder {
  explicit FIRBuilder(BasicBlock &block)
    : cursorRegion{block.getParent()}, cursorBlock{&block} {}
  template<typename A> Statement &Insert(A *s);
  template<typename A> Statement &InsertTerminator(A *s);
  void SetInsertionPoint(BasicBlock *bb);
  void ClearInsertionPoint();
  BasicBlock *GetInsertionPoint() const;
  Statement &createDummyStmt(const parser::CharBlock &pos);
  Statement &createAssign(const parser::AssignmentStmt *stmt);
  Statement &createAssign(const parser::AssignStmt *stmt);
  Statement &createPtrAssign(const parser::PointerAssignmentStmt *stmt);
  Statement &createCall(const parser::CallStmt *stmt);
  Statement &createAlloc(const parser::AllocateStmt *stmt);
  Statement &createDealloc(const parser::DeallocateStmt *stmt);
  Statement &createRetVoid();
  template<typename A> Statement &createReturn(A *expr);
  Statement &createUnreachable();
  Statement &createRuntimeCall(const parser::CharBlock &source);
  template<typename A> Statement &createRuntimeCall(const A *a);
  template<typename A>
  Statement &createConditionalBranch(
      A *condition, BasicBlock *trueBlock, BasicBlock *falseBlock);
  Statement &createBranch(BasicBlock *block);
  template<typename A>
  Statement &createIndirectBr(
      A *var, const std::vector<BasicBlock *> &potentials);
  template<typename A>
  Statement &createSwitch(const A &condition, BasicBlock *defaultCase,
      const SwitchStmt::ValueSuccPairListType &rest);
  void PushBlock(BasicBlock *block);
  BasicBlock *PopBlock();
  void dump() const;
  void SetCurrentRegion(Region *region);
  Region *GetCurrentRegion() const;

private:
  Region *cursorRegion;
  BasicBlock *cursorBlock;
  std::vector<BasicBlock *> blockStack;
};

}

#endif
