#ifndef HiSPN_OPS
#define HiSPN_OPS 

include "HiSPNDialect.td"
include "HiSPNInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/IR/RegionKindInterface.td"

/// Attributes 代表不同种类的arithmetic error
def AbsoluteErrorAttr : I32EnumAttrCase<"absolute_error",1>;
def RelativeErrorAttr : I32EnumAttrCase<"relative_error",2>;
def AbsoluteLogErrorAttr : I32EnumAttrCase<"absolute_log_error",2>;
def ErrorModelAttr : I32EnumAttr<"error-model","Arithmetic error kind",
[AbsoluteErrorAttr,RelativeErrorAttr,AbsoluteLogErrorAttr]> {
    let cppNamespace = "mlir::spn::high";
}


///
/// Single joint probability evaluation, holds the graph as region of this operation.
///



#endif
