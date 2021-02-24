//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#include "LoSPNTransformations.h"
#include "LoSPN/LoSPNPasses.h"

void spnc::LoSPNTransformations::initializePassPipeline(mlir::PassManager* pm, mlir::MLIRContext* ctx) {
  pm->addPass(mlir::spn::low::createLoSPNBufferizePass());
}
