// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include "Open3D/Core/Kernel/UnaryEW.h"

#include "Open3D/Core/CUDAState.cuh"
#include "Open3D/Core/CUDAUtils.h"
#include "Open3D/Core/Dispatch.h"
#include "Open3D/Core/Kernel/CUDALauncher.cuh"
#include "Open3D/Core/Tensor.h"

namespace open3d {
namespace kernel {

template <typename src_t, typename dst_t>
static OPEN3D_HOST_DEVICE void CUDACopyElementKernel(const void* src,
                                                     void* dst) {
    *static_cast<dst_t*>(dst) =
            static_cast<dst_t>(*static_cast<const src_t*>(src));
}

void CopyCUDA(const Tensor& src, Tensor& dst) {
    // It has been checked that
    // - src and dst have the same dtype
    // - at least one of src or dst is CUDA device
    SizeVector shape = src.GetShape();
    Dtype src_dtype = src.GetDtype();
    Dtype dst_dtype = dst.GetDtype();

    Device src_device = src.GetDevice();
    Device dst_device = dst.GetDevice();

    if (src_device.GetType() == Device::DeviceType::CUDA &&
        dst_device.GetType() == Device::DeviceType::CUDA) {
        if (src.IsContiguous() && dst.IsContiguous() &&
            src.GetShape() == dst.GetShape() && src_dtype == dst_dtype) {
            // MemoryManager handles p2p and non-p2p device copy.
            MemoryManager::Memcpy(
                    dst.GetDataPtr(), dst_device, src.GetDataPtr(), src_device,
                    DtypeUtil::ByteSize(src_dtype) * shape.NumElements());
        } else if (src_device == dst_device) {
            // For more optimized version, one can check if P2P from src to dst
            // is enabled, then put synchronization with streams on both src and
            // dst to wait for copy kernel to complete.
            CUDADeviceSwitcher switcher(src_device);
            Indexer indexer({src}, dst, DtypePolicy::NONE);
            DISPATCH_DTYPE_TO_TEMPLATE(src_dtype, [&]() {
                using src_t = scalar_t;
                DISPATCH_DTYPE_TO_TEMPLATE(dst_dtype, [&]() {
                    using dst_t = scalar_t;
                    CUDALauncher::LaunchUnaryEWKernel(
                            indexer,
                            // Need to wrap as extended CUDA lamba function
                            [] OPEN3D_HOST_DEVICE(const void* src, void* dst) {
                                CUDACopyElementKernel<src_t, dst_t>(src, dst);
                            });
                });
            });
        } else {
            dst.CopyFrom(src.Contiguous().Copy(dst_device));
        }
    } else if (src_device.GetType() == Device::DeviceType::CPU &&
                       dst_device.GetType() == Device::DeviceType::CUDA ||
               src_device.GetType() == Device::DeviceType::CUDA &&
                       dst_device.GetType() == Device::DeviceType::CPU) {
        Tensor src_conti = src.Contiguous();  // No op if already contiguous
        if (dst.IsContiguous() && src.GetShape() == dst.GetShape() &&
            src_dtype == dst_dtype) {
            MemoryManager::Memcpy(
                    dst.GetDataPtr(), dst_device, src_conti.GetDataPtr(),
                    src_conti.GetDevice(),
                    DtypeUtil::ByteSize(src_dtype) * shape.NumElements());
        } else {
            dst.CopyFrom(src.Contiguous().Copy(dst_device));
        }
    } else {
        utility::LogError("Wrong device type {} -> {}", src_device.ToString(),
                          dst_device.ToString());
    }
}

}  // namespace kernel
}  // namespace open3d
