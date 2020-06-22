//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#ifndef SPNC_COMPILER_SRC_CODEGEN_MLIR_LOWERING_PATTERNS_LOWERSPNTOLLVMPATTERNS_H
#define SPNC_COMPILER_SRC_CODEGEN_MLIR_LOWERING_PATTERNS_LOWERSPNTOLLVMPATTERNS_H

#include "SPNOperationLowering.h"
#include <codegen/mlir/dialects/spn/SPNDialect.h>

namespace mlir {
  namespace spn {

    ///
    /// Pattern to rewrite a HistogramValueOp to operations from the LLVM dialect.
    struct HistogramValueLowering : public SPNOpLowering<HistogramValueOp> {

      using SPNOpLowering<HistogramValueOp>::SPNOpLowering;

      LogicalResult matchAndRewrite(HistogramValueOp op, ArrayRef<Value> operands,
                                    ConversionPatternRewriter& rewriter) const override;

    };

  }
}

#endif //SPNC_COMPILER_SRC_CODEGEN_MLIR_LOWERING_PATTERNS_LOWERSPNTOLLVMPATTERNS_H
