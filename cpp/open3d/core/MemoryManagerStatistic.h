// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018-2021 www.open3d.org
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

#pragma once

#include <cstddef>
#include <map>
#include <mutex>

#include "open3d/core/Device.h"

namespace open3d {
namespace core {

class MemoryManagerStatistic {
public:
    enum class PrintLevel {
        /// Statistics for all used devices are printed.
        All = 0,
        /// Only devices with unbalanced counts are printed.
        /// This is typically an indicator for memory leaks.
        Unbalanced = 1,
        /// No statistics are printed.
        None = 2,
    };

    static MemoryManagerStatistic& getInstance();

    MemoryManagerStatistic(const MemoryManagerStatistic&) = delete;
    MemoryManagerStatistic& operator=(MemoryManagerStatistic&) = delete;

    /// Always print the statistics at the end of the program.
    ~MemoryManagerStatistic() { Print(); }

    void setPrintLevel(PrintLevel level);
    void Print() const;

    void IncrementCountMalloc(const Device& device);
    void IncrementCountFree(const Device& device);

private:
    MemoryManagerStatistic() = default;

    struct MemoryStatistics {
        size_t count_malloc_ = 0;
        size_t count_free_ = 0;
    };

    struct DeviceComparator {
        bool operator()(const Device& lhs, const Device& rhs) const {
            return lhs.ToString() < rhs.ToString();
        }
    };

    /// Only print unbalanced statistics by default.
    PrintLevel level_ = PrintLevel::Unbalanced;

    std::mutex statistics_mutex_;
    std::map<Device, MemoryStatistics, DeviceComparator> statistics_;
};

}  // namespace core
}  // namespace open3d
