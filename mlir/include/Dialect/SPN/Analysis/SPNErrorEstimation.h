//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#ifndef SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SPNERRORESTIMATION_H
#define SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SPNERRORESTIMATION_H

#include <map>
#include <memory>
#include <tuple>
#include <mlir/IR/Module.h>
#include <mlir/IR/Types.h>
#include <mlir/IR/StandardTypes.h>
#include "SPN/SPNOps.h"

typedef std::shared_ptr<void> arg_t;

namespace mlir {
  namespace spn {

    enum class data_representation { EM_FIXED_POINT, EM_FLOATING_POINT };

    /// Class to walk over a (sub-)graph, estimating error margins in the process and yielding a suitable data type.
    /// The needed parameters (error margin, error model, data representation) will be extracted from the SPN query.
    /// This allows this class to be constructed with only a single parameter, the corresponding operation pointer.
    /// Hence, it can be used as an analysis.
    /// Note: Currently, the data representation is fixed to: floating point.
    class SPNErrorEstimation {

    public:

      /// Constructor. Note that this will be primarily used as an analysis.
      /// \param root Operation pointer which will be treated as SPN-graph root.
      explicit SPNErrorEstimation(Operation* root);

      /// Retrieve the smallest type of the corresponding error model which can represent the SPN result within the
      /// given error margin -OR- if not possible: the biggest type available (e.g. Float64Type).
      /// \return mlir::Type "Optimal" type to represent the given SPN's value(s).
      Type getOptimalType();

    private:

      /// Process provided pointer to a SPN node and update internal counts / results.
      /// \param graphRoot Pointer to the defining operation, representing a SPN node.
      void analyzeGraph(Operation* graphRoot);

      /// Process provided pointer to a SPN node and update internal counts / results.
      /// \param subgraphRoot Pointer to the defining operation, representing a SPN node.
      void traverseSubgraph(Operation* subgraphRoot);

      /// Process minimum and maximum node values to estimate the least amount of magnitude (Integer / Exponent) bits.
      void estimateLeastMagnitudeBits();

      /// Check the error requirements .
      /// \return Boolean true if the selected type can represent the SPN's value(s) within the error margin.
      ///                 Otherwise: false.
      bool checkRequirements();

      /// Estimate the error introduced by the given addition operation w.r.t. the current format.
      /// Since the error-estimation needs the "real" value (i.e. exact / accurate), it has to be determined, too.
      /// \param op Pointer to the defining operation, representing a SPN node.
      void estimateErrorSum(SumOp op);

      /// Estimate the error introduced by the given multiplication operation w.r.t. the current format.
      /// Since the error-estimation needs the "real" value (i.e. exact / accurate), it has to be determined, too.
      /// \param op Pointer to the defining operation, representing a SPN node.
      void estimateErrorProduct(ProductOp op);

      /// Estimate the error introduced by the given categrical node w.r.t. the current format.
      /// Since the error-estimation needs the "real" value (i.e. exact / accurate), it has to be determined, too.
      /// \param op Pointer to the defining operation, representing a SPN node.
      void estimateErrorCategorical(CategoricalOp op);

      /// Estimate the error introduced by the given constant w.r.t. the current format.
      /// Since the error-estimation needs the "real" value (i.e. exact / accurate), it has to be determined, too.
      /// \param op Pointer to the defining operation, representing a SPN node.
      void estimateErrorConstant(ConstantOp op);

      /// Estimate the error introduced by the given gaussian node w.r.t. the current format.
      /// Since the error-estimation needs the "real" value (i.e. exact / accurate), it has to be determined, too.
      /// \param op Pointer to the defining operation, representing a SPN node.
      void estimateErrorGaussian(GaussianOp op);

      /// Estimate the error introduced by the given leaf w.r.t. the current format.
      /// Since the error-estimation needs the "real" value (i.e. exact / accurate), it has to be determined, too.
      /// \param op Pointer to the defining operation, representing a SPN node.
      void estimateErrorHistogram(HistogramOp op);

      /// Select the "optimal" type, using the current state of the analysis.
      void selectOptimalType();

      /// SPN root-node.
      Operation* rootNode;

      /// User-requested error model.
      enum data_representation est_data_representation;

      /// User-requested error model / calculation.
      enum error_model est_error_model;

      /// User-requested maximum error margin.
      double error_margin;

      /// Indicate if min-max information was processed.
      int iterationCount = 0;

      /// Indicate if min-max information was processed.
      bool estimatedLeastMagnitudeBits = false;

      /// Indicate if requirements are satisfied.
      bool satisfiedRequirements = false;

      /// Indicate if analysis should be aborted.
      bool abortAnalysis = false;

      /// This is the selected "optimal" mlir::Type.
      Type selectedType;

      /// Number of available floating point formats.
      static const int NUM_FLOAT_FORMATS = 48;

      /// Calculation constant "two".
      const double BASE_TWO = 2.0;

      /// Calculation EPSILON.
      double EPS = 0.0;

      /// Calculation error coefficient: 1 + EPSILON.
      double ERR_COEFFICIENT = 1.0;

      /// Calculation of Gaussian minimum probability value outside of 99% := exp(-0.5 * std::pow(2.575829303549,2.0)).
      const double GAUSS_99 = 0.036245200715160;

      /// For each operation store values: { accurate, defective, max, min, max_subtree_depth }
      std::map<mlir::Operation*, std::tuple<double, double, double, double, int>> spn_node_values;

      /// The global extreme-values of the SPN are used when determining the needed I(nteger) / E(xponent) values
      double spn_node_value_global_maximum = std::numeric_limits<double>::min();
      double spn_node_value_global_minimum = std::numeric_limits<double>::max();

      /// Current format information: "Fraction / Mantissa"
      int format_bits_significance = std::numeric_limits<int>::min();
      /// Current format information: "Integer / Exponent"
      int format_bits_magnitude = std::numeric_limits<int>::min();

      /// Tuples which model different floating point formats, like e.g. fp32 / single precision
      /// 0: Significance / "Fraction / Mantissa"
      /// 1: Magnitude / "Integer / Exponent"
      /// 2: function returning mlir::Type, providing an MLIRContext*
      std::tuple<int, int, std::function<Type(MLIRContext*)>> Float_Formats[NUM_FLOAT_FORMATS] = {
          {10, 5, Float16Type::get},
          {7, 8, BFloat16Type::get},
          {23, 8, Float32Type::get},
          {52, 11, Float64Type::get}
      };

    };

  }

}

#endif //SPNC_MLIR_DIALECTS_INCLUDE_DIALECT_SPN_ANALYSIS_SPNERRORESTIMATION_H