//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#ifndef SPNC_KERNEL_H
#define SPNC_KERNEL_H

#include <optional>
#include <cstdlib>

///
/// Namespace for all entities related to the SPN compiler.
///
namespace spnc {

  enum KernelQueryType : unsigned { JOINT_QUERY = 1 };

  enum KernelTarget : unsigned { CPU = 1, CUDA = 2 };

  ///
  /// Represents a kernel that is generated by the compiler and can be loaded and executed by the runtime.
  /// Contains information about the file-path of the generated kernel and the name of the generated
  /// function inside that file.
  ///
  class Kernel {

  public:

    ///
    /// Constructor.
    /// \param fN The full path to the kernel file (shared object).
    /// \param kN The full name of the toplevel SPN function to be called by the runtime.
    /// \param query_type Type of the query compiled into the kernel.
    /// \param _batchSize Batch size the kernel was optimized for.
    Kernel(const std::string& fN, const std::string& kN,
           unsigned query_type,
           unsigned target,
           unsigned _batchSize,
           unsigned _numFeatures, unsigned _bytesPerFeatures,
           unsigned numResults, unsigned bytesPerResult,
           const std::string& dataType) : _fileName{fN},
                                          _kernelName{kN},
                                          query{query_type},
                                          targetArch{target},
                                          batch_size{_batchSize},
                                          num_features{_numFeatures},
                                          bytes_per_feature{_bytesPerFeatures},
                                          num_results{numResults},
                                          bytes_per_result{bytesPerResult},
                                          dtype{dataType} {
      _unique_id = std::hash<std::string>{}(fN + kN);
    }

    /// Get the full file-path for this Kernel.
    /// \return The full file-path.
    std::string fileName() const { return std::string{_fileName}; }

    /// Get the name of the function to call.
    /// \return Name of the SPN function.
    const std::string& kernelName() const { return _kernelName; }

    /// Get the unique ID of this Kernel used to identify it.
    /// \return An integer value containing a unique ID.
    size_t unique_id() const { return _unique_id; }

    /// Get the type of query compiled into this kernel.
    /// \return [KernelQueryType] of the kernel.
    unsigned queryType() const { return query; }

    /// Get the target for which this kernel was compiled.
    /// \return [KernelTarget] of the kernel.
    unsigned target() const { return targetArch; }

    /// Get the batch size this kernel was optimized for.
    /// \return Optimized batch size, 1 if the kernel was optimized for single execution.
    unsigned batchSize() const { return batch_size; }

    /// Get the number of features of the SPN compiled into this kernel.
    /// \return Number of input features of the SPN.
    unsigned numFeatures() const { return num_features; }

    /// Get the number of bytes used to encode a single input feature value.
    /// \return Number of bytes used to encode each SPN input.
    unsigned bytesPerFeature() const { return bytes_per_feature; }

    /// Get the number of results returned per sample from this kernel.
    /// \return Number of result values returned per sample.
    unsigned numResults() const { return num_results; }

    /// Get the number of bytes used to encode each result value.
    /// \return Number of bytes used to encode a single result value.
    unsigned bytesPerResult() const { return bytes_per_result; }

    /// Get the data-type of the result values.
    /// \return Data-type of each result value.
    const std::string& dataType() const { return dtype; }

  private:

    std::string _fileName;

    std::string _kernelName;

    size_t _unique_id;

    unsigned query;

    unsigned targetArch;

    unsigned batch_size;

    unsigned num_features;

    unsigned bytes_per_feature;

    unsigned num_results;

    unsigned bytes_per_result;

    std::string dtype;

  };

}

#endif //SPNC_KERNEL_H
