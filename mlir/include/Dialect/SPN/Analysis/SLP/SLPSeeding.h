//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#ifndef SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SLP_SLPSEEDING_H
#define SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SLP_SLPSEEDING_H

#include "llvm/ADT/StringMap.h"

#include "SPN/SPNOps.h"

namespace mlir {
  namespace spn {
    namespace slp {
      namespace seeding {

        typedef llvm::SmallVector<Operation*, 4> seed;

        static std::vector<seed> getSeeds(Operation* root) {
          llvm::StringMap<std::vector<Operation*>> operationsByName;

          for (auto& op : root->getBlock()->getOperations()) {
            if (op.hasTrait<OpTrait::spn::Vectorizable>() || op.hasTrait<OpTrait::spn::Binarizable>()) {
              operationsByName[op.getName().getStringRef()].emplace_back(&op);
            }
          }

          // Sort operations by their number of operands in descending order to maximize vectorization tree sizes.
          for (auto& entry : operationsByName) {
            std::sort(std::begin(entry.second), std::end(entry.second), [&](Operation* a, Operation* b) {
              return a->getNumOperands() > b->getNumOperands();
            });
          }

          return {};
        }
      }
    }
  }
}

#endif //SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SLP_SLPSEEDING_H
