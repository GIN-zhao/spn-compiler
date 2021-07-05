//==============================================================================
// This file is part of the SPNC project under the Apache License v2.0 by the
// Embedded Systems and Applications Group, TU Darmstadt.
// For the full copyright and license information, please view the LICENSE
// file that was distributed with this source code.
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

#ifndef SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_COSTMODEL_H
#define SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_COSTMODEL_H

#include "SLPGraph.h"
#include "GraphConversion.h"
#include "PatternVisitors.h"

namespace mlir {
  namespace spn {
    namespace low {
      namespace slp {

        class CostModel : public PatternVisitor {
        public:
          double getScalarCost(Value const& value);
          double getSuperwordCost(Superword* superword, SLPVectorizationPattern* pattern);
          bool isExtractionProfitable(Value const& value);
          void setConversionState(std::shared_ptr<ConversionState> newConversionState);
          double getBlockCost(Block* block, SmallPtrSetImpl<Operation*> const& deadOps) const;
        protected:
          virtual double computeScalarCost(Value const& value) const = 0;
          virtual double computeExtractionCost(Superword* superword, size_t index) const = 0;
          double cost;
          // For insertion/extraction cost computation.
          LeafPatternVisitor leafVisitor;
          std::shared_ptr<ConversionState> conversionState;
        private:
          void updateCost(Value const& value, double newCost, bool updateUses);
          double getExtractionCost(Value const& value) const;
          static constexpr double MAX_COST = std::numeric_limits<double>::max();
          DenseMap<Value, double> cachedScalarCost;
        };

        class UnitCostModel : public CostModel {
          double computeScalarCost(Value const& value) const override;
          double computeExtractionCost(Superword* superword, size_t index) const override;
          void visitDefault(SLPVectorizationPattern const* pattern, Superword* superword) override;
          void visit(BroadcastSuperword const* pattern, Superword* superword) override;
          void visit(BroadcastInsertSuperword const* pattern, Superword* superword) override;
          void visit(VectorizeConstant const* pattern, Superword* superword) override;
          void visit(VectorizeSPNConstant const* pattern, Superword* superword) override;
          void visit(VectorizeGaussian const* pattern, Superword* superword) override;
          void visit(VectorizeLogAdd const* pattern, Superword* superword) override;
          void visit(VectorizeLogGaussian const* pattern, Superword* superword) override;
        };
      }
    }
  }
}

#endif //SPNC_MLIR_INCLUDE_CONVERSION_LOSPNTOCPU_VECTORIZATION_SLP_COSTMODEL_H
