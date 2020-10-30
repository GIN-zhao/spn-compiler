//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#include "SPNDialectPipeline.h"
#include "mlir/Transforms/Passes.h"
#include "SPN/SPNPasses.h"
#include "SPNtoStandard/SPNtoStandardConversionPass.h"
#include "SPNtoLLVM/SPNtoLLVMConversionPass.h"
#include <util/Logging.h>

using namespace mlir;
using namespace spnc;

void SPNDialectPipeline::initializePassPipeline(mlir::PassManager* pm, mlir::MLIRContext* ctx) {
  pm->addPass(mlir::spn::createSPNOpSimplifierPass());
  pm->addPass(mlir::createCanonicalizerPass());
  pm->addPass(mlir::spn::createSPNTypePinningPass());
  pm->addPass(mlir::spn::createSPNtoStandardConversionPass());
  pm->addPass(mlir::spn::createSPNtoLLVMConversionPass());
}