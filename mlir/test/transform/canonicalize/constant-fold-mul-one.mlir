// RUN: %optcall --canonicalize %s | FileCheck %s

module  {
  "lo_spn.kernel"() ( {
  ^bb0(%arg0: memref<?x2xf64>, %arg1: memref<?xf64>):  // no predecessors
    %c0 = constant 0 : index
    %0 = dim %arg0, %c0 : memref<?x2xf64>
    %1 = alloc(%0) : memref<?xf64>
    "lo_spn.task"(%arg0, %1) ( {
    ^bb0(%arg2: index, %arg3: memref<?x2xf64>, %arg4: memref<?xf64>):  // no predecessors
      %4 = "lo_spn.batch_read"(%arg3, %arg2) {sampleIndex = 0 : ui32} : (memref<?x2xf64>, index) -> f64
      %5 = "lo_spn.batch_read"(%arg3, %arg2) {sampleIndex = 1 : ui32} : (memref<?x2xf64>, index) -> f64
      %6 = "lo_spn.body"(%4, %5) ( {
      ^bb0(%arg5: f64, %arg6: f64):  // no predecessors
        %20 = "lo_spn.mul"(%arg5, %arg6) : (f64, f64) -> f64
        %21 = "lo_spn.constant"() {type = f64, value = 1.0e+00 : f64} : () -> f64
        %22 = "lo_spn.mul"(%20, %21) : (f64, f64) -> f64
        %23 = "lo_spn.constant"() {type = f64, value = 2.0e+00 : f64} : () -> f64
        %24 = "lo_spn.constant"() {type = f64, value = 3.0e+00 : f64} : () -> f64
        %25 = "lo_spn.mul"(%23, %24) : (f64, f64) -> f64
        %26 = "lo_spn.mul"(%22, %25) : (f64, f64) -> f64
        "lo_spn.yield"(%26) : (f64) -> ()
      }) : (f64, f64) -> f64
      "lo_spn.batch_write"(%6, %arg4, %arg2) : (f64, memref<?xf64>, index) -> ()
      "lo_spn.return"() : () -> ()
    }) {batchSize = 1 : ui32} : (memref<?x2xf64>, memref<?xf64>) -> ()
    %2 = tensor_load %1 : memref<?xf64>
    %3 = tensor_to_memref %2 : memref<?xf64>
    "lo_spn.copy"(%3, %arg1) : (memref<?xf64>, memref<?xf64>) -> ()
    "lo_spn.return"() : () -> ()
  }) {sym_name = "spn_kernel", type = (memref<?x2xf64>, memref<?xf64>) -> ()} : () -> ()
}

// NOTE: Assertions have been autogenerated by utils/generate-test-checks.py


// CHECK-LABEL:   "lo_spn.kernel"() ( {
// CHECK:         ^bb0(%[[VAL_0:.*]]: memref<?x2xf64>, %[[VAL_1:.*]]: memref<?xf64>):
// CHECK:           %[[VAL_2:.*]] = constant 0 : index
// CHECK:           %[[VAL_3:.*]] = dim %[[VAL_0]], %[[VAL_2]] : memref<?x2xf64>
// CHECK:           %[[VAL_4:.*]] = alloc(%[[VAL_3]]) : memref<?xf64>
// CHECK:           "lo_spn.task"(%[[VAL_0]], %[[VAL_4]]) ( {
// CHECK:           ^bb0(%[[VAL_5:.*]]: index, %[[VAL_6:.*]]: memref<?x2xf64>, %[[VAL_7:.*]]: memref<?xf64>):
// CHECK:             %[[VAL_8:.*]] = "lo_spn.batch_read"(%[[VAL_6]], %[[VAL_5]]) {sampleIndex = 0 : ui32} : (memref<?x2xf64>, index) -> f64
// CHECK:             %[[VAL_9:.*]] = "lo_spn.batch_read"(%[[VAL_6]], %[[VAL_5]]) {sampleIndex = 1 : ui32} : (memref<?x2xf64>, index) -> f64
// CHECK:             %[[VAL_10:.*]] = "lo_spn.body"(%[[VAL_8]], %[[VAL_9]]) ( {
// CHECK:             ^bb0(%[[VAL_11:.*]]: f64, %[[VAL_12:.*]]: f64):
// CHECK:               %[[VAL_13:.*]] = constant 6.000000e+00 : f64
// CHECK:               %[[VAL_14:.*]] = "lo_spn.mul"(%[[VAL_11]], %[[VAL_12]]) : (f64, f64) -> f64
// CHECK:               %[[VAL_15:.*]] = "lo_spn.mul"(%[[VAL_14]], %[[VAL_13]]) : (f64, f64) -> f64
// CHECK:               "lo_spn.yield"(%[[VAL_15]]) : (f64) -> ()
// CHECK:             }) : (f64, f64) -> f64
// CHECK:             "lo_spn.batch_write"(%[[VAL_16:.*]], %[[VAL_7]], %[[VAL_5]]) : (f64, memref<?xf64>, index) -> ()
// CHECK:             "lo_spn.return"() : () -> ()
// CHECK:           }) {batchSize = 1 : ui32} : (memref<?x2xf64>, memref<?xf64>) -> ()
// CHECK:           "lo_spn.copy"(%[[VAL_4]], %[[VAL_1]]) : (memref<?xf64>, memref<?xf64>) -> ()
// CHECK:           "lo_spn.return"() : () -> ()
// CHECK:         }) {sym_name = "spn_kernel", type = (memref<?x2xf64>, memref<?xf64>) -> ()} : () -> ()
