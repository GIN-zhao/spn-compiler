//==============================================================================
// This file is part of the SPNC project under the Apache License v2.0 by the
// Embedded Systems and Applications Group, TU Darmstadt.
// For the full copyright and license information, please view the LICENSE
// file that was distributed with this source code.
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

#ifndef SPNC_MLIR_INCLUDE_DIALECT_HISPN_HISPNOPS_H
#define SPNC_MLIR_INCLUDE_DIALECT_HISPN_HISPNOPS_H

#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/IR/RegionKindInterface.h"
#include "HiSPN/HiSPNInterfaces.h"
#include "HiSPN/HiSPNEnums.h"

#define GET_OP_CLASSES
#include "HiSPN/HiSPNOps.h.inc"

#endif //SPNC_MLIR_INCLUDE_DIALECT_HISPN_HISPNOPS_H
