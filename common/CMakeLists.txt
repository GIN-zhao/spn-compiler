# ==============================================================================
#  This file is part of the SPNC project under the Apache License v2.0 by the
#  Embedded Systems and Applications Group, TU Darmstadt.
#  For the full copyright and license information, please view the LICENSE
#  file that was distributed with this source code.
#  SPDX-License-Identifier: Apache-2.0
# ==============================================================================

add_library(spnc-common INTERFACE)

target_include_directories(spnc-common
        INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

doxygen_doc(TARGET_NAME spnc-common SRC_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)