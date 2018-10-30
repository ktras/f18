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

#ifndef FORTRAN_IR_GRAPH_WRITER_H_
#define FORTRAN_IR_GRAPH_WRITER_H_

#include "fir-program.h"
#include <list>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace Fortran::IR {

struct GraphWriter {
  static void setOutput(llvm::raw_ostream *output);
  static void setOutput(const std::string &filename);
  static void print(Program &program);
  static void print(Procedure &procedure);
  static void print(Region &region);

private:
  GraphWriter(llvm::raw_ostream &output) : output{output} {}
  ~GraphWriter();
  void dump(Program &program);
  void dump(Procedure &procedure, bool box = false);
  void dump(Region &region, bool box = false);
  void dumpHeader();
  void dumpFooter();
  unsigned counter();
  void dump(
      BasicBlock &block, std::optional<const char *> color = std::nullopt);
  void dumpInternalEdges(BasicBlock &block, std::set<BasicBlock *> &nodeSet,
      std::list<std::pair<BasicBlock *, BasicBlock *>> &emitAfter);
  std::list<BasicBlock *> succ_list(BasicBlock &block);
  std::string block_id(BasicBlock &block);
  static llvm::raw_ostream &getOutput();

  unsigned count{0u};
  llvm::raw_ostream &output;
  unsigned blockNum{0u};
  bool isEntry{false};
  bool isExit{false};
  std::map<BasicBlock *, unsigned> blockIds;
  static std::optional<llvm::raw_ostream *> defaultOutput;
};

}

#endif
