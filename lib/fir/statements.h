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

#ifndef FORTRAN_IR_STATEMENTS_H_
#define FORTRAN_IR_STATEMENTS_H_

#include "mixin.h"
#include "../common/idioms.h"
#include "../common/indirection.h"
#include "../evaluate/expression.h"
#include "../parser/parse-tree.h"
#include "../semantics/symbol.h"
#include <initializer_list>
#include <iostream>

namespace Fortran::IR {

class Statement;
class BasicBlock;
CLASS_TRAIT(StatementTrait)
CLASS_TRAIT(TerminatorTrait)
CLASS_TRAIT(ActionTrait)

using Expression = const common::OwningPointer<evaluate::GenericExprWrapper>;
using Variable = const semantics::Symbol;
using PathVariable = const parser::Variable;

struct Evaluation
  : public SumTypeCopyMixin<
        std::variant<Expression *, Variable *, PathVariable *>> {
  SUM_TYPE_COPY_MIXIN(Evaluation)
  Evaluation(PathVariable *pv) : SumTypeCopyMixin{pv};
  template<typename A> Evaluation(A *a) : SumTypeCopyMixin{a} {}
  std::string dump() const;
};

struct Stmt_impl {
  using StatementTrait = std::true_type;
};

struct TerminatorStmt_impl : public Stmt_impl {
  virtual std::list<BasicBlock *> succ_blocks() const;
  using TerminatorTrait = std::true_type;
};

struct ReturnStmt : public TerminatorStmt_impl {
  ~ReturnStmt() = default;
  static ReturnStmt *Create();
  static ReturnStmt *Create(Expression *expression);

private:
  Expression *returnValue;
  explicit ReturnStmt(Expression *);
};

struct BranchStmt : public TerminatorStmt_impl {
  friend Statement;
  ~BranchStmt() = default;
  static BranchStmt *Create(
      Expression *condition, BasicBlock *trueBlock, BasicBlock *falseBlock);
  static BranchStmt *Create(BasicBlock *succ);
  bool hasCondition() const;
  std::list<BasicBlock *> succ_blocks() const override;

private:
  explicit BranchStmt(
      Expression *condition, BasicBlock *trueBlock, BasicBlock *falseBlock);
  static constexpr unsigned TrueIndex{0u};
  static constexpr unsigned FalseIndex{1u};
  Expression *condition;
  BasicBlock *succs[2];
};

struct SwitchStmt : public TerminatorStmt_impl {
  using ValueSuccPairType = std::pair<Expression *, BasicBlock *>;
  using ValueSuccPairListType = std::vector<ValueSuccPairType>;
  ~SwitchStmt() = default;
  static SwitchStmt *Create(const Evaluation &switchEval,
      BasicBlock *defaultBlock, const ValueSuccPairListType &args);
  BasicBlock *defaultSucc() const;
  std::list<BasicBlock *> succ_blocks() const override;
  const Evaluation &getCond() const;

private:
  explicit SwitchStmt(const Evaluation &condition, BasicBlock *defaultBlock,
      const ValueSuccPairListType &args);
  Evaluation condition;
  ValueSuccPairListType valueSuccPairs;
};

struct IndirectBrStmt : public TerminatorStmt_impl {
  using TargetListType = std::vector<BasicBlock *>;
  ~IndirectBrStmt() = default;
  static IndirectBrStmt *Create(
      Variable *variable, const TargetListType &potentialTargets);

private:
  explicit IndirectBrStmt(
      Variable *variable, const TargetListType &potentialTargets)
    : variable{variable}, potentialTargets{potentialTargets} {}
  Variable *variable;
  TargetListType potentialTargets;
};

struct UnreachableStmt : public TerminatorStmt_impl {
  static UnreachableStmt *Create();
  ~UnreachableStmt() = default;

private:
  explicit UnreachableStmt() = default;
};

struct ActionStmt_impl : public Stmt_impl {
  using ActionTrait = std::true_type;
  // TODO: DynamicType is a placeholder for now
  std::optional<evaluate::DynamicType> type_{std::nullopt};
};

struct AssignmentStmt
  : public ActionStmt_impl,
    private SumTypeCopyMixin<std::variant<const parser::AssignmentStmt *,
        const parser::AssignStmt *>> {
  friend Statement;
  ~AssignmentStmt() = default;
  static AssignmentStmt *Create(const parser::AssignmentStmt *s);
  static AssignmentStmt *Create(const parser::AssignStmt *s);

private:
  explicit AssignmentStmt(const parser::AssignmentStmt *s)
    : SumTypeCopyMixin{s} {}
  explicit AssignmentStmt(const parser::AssignStmt *s) : SumTypeCopyMixin{s} {}
};

struct PointerAssignStmt : public ActionStmt_impl {
  friend Statement;
  ~PointerAssignStmt() = default;
  static PointerAssignStmt *Create(const parser::PointerAssignmentStmt *s);

private:
  explicit PointerAssignStmt(const parser::PointerAssignmentStmt *s)
    : assign{s} {}
  const parser::PointerAssignmentStmt *assign;
};

struct MemoryStmt_impl : public ActionStmt_impl {};
struct LoadStmt : public MemoryStmt_impl {};
struct StoreStmt : public MemoryStmt_impl {};

struct AllocateStmt : public MemoryStmt_impl {
  friend Statement;
  ~AllocateStmt() = default;
  static AllocateStmt *Create(const parser::AllocateStmt *s);

private:
  explicit AllocateStmt(const parser::AllocateStmt *s) : alloc{s} {}
  const parser::AllocateStmt *alloc;
};

struct DeallocateStmt : public MemoryStmt_impl {
  friend Statement;
  ~DeallocateStmt() = default;
  static DeallocateStmt *Create(const parser::DeallocateStmt *s);

private:
  explicit DeallocateStmt(const parser::DeallocateStmt *s) : dealloc{s} {}
  const parser::DeallocateStmt *dealloc;
};

struct CastingStmt_impl : public ActionStmt_impl {};
struct TruncStmt : public CastingStmt_impl {};
struct ExtendStmt : public CastingStmt_impl {};
struct FPToIntStmt : public CastingStmt_impl {};
struct IntToFPStmt : public CastingStmt_impl {};
struct FPTruncStmt : public CastingStmt_impl {};
struct FPExtendStmt : public CastingStmt_impl {};

struct CompareStmt_impl : public ActionStmt_impl {};

struct IntCompareStmt : public CompareStmt_impl {};

struct FPCompareStmt : public CompareStmt_impl {};

struct ScopeStmt_impl : public ActionStmt_impl {};
struct ScopeEnterStmt : public ScopeStmt_impl {};
struct ScopeExitStmt : public ScopeStmt_impl {};
struct PHIStmt : public ActionStmt_impl {};
struct EvalStmt : public ActionStmt_impl {};
struct CallStmt_impl : public ActionStmt_impl {};

struct CallStmt : public CallStmt_impl {
  friend Statement;
  ~CallStmt() = default;
  static CallStmt *Create(const parser::CallStmt *s);

private:
  explicit CallStmt(const parser::CallStmt *s) : call{s} {}
  const parser::CallStmt *call;
};

struct RuntimeStmt
  : public CallStmt_impl,
    private SumTypeCopyMixin<
        std::variant<const parser::BackspaceStmt *, const parser::CloseStmt *,
            const parser::EndfileStmt *, const parser::FailImageStmt *,
            const parser::StopStmt *, /*fixme*/ unsigned>> {
  friend Statement;
  ~RuntimeStmt() = default;
  template<typename T> static RuntimeStmt *Create(const T *s);
  static RuntimeStmt *Create(const parser::CharBlock &pos);

private:
  template<typename T> explicit RuntimeStmt(const T *s) : SumTypeCopyMixin{s} {}
  explicit RuntimeStmt(const parser::CharBlock &pos)
    : SumTypeCopyMixin{0u}, text{pos} {}
  const parser::CharBlock text;
};

}

#endif
