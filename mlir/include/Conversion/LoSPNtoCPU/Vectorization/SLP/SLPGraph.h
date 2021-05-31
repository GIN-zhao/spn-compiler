//==============================================================================
// This file is part of the SPNC project under the Apache License v2.0 by the
// Embedded Systems and Applications Group, TU Darmstadt.
// For the full copyright and license information, please view the LICENSE
// file that was distributed with this source code.
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

#ifndef SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_SLPGRAPH_H
#define SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_SLPGRAPH_H

#include "mlir/IR/Operation.h"
#include "mlir/IR/BuiltinTypes.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"

namespace mlir {
  namespace spn {
    namespace low {
      namespace slp {

        class SLPNode;

        class Superword {

          friend SLPNode;

        public:

          explicit Superword(ArrayRef<Value> const& values);
          explicit Superword(ArrayRef<Operation*> const& operations);

          Value getElement(size_t lane) const;
          void setElement(size_t lane, Value const& value);
          Value operator[](size_t lane) const;
          bool contains(Value const& value) const;

          bool isLeaf() const;
          bool uniform() const;
          bool splattable() const;

          size_t numLanes() const;
          SmallVectorImpl<Value>::const_iterator begin() const;
          SmallVectorImpl<Value>::const_iterator end() const;

          size_t numOperands() const;
          void addOperand(std::shared_ptr<Superword> operandWord);
          Superword* getOperand(size_t index) const;
          SmallVector<Superword*, 2> getOperands() const;

          VectorType getVectorType() const;
          Type getElementType() const;
          Location getLoc() const;

        private:
          SmallVector<Value, 4> values;
          SmallVector<std::shared_ptr<Superword>> operandWords;
        };

        class SLPNode {

        public:

          SLPNode(std::shared_ptr<Superword> superword);

          void addSuperword(std::shared_ptr<Superword> superword);
          std::shared_ptr<Superword> getSuperword(size_t index) const;

          Value getValue(size_t lane, size_t index) const;
          void setValue(size_t lane, size_t index, Value const& newValue);

          bool contains(Value const& value) const;

          bool isSuperwordRoot(Superword const& superword) const;

          size_t numLanes() const;
          size_t numSuperwords() const;
          size_t numOperands() const;

          void addOperand(std::shared_ptr<SLPNode> operandNode);
          SLPNode* getOperand(size_t index) const;
          ArrayRef<std::shared_ptr<SLPNode>> getOperands() const;

        private:
          SmallVector<std::shared_ptr<Superword>> superwords;
          SmallVector<std::shared_ptr<SLPNode>> operandNodes;
        };

        class SLPGraph {
          friend class SLPGraphBuilder;
        public:
          SLPGraph(ArrayRef<Value> const& seed, unsigned lookAhead);
          Superword* getRoot() const;
          DenseMap<Superword*, SmallPtrSet<Superword*, 4>> dependencyMap() const;
        private:
          std::shared_ptr<Superword> root;
          DenseMap<Value, SmallVector<std::shared_ptr<Superword>>> superwordsByValue;
          unsigned const lookAhead;
        };

        namespace graph {

          /// Walks through the graph rooted at 'root' in post order and applies function 'f' to every node.
          template<typename Node, typename Function>
          void walk(Node* root, Function f) {
            // false = visit operands, true = finished
            SmallVector<std::pair<Node*, bool>> worklist;
            llvm::SmallSet<Node*, 32> finishedNodes;
            worklist.emplace_back(root, false);
            while (!worklist.empty()) {
              if (finishedNodes.contains(worklist.back().first)) {
                worklist.pop_back();
                continue;
              }
              auto* node = worklist.back().first;
              bool insert = worklist.back().second;
              worklist.pop_back();
              if (insert) {
                finishedNodes.insert(node);
                f(node);
              } else {
                worklist.emplace_back(node, true);
                for (size_t i = node->numOperands(); i-- > 0;) {
                  worklist.emplace_back(node->getOperand(i), false);
                }
              }
            }
          }

          template<typename Node>
          SmallVector<Node*> postOrder(Node* root) {
            SmallVector<Node*> order;
            walk(root, [&](Node* node) {
              order.template emplace_back(node);
            });
            return order;
          }

        }
      }
    }
  }
}

#endif //SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_SLPGRAPH_H
