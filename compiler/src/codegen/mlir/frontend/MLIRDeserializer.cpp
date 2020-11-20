//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#include "MLIRDeserializer.h"
#include "xspn/xspn/serialization/binary/capnproto/spflow.capnp.h"
#include "capnp/serialize.h"
#include <fcntl.h>
#include <unistd.h>
#include "util/Logging.h"
#include <string>
#include <regex>
#include <mlir/IR/Verifier.h>
#include "mlir/IR/StandardTypes.h"

using namespace capnp;
using namespace mlir;
using namespace mlir::spn;

spnc::MLIRDeserializer::BinaryFileHandler::BinaryFileHandler(const std::string& fileName) {
  fd = open(fileName.c_str(), O_RDONLY);
}

spnc::MLIRDeserializer::BinaryFileHandler::~BinaryFileHandler() noexcept {
  close(fd);
}

spnc::MLIRDeserializer::MLIRDeserializer(BinarySPN _inputFile, std::shared_ptr<::mlir::MLIRContext> _context) :
    context{std::move(_context)}, builder{context.get()}, inputFile{std::move(_inputFile)} {
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

    module->dump();
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
  deserializeJointQuery(query.getJoint(), batchSize);
}

void spnc::MLIRDeserializer::deserializeJointQuery(JointProbability::Reader&& query, int batchSize) {
  if (!query.hasModel()) {
    SPNC_FATAL_ERROR("No model attached to query");
  }
  builder.setInsertionPointToStart(module->getBody());
  auto numFeatures = query.getModel().getNumFeatures();
  std::cout << "Number of features: " << numFeatures << std::endl;
  auto numFeaturesAttr = builder.getUI32IntegerAttr(numFeatures);
  auto featureType = translateTypeString(query.getModel().getFeatureType());
  auto featureTypeAttr = TypeAttr::get(featureType);
  std::string modelName = query.getModel().getName();
  if (modelName.length() == 0) {
    modelName = "spn_kernel";
  }
  auto kernelNameAttr = builder.getStringAttr(modelName);
  // TODO Attach information about allowed relative error to query.
  auto queryOp =
      builder.create<SingleJointQuery>(builder.getUnknownLoc(), numFeaturesAttr, featureTypeAttr, kernelNameAttr);
  auto block = builder.createBlock(&queryOp.getRegion());
  inputs.resize(numFeatures);
  for (int i = 0; i < numFeatures; ++i) {
    inputs[i] = queryOp.getRegion().addArgument(featureType);
  }
  builder.setInsertionPointToEnd(block);
  deserializeModel(query.getModel());
  auto resultValue = getValueForNode(query.getModel().getRootNode());
  builder.create<mlir::spn::ReturnOp>(builder.getUnknownLoc(), resultValue);
}

void spnc::MLIRDeserializer::deserializeModel(Model::Reader&& model) {
  for (auto node : model.getNodes()) {
    deserializeNode(node);
  }
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
    case Node::GAUSSIAN: SPNC_FATAL_ERROR("Gaussian leaf nodes not yet supported");
    case Node::CATEGORICAL: SPNC_FATAL_ERROR("Categorical leaf nodes not yet supported");
    default: SPNC_FATAL_ERROR("Unsupported node type ", node.toString().flatten().cStr());
  }
  // Add mapping from unique node ID to operation/value.
  node2value[node.getId()] = op;
}

mlir::spn::WeightedSumOp spnc::MLIRDeserializer::deserializeSum(SumNode::Reader&& sum) {
  llvm::SmallVector<Value, 10> ops;
  for (auto a : sum.getChildren()) {
    ops.push_back(getValueForNode(a));
  }
  llvm::SmallVector<double, 10> weights;
  for (auto w : sum.getWeights()) {
    weights.push_back(w);
  }
  return builder.create<WeightedSumOp>(builder.getUnknownLoc(), ops, weights);
}

mlir::spn::ProductOp spnc::MLIRDeserializer::deserializeProduct(ProductNode::Reader&& product) {
  llvm::SmallVector<Value, 10> ops;
  for (auto p : product.getChildren()) {
    ops.push_back(getValueForNode(p));
  }
  return builder.create<ProductOp>(builder.getUnknownLoc(), ops);
}

mlir::spn::HistogramOp spnc::MLIRDeserializer::deserializeHistogram(HistogramLeaf::Reader&& histogram) {
  if (!inputs.inBounds(histogram.getScope() - 1)) {
    SPNC_FATAL_ERROR("Histograms references unknown feature!")
  }
  auto indexVar = inputs[histogram.getScope() - 1];
  auto breaks = histogram.getBreaks();
  auto densities = histogram.getDensities();
  SmallVector<bucket_t, 256> buckets;
  // Construct histogram from breaks and densities.
  for (int i = 0; i < breaks.size() - 1; ++i) {
    auto lb = breaks[i];
    auto ub = breaks[i + 1];
    auto d = densities[i];
    buckets.push_back(std::tie(lb, ub, d));
  }
  return builder.create<HistogramOp>(builder.getUnknownLoc(), indexVar, buckets);
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
    // match[1] captures an "u" if the type is unsigned, e.g. "uint8".
    auto isUnsigned = match[1].length() != 0;
    // match[2] captures the width of the type.
    auto width = std::stoi(match[2]);
    return builder.getIntegerType(width, !isUnsigned);
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
