#pragma once

#include <mlir/IR/Dialect.h>

namespace mlir
{
    namespace spn
    {
        namespace high
        {
            class ProbabilityType : Type::TypeBase<ProbabilityType, Type, TypeStorage>
            {
            public:
                using Base::Base;
            }
        }
    }
}

#include "HiSPN/HiSPNDialect.h.inc"