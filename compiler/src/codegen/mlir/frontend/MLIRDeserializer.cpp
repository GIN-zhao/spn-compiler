//==============================================================================
// This file is part of the SPNC project under the Apache License v2.0 by the
// Embedded Systems and Applications Group, TU Darmstadt.
// For the full copyright and license information, please view the LICENSE
// file that was distributed with this source code.
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

#include "MLIRDeserializer.h"
#include "xspn/xspn/serialization/binary/capnproto/spflow.capnp.h"
#include "capnp/serialize.h"
#include <fcntl.h>
#include <unistd.h>
#include "util/Logging.h"
#include <string>
#include <regex>
#include <mlir/IR/Verifier.h>
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "llvm/Support/Debug.h"
#include "Kernel.h"
#include <driver/GlobalOptions.h>

using namespace capnp;
using namespace mlir;

spnc::MLIRDeserializer::BinaryFileHandler::BinaryFileHandler(const std::string& fileName) {
  fd = open(fileName.c_str(), O_RDONLY);
}

spnc::MLIRDeserializer::BinaryFileHandler::~BinaryFileHandler() noexcept {
  close(fd);
}

spnc::MLIRDeserializer::MLIRDeserializer(BinarySPN _inputFile, std::shared_ptr<::mlir::MLIRContext> _context,
                                         std::shared_ptr<KernelInfo> info) :
    kernelInfo{std::move(info)}, context{std::move(_context)}, builder{context.get()}, 
    inputFile{std::move(_inputFile)} {
  module = std::make_unique<mlir::ModuleOp>(mlir::ModuleOp::create(builder.getUnknownLoc()));
}

mlir::ModuleOp& spnc::MLIRDeserializer::execute() {

  if (!cached) {
    BinaryFileHandler fileHandler{inputFile.fileName()};
    StreamFdMessageReader message{fileHandler.getFileDescriptor()};

    auto header = message.getRoot<Header>();

    if (header.isModel()) {
      SPNC_FATAL_ERROR("Cannot compile raw models");
    }

    deserializeQuery(header.getQuery());

    if (spnc::option::dumpIR.get(*this->config)) {
      llvm::dbgs() << "\n// *** IR after deserialization ***\n";
      module->dump();
    }
    if (failed(::mlir::verify(module->getOperation()))) {
      SPNC_FATAL_ERROR("Verification of the generated MLIR module failed!");
    }
    cached = true;
  }

  return *module;
}

void spnc::MLIRDeserializer::deserializeQuery(Query::Reader&& query) {
  int batchSize = query.getBatchSize();
  if (!query.hasJoint()) {
    SPNC_FATAL_ERROR("Can only deserialize joint queries");
  }
  auto errorKind =
      (query.getErrorKind() == ErrorKind::ABSOLUTE) ? mlir::spn::high::error_model::absolute_error
                                                    : mlir::spn::high::error_model::relative_error;
  deserializeJointQuery(query.getJoint(), batchSize, errorKind, query.getMaxError());
}

void spnc::MLIRDeserializer::deserializeJointQuery(JointProbability::Reader&& query, int batchSize,
                                                   mlir::spn::high::error_model errorKind, double maxError) {
  if (!query.hasModel()) {
    SPNC_FATAL_ERROR("No model attached to query");
  }
  builder.setInsertionPointToStart(module->getBody());
  auto numFeatures = query.getModel().getNumFeatures();
  auto numFeaturesAttr = builder.getUI32IntegerAttr(numFeatures);
  auto featureType = translateTypeString(query.getModel().getFeatureType());
  auto featureTypeAttr = TypeAttr::get(featureType);
  std::string modelName = query.getModel().getName();
  if (modelName.length() == 0) {
    modelName = "spn_kernel";
  }
  auto kernelNameAttr = builder.getStringAttr(modelName);
  auto batchSizeAttr = builder.getUI32IntegerAttr(batchSize);

  // Store information about the kernel for use in later stages of the toolchain.
  kernelInfo->kernelName = modelName;
  kernelInfo->batchSize = batchSize;
  kernelInfo->queryType = spnc::KernelQueryType::JOINT_QUERY;
  kernelInfo->numFeatures = numFeatures;
  kernelInfo->bytesPerFeature = sizeInByte(featureType);

  auto queryOp =
      builder.create<mlir::spn::high::JointQuery>(builder.getUnknownLoc(), numFeaturesAttr,
                                                  featureTypeAttr, kernelNameAttr, batchSizeAttr,
                                                  builder.getI32IntegerAttr(static_cast<int32_t>(errorKind)),
                                                  builder.getF64FloatAttr(maxError),
                                                  builder.getBoolAttr(query.getSupportMarginal()));
  // Insertion is automatically set to beginning of new block.
  (void) builder.createBlock(&queryOp.getRegion());

  deserializeModel(query.getModel());
}

void spnc::MLIRDeserializer::deserializeModel(Model::Reader&& model) {
  // Construct the graph holding the actual operations of the DAG.
  auto graph = builder.create<mlir::spn::high::Graph>(builder.getUnknownLoc(),
                                                      builder.getUI32IntegerAttr(model.getNumFeatures()));
  // Create the single block inside that graph.
  auto block = builder.createBlock(&graph.getRegion());

  // Sort scope in ascending order and construct a block argument for each variable (element of the scope).
  SmallVector<int, 10> scope;
  for (auto s : model.getScope()) {
    scope.push_back(s);
  }
  std::sort(scope.begin(), scope.end());
  auto featureType = translateTypeString(model.getFeatureType());
  for (auto s : scope) {
    // Add mapping from input (scope) to block argument.
    inputs[s] = graph.getRegion().addArgument(featureType);
  }
  builder.setInsertionPointToEnd(block);

  for (auto node : model.getNodes()) {
    deserializeNode(node);
  }

  // Insert the RootNode to mark the root of the DAG.
  auto resultValue = getValueForNode(model.getRootNode());
  builder.create<mlir::spn::high::RootNode>(builder.getUnknownLoc(), resultValue);
}

void spnc::MLIRDeserializer::deserializeNode(Node::Reader& node) {
  Value op;
  switch (node.which()) {
    case Node::SUM: op = deserializeSum(node.getSum());
      break;
    case Node::PRODUCT: op = deserializeProduct(node.getProduct());
      break;
    case Node::HIST: op = deserializeHistogram(node.getHist());
      break;
    case Node::GAUSSIAN: op = deserializeGaussian(node.getGaussian());
      break;
    case Node::CATEGORICAL: op = deserializeCaterogical(node.getCategorical());
      break;
    default: SPNC_FATAL_ERROR("Unsupported node type ", node.toString().flatten().cStr());
  }
  // Add mapping from unique node ID to operation/value.
  assert(op);
  node2value[node.getId()] = op;
}

mlir::spn::high::SumNode spnc::MLIRDeserializer::deserializeSum(SumNode::Reader&& sum) {
  llvm::SmallVector<Value, 10> ops;
  for (auto a : sum.getChildren()) {
    ops.push_back(getValueForNode(a));
  }
  llvm::SmallVector<double, 10> weights;
  for (auto w : sum.getWeights()) {
    weights.push_back(w);
  }
  return builder.create<mlir::spn::high::SumNode>(builder.getUnknownLoc(), ops, weights);
}

mlir::spn::high::ProductNode spnc::MLIRDeserializer::deserializeProduct(ProductNode::Reader&& product) {
  llvm::SmallVector<Value, 10> ops;
  for (auto p : product.getChildren()) {
    ops.push_back(getValueForNode(p));
  }
  return builder.create<mlir::spn::high::ProductNode>(builder.getUnknownLoc(), ops);
}

mlir::spn::high::HistogramNode spnc::MLIRDeserializer::deserializeHistogram(HistogramLeaf::Reader&& histogram) {
  Value indexVar = getInputValueByIndex(histogram.getScope());
  auto breaks = histogram.getBreaks();
  auto densities = histogram.getDensities();
  SmallVector<bucket_t, 256> buckets;
  // Construct histogram from breaks and densities.
  for (unsigned i = 0; i < breaks.size() - 1; ++i) {
    auto lb = breaks[i];
    auto ub = breaks[i + 1];
    auto d = densities[i];
    buckets.push_back(std::tie(lb, ub, d));
  }
  return builder.create<mlir::spn::high::HistogramNode>(builder.getUnknownLoc(), indexVar, buckets);
}

mlir::spn::high::CategoricalNode spnc::MLIRDeserializer::deserializeCaterogical(CategoricalLeaf::Reader&& categorical) {
  auto indexVar = getInputValueByIndex(categorical.getScope());
  SmallVector<double, 10> probabilities;
  for (auto p : categorical.getProbabilities()) {
    probabilities.push_back(p);
  }
  return builder.create<mlir::spn::high::CategoricalNode>(builder.getUnknownLoc(), indexVar, probabilities);
}

mlir::spn::high::GaussianNode spnc::MLIRDeserializer::deserializeGaussian(GaussianLeaf::Reader&& gaussian) {
  auto indexVar = getInputValueByIndex(gaussian.getScope());
  return builder.create<mlir::spn::high::GaussianNode>(builder.getUnknownLoc(), indexVar,
                                                       gaussian.getMean(), gaussian.getStddev());
}

mlir::Value spnc::MLIRDeserializer::getInputValueByIndex(int index) {
  if (!inputs.count(index)) {
    SPNC_FATAL_ERROR("Leaf node references unknown feature!")
  }
  return inputs[index];
}

mlir::Value spnc::MLIRDeserializer::getValueForNode(int id) {
  if (!node2value.count(id)) {
    SPNC_FATAL_ERROR("No definition found for node with ID: ", id);
  }
  return node2value[id];
}

mlir::Type spnc::MLIRDeserializer::translateTypeString(const std::string& text) {
  std::smatch match;
  // Test for an integer type, given as [u]int(WIDTH).
  std::regex intRegex{R"(([u]?)int([1-9]+))"};
  if (std::regex_match(text, match, intRegex)) {
    // match[2] captures the width of the type.
    auto width = std::stoi(match[2]);
    return IntegerType::get(context.get(), width);
  }
  // Test for a floating-point type, given as float(WIDTH).
  std::regex floatRegex{R"(float([1-9]+))"};
  if (std::regex_match(text, match, floatRegex)) {
    // match[1] captures the width of the type.
    auto width = std::stoi(match[1]);
    switch (width) {
      case 16: return builder.getF16Type();
      case 32: return builder.getF32Type();
      case 64: return builder.getF64Type();
      default: SPNC_FATAL_ERROR("Unsupported floating-point type ", text);
    }
  }
  SPNC_FATAL_ERROR("Unsupported feature data type ", text);
}

unsigned spnc::MLIRDeserializer::sizeInByte(mlir::Type type) {
  if (auto intType = type.dyn_cast<IntegerType>()) {
    return intType.getWidth() >> 3;
  }
  if (auto floatType = type.dyn_cast<FloatType>()) {
    return floatType.getWidth() >> 3;
  }
  SPNC_FATAL_ERROR("Unsupported feature data type");
}