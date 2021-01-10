//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#ifndef SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SLP_SLPGRAPH_H
#define SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SLP_SLPGRAPH_H

#include "mlir/IR/Operation.h"
#include "mlir/IR/OpDefinition.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringMap.h"
#include "SLPNode.h"

#include <vector>
#include <set>

namespace mlir {
  namespace spn {
    namespace slp {

      ///
      /// Graph class storing Use-Def chains of an SPN.
      class SLPTree {

      public:

        /// Constructor, initialize analysis.
        /// \param root Root node of a (sub-)graph or query operation.
        /// \param width The target width of the SLP vectors.
        explicit SLPTree(Operation* op, size_t width);

      private:

        void buildGraph(std::vector<Operation*> const& values, SLPNode& parentNode);

        bool vectorizable(std::vector<Operation*> const& values) const;
        bool commutative(std::vector<Operation*> const& values) const;
        bool attachableOperands(OperationName const& currentOperation, std::vector<Operation*> const& operands) const;

        SLPNode graph;

      };
    }
  }
}

#endif //SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SLP_SLPGRAPH_H
