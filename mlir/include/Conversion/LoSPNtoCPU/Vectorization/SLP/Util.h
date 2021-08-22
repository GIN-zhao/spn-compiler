//==============================================================================
// This file is part of the SPNC project under the Apache License v2.0 by the
// Embedded Systems and Applications Group, TU Darmstadt.
// For the full copyright and license information, please view the LICENSE
// file that was distributed with this source code.
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

#ifndef SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_UTIL_H
#define SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_UTIL_H

#include "SLPGraph.h"
#include "LoSPN/LoSPNOps.h"
#include "mlir/IR/Operation.h"

namespace mlir {
  namespace spn {
    namespace low {
      namespace slp {

        namespace option {
          // For explanations, see GlobalOptions.h.
          extern unsigned maxNodeSize;
          extern unsigned maxLookAhead;
          extern unsigned maxAttempts;
          extern unsigned maxSuccessfulIterations;
          extern bool reorderInstructionsDFS;
          extern bool allowDuplicateElements;
          extern bool allowTopologicalMixing;
          extern bool useXorChains;
        }

        bool vectorizable(Operation* op);
        bool vectorizable(Value value);

        template<typename ValueIterator>
        bool vectorizable(ValueIterator begin, ValueIterator end) {
          if (begin->template isa<BlockArgument>()) {
            return false;
          }
          auto const& name = begin->getDefiningOp()->getName();
          ++begin;
          while (begin != end) {
            if (!vectorizable(*begin) || begin->getDefiningOp()->getName() != name) {
              return false;
            }
            ++begin;
          }
          return true;
        }

        bool ofVectorizableType(Value value);

        template<typename ValueIterator>
        bool ofVectorizableType(ValueIterator begin, ValueIterator end) {
          return std::all_of(begin, end, [&](auto const& value) {
            return ofVectorizableType(value);
          });
        }

        bool commutative(Value value);

        template<typename ValueIterator>
        bool commutative(ValueIterator begin, ValueIterator end) {
          while (begin != end) {
            if (!commutative(*begin)) {
              return false;
            }
            ++begin;
          }
          return true;
        }

        bool consecutiveLoads(Value lhs, Value rhs);

        template<typename ValueIterator>
        bool consecutiveLoads(ValueIterator begin, ValueIterator end) {
          Value previous = *begin;
          if (++begin == end || previous.isa<BlockArgument>() || !dyn_cast<SPNBatchRead>(previous.getDefiningOp())) {
            return false;
          }
          while (begin != end) {
            Value current = *begin;
            if (!consecutiveLoads(previous, current)) {
              return false;
            }
            previous = current;
            ++begin;
          }
          return true;
        }

        template<typename ValueIterator>
        bool allLeaf(ValueIterator begin, ValueIterator end) {
          while (begin != end) {
            if (auto* definingOp = begin->getDefiningOp()) {
              if (!dyn_cast<LeafNodeInterface>(definingOp)) {
                return false;
              }
              ++begin;
            } else {
              return false;
            }
          }
          return true;
        }

        bool anyGaussianMarginalized(Superword const& superword);

        SmallVector<Value, 2> getOperands(Value value);

        void sortByOpcode(SmallVectorImpl<Value>& values, Optional<OperationName> smallestOpcode = llvm::None);

        void dumpSuperword(Superword const& superword);
        void dumpSLPNode(SLPNode const& node);

        void dumpOpGraph(ArrayRef<Value> values);
        void dumpSuperwordGraph(Superword* root);
        void dumpSLPGraph(SLPNode* root, bool includeInputs = false);
        void dumpDependencyGraph(DependencyGraph const& dependencyGraph);

      }
    }
  }
}

#endif //SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_UTIL_H
