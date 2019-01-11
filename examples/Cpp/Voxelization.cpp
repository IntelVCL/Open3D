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

#include <Core/Core.h>
#include <IO/IO.h>
#include <Visualization/Visualization.h>

using namespace open3d;

int main(int argc, char **args) {

    using namespace open3d;

    SetVerbosityLevel(VerbosityLevel::VerboseAlways);
    if (argc < 3) {
        PrintOpen3DVersion();
        PrintInfo("Usage:\n");
        PrintInfo("    > Voxelization [pointcloud_filename] [voxel_filename_ply]\n");
        return 1;
    }

    auto pcd = CreatePointCloudFromFile(args[1]);
    auto voxel = CreateVoxelGridFromPointCloud(*pcd, 0.05);
    DrawGeometries({voxel});
    WriteVoxelGrid(args[2], *voxel);
    auto voxel_read = CreateVoxelGridFromFile(args[2]);
    DrawGeometries({voxel_read});
}