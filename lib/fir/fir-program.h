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

#ifndef FORTRAN_IR_PROGRAM_H_
#define FORTRAN_IR_PROGRAM_H_

namespace Fortran::IR {
struct Program;
struct GraphWriter;
}

#include "fir-procedure.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/ilist.h"
#include <string>

namespace Fortran::IR {

// FIXME
using FunctionType = int;
using AttributeList = int;

struct Program final {
  friend GraphWriter;
  using ProcedureListType = llvm::iplist<Procedure>;
  using ProcedureMapType = llvm::StringMap<Procedure *>;

  explicit Program(llvm::StringRef id);
  ~Program();
  void insertBefore(Procedure *subprog, Procedure *before = nullptr);
  ProcedureListType &getSublist(Procedure *);
  bool containsProcedure(llvm::StringRef name);
  std::string getName() const;
  Procedure *getOrInsertProcedure(
      llvm::StringRef name, FunctionType *procTy, AttributeList attrs);

private:
  ProcedureListType procedureList;
  ProcedureMapType procedureMap;
  const std::string name;
};

}

#endif
