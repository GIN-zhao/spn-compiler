// RUN: %optcall --lospn-bufferize %s | FileCheck %s

module  {
  "lo_spn.kernel"() ( {
  ^bb0(%arg0: tensor<?x2xi32>):  // no predecessors
    %0 = "lo_spn.task"(%arg0) ( {
    ^bb0(%arg1: index, %arg2: tensor<?x2xi32>):  // no predecessors
      %1 = "lo_spn.batch_extract"(%arg2, %arg1) {sampleIndex = 0 : ui32} : (tensor<?x2xi32>, index) -> i32
      %2 = "lo_spn.batch_extract"(%arg2, %arg1) {sampleIndex = 1 : ui32} : (tensor<?x2xi32>, index) -> i32
      %3 = "lo_spn.body"(%1, %2) ( {
      ^bb0(%arg3: i32, %arg4: i32):  // no predecessors
        %5 = "lo_spn.histogram"(%arg3) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 2.500000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 7.500000e-01 : f64}], supportMarginal = false} : (i32) -> f64
        %6 = "lo_spn.histogram"(%arg4) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 4.500000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 5.500000e-01 : f64}], supportMarginal = false} : (i32) -> f64
        %7 = "lo_spn.mul"(%5, %6) : (f64, f64) -> f64
        %8 = "lo_spn.histogram"(%arg3) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 3.300000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 6.700000e-01 : f64}], supportMarginal = false} : (i32) -> f64
        %9 = "lo_spn.histogram"(%arg4) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 8.750000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 1.250000e-01 : f64}], supportMarginal = false} : (i32) -> f64
        %10 = "lo_spn.mul"(%8, %9) : (f64, f64) -> f64
        %11 = "lo_spn.constant"() {type = f64, value = 3.000000e-01 : f64} : () -> f64
        %12 = "lo_spn.mul"(%7, %11) : (f64, f64) -> f64
        %13 = "lo_spn.constant"() {type = f64, value = 0.69999999999999996 : f64} : () -> f64
        %14 = "lo_spn.mul"(%10, %13) : (f64, f64) -> f64
        %15 = "lo_spn.add"(%12, %14) : (f64, f64) -> f64
        %16 = "lo_spn.log"(%15) : (f64) -> f64
        "lo_spn.yield"(%16) : (f64) -> ()
      }) : (i32, i32) -> f64
      %4 = "lo_spn.batch_collect"(%3, %arg1) : (f64, index) -> tensor<?xf64>
      "lo_spn.return"(%4) : (tensor<?xf64>) -> ()
    }) {batchSize = 10 : ui32} : (tensor<?x2xi32>) -> tensor<?xf64>
    "lo_spn.return"(%0) : (tensor<?xf64>) -> ()
  }) {sym_name = "spn_kernel", type = (tensor<?x2xi32>) -> tensor<?xf64>} : () -> ()
}

// NOTE: Assertions have been autogenerated by utils/generate-test-checks.py


// CHECK-LABEL:   "lo_spn.kernel"() ( {
// CHECK:         ^bb0(%[[VAL_0:.*]]: memref<?x2xi32>, %[[VAL_1:.*]]: memref<?xf64>):
// CHECK:           %[[VAL_2:.*]] = constant 0 : index
// CHECK:           %[[VAL_3:.*]] = dim %[[VAL_0]], %[[VAL_2]] : memref<?x2xi32>
// CHECK:           %[[VAL_4:.*]] = alloc(%[[VAL_3]]) : memref<?xf64>
// CHECK:           "lo_spn.task"(%[[VAL_0]], %[[VAL_4]]) ( {
// CHECK:           ^bb0(%[[VAL_5:.*]]: index, %[[VAL_6:.*]]: memref<?x2xi32>, %[[VAL_7:.*]]: memref<?xf64>):
// CHECK:             %[[VAL_8:.*]] = "lo_spn.batch_read"(%[[VAL_6]], %[[VAL_5]]) {sampleIndex = 0 : ui32} : (memref<?x2xi32>, index) -> i32
// CHECK:             %[[VAL_9:.*]] = "lo_spn.batch_read"(%[[VAL_6]], %[[VAL_5]]) {sampleIndex = 1 : ui32} : (memref<?x2xi32>, index) -> i32
// CHECK:             %[[VAL_10:.*]] = "lo_spn.body"(%[[VAL_8]], %[[VAL_9]]) ( {
// CHECK:             ^bb0(%[[VAL_11:.*]]: i32, %[[VAL_12:.*]]: i32):
// CHECK:               %[[VAL_13:.*]] = "lo_spn.histogram"(%[[VAL_11]]) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 2.500000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 7.500000e-01 : f64}], supportMarginal = false} : (i32) -> f64
// CHECK:               %[[VAL_14:.*]] = "lo_spn.histogram"(%[[VAL_12]]) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 4.500000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 5.500000e-01 : f64}], supportMarginal = false} : (i32) -> f64
// CHECK:               %[[VAL_15:.*]] = "lo_spn.mul"(%[[VAL_13]], %[[VAL_14]]) : (f64, f64) -> f64
// CHECK:               %[[VAL_16:.*]] = "lo_spn.histogram"(%[[VAL_11]]) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 3.300000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 6.700000e-01 : f64}], supportMarginal = false} : (i32) -> f64
// CHECK:               %[[VAL_17:.*]] = "lo_spn.histogram"(%[[VAL_12]]) {bucketCount = 2 : ui32, buckets = [{lb = 0 : i32, ub = 1 : i32, val = 8.750000e-01 : f64}, {lb = 1 : i32, ub = 2 : i32, val = 1.250000e-01 : f64}], supportMarginal = false} : (i32) -> f64
// CHECK:               %[[VAL_18:.*]] = "lo_spn.mul"(%[[VAL_16]], %[[VAL_17]]) : (f64, f64) -> f64
// CHECK:               %[[VAL_19:.*]] = "lo_spn.constant"() {type = f64, value = 3.000000e-01 : f64} : () -> f64
// CHECK:               %[[VAL_20:.*]] = "lo_spn.mul"(%[[VAL_15]], %[[VAL_19]]) : (f64, f64) -> f64
// CHECK:               %[[VAL_21:.*]] = "lo_spn.constant"() {type = f64, value = 0.69999999999999996 : f64} : () -> f64
// CHECK:               %[[VAL_22:.*]] = "lo_spn.mul"(%[[VAL_18]], %[[VAL_21]]) : (f64, f64) -> f64
// CHECK:               %[[VAL_23:.*]] = "lo_spn.add"(%[[VAL_20]], %[[VAL_22]]) : (f64, f64) -> f64
// CHECK:               %[[VAL_24:.*]] = "lo_spn.log"(%[[VAL_23]]) : (f64) -> f64
// CHECK:               "lo_spn.yield"(%[[VAL_24]]) : (f64) -> ()
// CHECK:             }) : (i32, i32) -> f64
// CHECK:             "lo_spn.batch_write"(%[[VAL_25:.*]], %[[VAL_7]], %[[VAL_5]]) : (f64, memref<?xf64>, index) -> ()
// CHECK:             "lo_spn.return"() : () -> ()
// CHECK:           }) {batchSize = 10 : ui32} : (memref<?x2xi32>, memref<?xf64>) -> ()
// CHECK:           %[[VAL_26:.*]] = tensor_load %[[VAL_4]] : memref<?xf64>
// CHECK:           %[[VAL_27:.*]] = tensor_to_memref %[[VAL_26]] : memref<?xf64>
// CHECK:           "lo_spn.copy"(%[[VAL_27]], %[[VAL_1]]) : (memref<?xf64>, memref<?xf64>) -> ()
// CHECK:           "lo_spn.return"() : () -> ()
// CHECK:         }) {sym_name = "spn_kernel", type = (memref<?x2xi32>, memref<?xf64>) -> ()} : () -> ()
