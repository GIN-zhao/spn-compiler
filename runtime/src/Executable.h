//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#ifndef SPNC_EXECUTABLE_H
#define SPNC_EXECUTABLE_H

#include <cstdlib>
#include <Kernel.h>

using namespace spnc;

namespace spnc_rt {

  typedef void (* single_kernel)(void* input_ptr,
                                 void* aligned_input_ptr,
                                 int input_offset,
                                 int input_size_dim1,
                                 int input_stride_dim1,
                                 int input_size_dim2,
                                 int input_stride_dim2,
                                 double* output_ptr,
                                 double* output_aligned_ptr,
                                 int output_offset,
                                 int output_size,
                                 int output_stride);

  typedef void (* batch_kernel)(void* input_ptr,
                                void* aligned_input_ptr,
                                int input_offset,
                                int input_size_dim1,
                                int input_stride_dim1,
                                int input_size_dim2,
                                int input_stride_dim2,
                                double* output_ptr,
                                double* output_aligned_ptr,
                                int output_offset,
                                int output_size,
                                int output_stride);

  union kernel_function {
    single_kernel single;
    batch_kernel batch;
  };

  ///
  /// Manages a Kernel by loading it from the shared object using libelf.
  class Executable {

  public:

    /// Constructor.
    /// \param kernel Kernel to load and eventually execute.
    explicit Executable(const Kernel& kernel);

    Executable(const Executable&) = delete;

    Executable& operator=(const Executable&) = delete;

    /// Move constructor.
    /// \param other Move source.
    Executable(Executable&& other) noexcept;

    /// Move assignment operator.
    /// \param other Move source.
    /// \return Reference to move target.
    Executable& operator=(Executable&& other) noexcept;

    ~Executable();

    /// Execute the Kernel.
    /// \param num_elements Number of queries in the batch.
    /// \param inputs Input SPN evidence.
    /// \param outputs SPN output probabilities.
    void execute(size_t num_elements, void* inputs, double* outputs);

  private:
    const Kernel* kernel;

    void* handle;

    kernel_function kernel_func;

    void initialize();

    void executeSingle(size_t num_samples, void* inputs, double* outputs);

    void executeBatch(size_t num_samples, void* inputs, double* outputs);

  };

}

#endif //SPNC_EXECUTABLE_H
