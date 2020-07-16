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

#include "open3d/tgeometry/PointCloud.h"
#include "open3d/core/TensorList.h"

#include "core/CoreTest.h"
#include "tests/UnitTest.h"

namespace open3d {
namespace tests {

class PointCloudPermuteDevices : public PermuteDevices {};
INSTANTIATE_TEST_SUITE_P(PointCloud,
                         PointCloudPermuteDevices,
                         testing::ValuesIn(PermuteDevices::TestCases()));

class PointCloudPermuteDevicePairs : public PermuteDevicePairs {};
INSTANTIATE_TEST_SUITE_P(
        PointCloud,
        PointCloudPermuteDevicePairs,
        testing::ValuesIn(PointCloudPermuteDevicePairs::TestCases()));

TEST_P(PointCloudPermuteDevices, DefaultConstructor) {
    tgeometry::PointCloud pcd;

    // Inherited from Geometry3D.
    EXPECT_EQ(pcd.GetGeometryType(),
              tgeometry::Geometry::GeometryType::PointCloud);
    EXPECT_EQ(pcd.Dimension(), 3);

    // Public members.
    EXPECT_TRUE(pcd.IsEmpty());
    EXPECT_FALSE(pcd.HasPoints());
    EXPECT_FALSE(pcd.HasColors());
    EXPECT_FALSE(pcd.HasNormals());
}

TEST_P(PointCloudPermuteDevices, ConstructFromPoints) {
    core::Device device = GetParam();
    core::Dtype dtype = core::Dtype::Float32;
    core::Tensor t = core::Tensor::Ones({10, 3}, dtype, device);
    core::Tensor single_point = core::Tensor::Ones({3}, dtype, device);

    // Copied
    core::TensorList points =
            core::TensorList::FromTensor(t, /*inplace=*/false);
    tgeometry::PointCloud pcd(points);
    EXPECT_TRUE(pcd.HasPoints());
    EXPECT_EQ(pcd["points"].GetSize(), 10);
    pcd["points"].PushBack(single_point);
    EXPECT_EQ(pcd["points"].GetSize(), 11);

    // Inplace tensorlist: cannot push_back
    points = core::TensorList::FromTensor(t, /*inplace=*/true);
    pcd = tgeometry::PointCloud(points);
    EXPECT_TRUE(pcd.HasPoints());
    EXPECT_EQ(pcd["points"].GetSize(), 10);
    EXPECT_ANY_THROW(pcd["points"].PushBack(single_point));
}

TEST_P(PointCloudPermuteDevices, ConstructFromPointDict) {
    core::Device device = GetParam();
    core::Dtype dtype = core::Dtype::Float32;

    core::TensorList points = core::TensorList::FromTensor(
            core::Tensor::Ones({10, 3}, dtype, device));
    core::TensorList colors = core::TensorList::FromTensor(
            core::Tensor::Ones({10, 3}, dtype, device) * 0.5);
    core::TensorList normals = core::TensorList::FromTensor(
            core::Tensor::Ones({10, 3}, dtype, device) * 0.25);
    std::unordered_map<std::string, core::TensorList> point_dict{
            {"points", points},
            {"colors", colors},
            {"normals", normals},
    };

    tgeometry::PointCloud pcd(point_dict);
    EXPECT_TRUE(pcd.HasPoints());
    EXPECT_TRUE(pcd.HasColors());
    EXPECT_TRUE(pcd.HasNormals());

    EXPECT_TRUE(pcd["points"].AsTensor().AllClose(
            core::Tensor::Ones({10, 3}, dtype, device)));
    EXPECT_TRUE(pcd["colors"].AsTensor().AllClose(
            core::Tensor::Ones({10, 3}, dtype, device) * 0.5));
    EXPECT_TRUE(pcd["normals"].AsTensor().AllClose(
            core::Tensor::Ones({10, 3}, dtype, device) * 0.25));
}

TEST_P(PointCloudPermuteDevices, SyncPushBack) {
    core::Device device = GetParam();
    core::Dtype dtype = core::Dtype::Float32;

    // Crate pointcloud.
    core::TensorList points = core::TensorList::FromTensor(
            core::Tensor::Ones({10, 3}, dtype, device));
    core::TensorList colors = core::TensorList::FromTensor(
            core::Tensor::Ones({10, 3}, dtype, device) * 0.5);
    tgeometry::PointCloud pcd({
            {"points", points},
            {"colors", colors},
    });

    // Good.
    std::unordered_map<std::string, core::Tensor> point_struct;
    EXPECT_EQ(pcd["points"].GetSize(), 10);
    EXPECT_EQ(pcd["colors"].GetSize(), 10);
    point_struct["points"] = core::Tensor::Ones({3}, dtype, device);
    point_struct["colors"] = core::Tensor::Ones({3}, dtype, device);
    pcd.SyncPushBack(point_struct);
    EXPECT_EQ(pcd["points"].GetSize(), 11);
    EXPECT_EQ(pcd["colors"].GetSize(), 11);

    // // Missing key.
    // point_struct.clear();
    // point_struct["points"] = core::Tensor::Ones({3}, dtype, device);
    // EXPECT_ANY_THROW(pcd.SyncPushBack(point_struct));

    // // Wrong dtype.
    // point_struct["points"] = core::Tensor::Ones({3}, core::Dtype::Bool,
    // device); point_struct["colors"] = core::Tensor::Ones({3}, dtype, device);
    // EXPECT_ANY_THROW(pcd.SyncPushBack(point_struct));

    // // Wrong shape.
    // point_struct["points"] = core::Tensor::Ones({5}, core::Dtype::Bool,
    // device); point_struct["colors"] = core::Tensor::Ones({3}, dtype, device);
    // EXPECT_ANY_THROW(pcd.SyncPushBack(point_struct));
}

TEST_P(PointCloudPermuteDevices, GetMinBound_GetMaxBound_GetCenter) {
    core::Device device = GetParam();
    tgeometry::PointCloud pcd(core::Dtype::Float32, device);

    core::TensorList& points = pcd.GetPointAttr("points");
    points.PushBack(core::Tensor(std::vector<float>{1, 2, 3}, {3},
                                 core::Dtype::Float32, device));
    points.PushBack(core::Tensor(std::vector<float>{4, 5, 6}, {3},
                                 core::Dtype::Float32, device));

    EXPECT_FALSE(pcd.IsEmpty());
    EXPECT_TRUE(pcd.HasPoints());
    EXPECT_EQ(pcd.GetMinBound().ToFlatVector<float>(),
              std::vector<float>({1, 2, 3}));
    EXPECT_EQ(pcd.GetMaxBound().ToFlatVector<float>(),
              std::vector<float>({4, 5, 6}));
    EXPECT_EQ(pcd.GetCenter().ToFlatVector<float>(),
              std::vector<float>({2.5, 3.5, 4.5}));
}

TEST_P(PointCloudPermuteDevices, Scale) {
    core::Device device = GetParam();
    tgeometry::PointCloud pcd;
    core::TensorList& points = pcd.GetPointAttr("points");
    points = core::TensorList::FromTensor(
            core::Tensor(std::vector<float>{0, 0, 0, 1, 1, 1, 2, 2, 2}, {3, 3},
                         core::Dtype::Float32, device));
    core::Tensor center(std::vector<float>{1, 1, 1}, {3}, core::Dtype::Float32,
                        device);
    float scale = 4;
    pcd.Scale(scale, center);
    EXPECT_EQ(points.AsTensor().ToFlatVector<float>(),
              std::vector<float>({-3, -3, -3, 1, 1, 1, 5, 5, 5}));
}

TEST_P(PointCloudPermuteDevices, FromLegacyPointCloud) {
    core::Device device = GetParam();
    geometry::PointCloud legacy_pcd;
    legacy_pcd.points_ = std::vector<Eigen::Vector3d>{Eigen::Vector3d(0, 0, 0),
                                                      Eigen::Vector3d(0, 0, 0)};
    legacy_pcd.colors_ = std::vector<Eigen::Vector3d>{Eigen::Vector3d(1, 1, 1),
                                                      Eigen::Vector3d(1, 1, 1)};

    // Float32: vector3d will be converted to float32.
    core::Dtype dtype = core::Dtype::Float32;
    tgeometry::PointCloud pcd = tgeometry::PointCloud::FromLegacyPointCloud(
            legacy_pcd, dtype, device);
    EXPECT_TRUE(pcd.HasPoints());
    EXPECT_TRUE(pcd.HasColors());
    EXPECT_FALSE(pcd.HasNormals());
    EXPECT_TRUE(pcd["points"].AsTensor().AllClose(
            core::Tensor::Zeros({2, 3}, dtype, device)));
    EXPECT_TRUE(pcd["colors"].AsTensor().AllClose(
            core::Tensor::Ones({2, 3}, dtype, device)));

    // Float64 case.
    dtype = core::Dtype::Float64;
    pcd = tgeometry::PointCloud::FromLegacyPointCloud(legacy_pcd, dtype,
                                                      device);
    EXPECT_TRUE(pcd.HasPoints());
    EXPECT_TRUE(pcd.HasColors());
    EXPECT_FALSE(pcd.HasNormals());
    EXPECT_TRUE(pcd["points"].AsTensor().AllClose(
            core::Tensor::Zeros({2, 3}, dtype, device)));
    EXPECT_TRUE(pcd["colors"].AsTensor().AllClose(
            core::Tensor::Ones({2, 3}, dtype, device)));
}

TEST_P(PointCloudPermuteDevices, ToLegacyPointCloud) {
    core::Device device = GetParam();
    core::Dtype dtype = core::Dtype::Float32;
    core::TensorList points = core::TensorList::FromTensor(
            core::Tensor::Ones({2, 3}, dtype, device));
    core::TensorList colors = core::TensorList::FromTensor(
            core::Tensor::Ones({2, 3}, dtype, device) * 2.);
    tgeometry::PointCloud pcd({
            {"points", points},
            {"colors", colors},
    });

    geometry::PointCloud legacy_pcd = pcd.ToLegacyPointCloud();
    EXPECT_TRUE(legacy_pcd.HasPoints());
    EXPECT_TRUE(legacy_pcd.HasColors());
    EXPECT_FALSE(legacy_pcd.HasNormals());
    EXPECT_EQ(legacy_pcd.points_.size(), 2);
    EXPECT_EQ(legacy_pcd.colors_.size(), 2);
    EXPECT_EQ(legacy_pcd.normals_.size(), 0);
    ExpectEQ(legacy_pcd.points_,
             std::vector<Eigen::Vector3d>{Eigen::Vector3d(1, 1, 1),
                                          Eigen::Vector3d(1, 1, 1)});
    ExpectEQ(legacy_pcd.colors_,
             std::vector<Eigen::Vector3d>{Eigen::Vector3d(2, 2, 2),
                                          Eigen::Vector3d(2, 2, 2)});
}

}  // namespace tests
}  // namespace open3d
