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

#include "Open3D/Geometry/VoxelGrid.h"

#include <numeric>
#include <unordered_map>

#include "Open3D/Camera/PinholeCameraParameters.h"
#include "Open3D/Geometry/BoundingVolume.h"
#include "Open3D/Geometry/Image.h"
#include "Open3D/Geometry/Octree.h"
#include "Open3D/Utility/Console.h"
#include "Open3D/Utility/Helper.h"

namespace open3d {
namespace geometry {

VoxelGrid::VoxelGrid(const VoxelGrid &src_voxel_grid)
    : Geometry3D(Geometry::GeometryType::VoxelGrid),
      voxel_size_(src_voxel_grid.voxel_size_),
      origin_(src_voxel_grid.origin_),
      voxels_(src_voxel_grid.voxels_) {}

VoxelGrid &VoxelGrid::Clear() {
    voxel_size_ = 0.0;
    origin_ = Eigen::Vector3d::Zero();
    voxels_.clear();
    return *this;
}

bool VoxelGrid::IsEmpty() const { return !HasVoxels(); }

Eigen::Vector3d VoxelGrid::GetMinBound() const {
    if (!HasVoxels()) {
        return origin_;
    } else {
        Eigen::Array3i min_grid_index = voxels_.begin()->first;
        for (const auto &it : voxels_) {
            const geometry::Voxel &voxel = it.second;
            min_grid_index = min_grid_index.min(voxel.grid_index_.array());
        }
        return min_grid_index.cast<double>() * voxel_size_ + origin_.array();
    }
}

Eigen::Vector3d VoxelGrid::GetMaxBound() const {
    if (!HasVoxels()) {
        return origin_;
    } else {
        Eigen::Array3i max_grid_index = voxels_.begin()->first;
        for (const auto &it : voxels_) {
            const geometry::Voxel &voxel = it.second;
            max_grid_index = max_grid_index.max(voxel.grid_index_.array());
        }
        return (max_grid_index.cast<double>() + 1) * voxel_size_ +
               origin_.array();
    }
}

Eigen::Vector3d VoxelGrid::GetCenter() const {
    Eigen::Vector3d center(0, 0, 0);
    if (!HasVoxels()) {
        return center;
    }
    const Eigen::Vector3d half_voxel_size(0.5 * voxel_size_, 0.5 * voxel_size_,
                                          0.5 * voxel_size_);
    for (const auto &it : voxels_) {
        const geometry::Voxel &voxel = it.second;
        center += voxel.grid_index_.cast<double>() * voxel_size_ + origin_ +
                  half_voxel_size;
    }
    center /= double(voxels_.size());
    return center;
}

AxisAlignedBoundingBox VoxelGrid::GetAxisAlignedBoundingBox() const {
    AxisAlignedBoundingBox box;
    box.min_bound_ = GetMinBound();
    box.max_bound_ = GetMaxBound();
    return box;
}

OrientedBoundingBox VoxelGrid::GetOrientedBoundingBox() const {
    return OrientedBoundingBox::CreateFromAxisAlignedBoundingBox(
            GetAxisAlignedBoundingBox());
}

VoxelGrid &VoxelGrid::Transform(const Eigen::Matrix4d &transformation) {
    throw std::runtime_error("VoxelGrid::Transform is not supported");
    return *this;
}

VoxelGrid &VoxelGrid::Translate(const Eigen::Vector3d &translation,
                                bool relative) {
    throw std::runtime_error("Not implemented");
    return *this;
}

VoxelGrid &VoxelGrid::Scale(const double scale, bool center) {
    throw std::runtime_error("Not implemented");
    return *this;
}

VoxelGrid &VoxelGrid::Rotate(const Eigen::Vector3d &rotation,
                             bool center,
                             RotationType type) {
    throw std::runtime_error("Not implemented");
    return *this;
}

VoxelGrid &VoxelGrid::operator+=(const VoxelGrid &voxelgrid) {
    if (voxel_size_ != voxelgrid.voxel_size_) {
        utility::LogWarningf(
                "[VoxelGrid] Could not combine VoxelGrid because voxel_size "
                "differs (this=%f, other=%f.\n",
                voxel_size_, voxelgrid.voxel_size_);
        return *this;
    }
    if (origin_ != voxelgrid.origin_) {
        utility::LogWarningf(
                "[VoxelGrid] Could not combine VoxelGrid because origin "
                "differs (this=%f,%f,%f, other=%f,%f,%f.\n",
                origin_(0), origin_(1), origin_(2), voxelgrid.origin_(0),
                voxelgrid.origin_(1), voxelgrid.origin_(2));
        return *this;
    }
    if (this->HasColors() != voxelgrid.HasColors()) {
        utility::LogWarningf(
                "[VoxelGrid] Could not combine VoxelGrid one has colors and "
                "the other not.\n");
        return *this;
    }
    std::unordered_map<Eigen::Vector3i, AvgColorVoxel,
                       utility::hash_eigen::hash<Eigen::Vector3i>>
            voxelindex_to_accpoint;
    Eigen::Vector3d ref_coord;
    Eigen::Vector3i voxel_index;
    bool has_colors = voxelgrid.HasColors();
    for (const auto &it : voxelgrid.voxels_) {
        const geometry::Voxel &voxel = it.second;
        if (has_colors) {
            voxelindex_to_accpoint[voxel.grid_index_].Add(voxel.grid_index_,
                                                          voxel.color_);
        } else {
            voxelindex_to_accpoint[voxel.grid_index_].Add(voxel.grid_index_);
        }
    }
    for (const auto &it : voxels_) {
        const geometry::Voxel &voxel = it.second;
        if (has_colors) {
            voxelindex_to_accpoint[voxel.grid_index_].Add(voxel.grid_index_,
                                                          voxel.color_);
        } else {
            voxelindex_to_accpoint[voxel.grid_index_].Add(voxel.grid_index_);
        }
    }
    this->voxels_.clear();
    for (const auto &accpoint : voxelindex_to_accpoint) {
        this->voxels_[accpoint.second.GetVoxelIndex()] =
                Voxel(accpoint.second.GetVoxelIndex(),
                      accpoint.second.GetAverageColor());
    }
    return *this;
}

VoxelGrid VoxelGrid::operator+(const VoxelGrid &voxelgrid) const {
    return (VoxelGrid(*this) += voxelgrid);
}

Eigen::Vector3i VoxelGrid::GetVoxel(const Eigen::Vector3d &point) const {
    Eigen::Vector3d voxel_f = (point - origin_) / voxel_size_;
    return (Eigen::floor(voxel_f.array())).cast<int>();
}

std::vector<Eigen::Vector3d> VoxelGrid::GetVoxelBoundingPoints(
        Eigen::Vector3i index) const {
    double r = voxel_size_ / 2.0;
    auto x = GetVoxelCenterCoordinate(index);
    std::vector<Eigen::Vector3d> points;
    points.push_back(x + Eigen::Vector3d(-r, -r, -r));
    points.push_back(x + Eigen::Vector3d(-r, -r, r));
    points.push_back(x + Eigen::Vector3d(r, -r, -r));
    points.push_back(x + Eigen::Vector3d(r, -r, r));
    points.push_back(x + Eigen::Vector3d(-r, r, -r));
    points.push_back(x + Eigen::Vector3d(-r, r, r));
    points.push_back(x + Eigen::Vector3d(r, r, -r));
    points.push_back(x + Eigen::Vector3d(r, r, r));
    return points;
}

std::vector<bool> VoxelGrid::CheckIfIncluded(
        const std::vector<Eigen::Vector3d>& queries) {
    std::vector<bool> output;
    output.resize(queries.size());
    size_t i = 0;
    for (auto &query_double : queries) {
        auto query = GetVoxel(query_double);
        auto it = voxels_.find(query);
        if (it == voxels_.end())
            output[i] = false;
        else
            output[i] = true;
        i++;
    }
    return output;
}

void VoxelGrid::CreateFromOctree(const Octree &octree) {
    // TODO: currently only handles color leaf nodes
    // Get leaf nodes and their node_info
    std::unordered_map<std::shared_ptr<OctreeColorLeafNode>,
                       std::shared_ptr<OctreeNodeInfo>>
            map_node_to_node_info;
    auto f_collect_nodes =
            [&map_node_to_node_info](
                    const std::shared_ptr<OctreeNode> &node,
                    const std::shared_ptr<OctreeNodeInfo> &node_info) -> void {
        if (auto color_leaf_node =
                    std::dynamic_pointer_cast<OctreeColorLeafNode>(node)) {
            map_node_to_node_info[color_leaf_node] = node_info;
        }
    };
    octree.Traverse(f_collect_nodes);

    // Prepare dimensions for voxel
    origin_ = octree.origin_;
    voxels_.clear();
    for (const auto &it : map_node_to_node_info) {
        voxel_size_ = std::min(voxel_size_, it.second->size_);
    }

    // Convert nodes to voxels
    for (const auto &it : map_node_to_node_info) {
        const std::shared_ptr<OctreeColorLeafNode> &node = it.first;
        const std::shared_ptr<OctreeNodeInfo> &node_info = it.second;
        Eigen::Array3d node_center =
                Eigen::Array3d(node_info->origin_) + node_info->size_ / 2.0;
        Eigen::Vector3i grid_index =
                Eigen::floor((node_center - Eigen::Array3d(origin_)) /
                             voxel_size_)
                        .cast<int>();
        voxels_[grid_index] = Voxel(grid_index, node->color_);
    }
}

std::shared_ptr<geometry::Octree> VoxelGrid::ToOctree(
        const size_t &max_depth) const {
    auto octree = std::make_shared<geometry::Octree>(max_depth);
    octree->CreateFromVoxelGrid(*this);
    return octree;
}

VoxelGrid &VoxelGrid::CarveDepthMap(
        const Image &depth_map,
        const camera::PinholeCameraParameters &camera_parameter) {
    if (depth_map.height_ != camera_parameter.intrinsic_.height_ ||
        depth_map.width_ != camera_parameter.intrinsic_.width_) {
        utility::LogWarning(
                "[VoxelGrid] provided depth_map dimensions are not compatible "
                "with the provided camera_parameters\n");
        return *this;
    }

    auto rot = camera_parameter.extrinsic_.block<3, 3>(0, 0);
    auto trans = camera_parameter.extrinsic_.block<3, 1>(0, 3);
    auto intrinsic = camera_parameter.intrinsic_.intrinsic_matrix_;

    // get for each voxel if it projects to a valid pixel and check if the voxel
    // depth is behind the depth of the depth map at the projected pixel.
    bool carve = true;
    for (auto it = voxels_.begin(); it != voxels_.end();) {
        const geometry::Voxel &voxel = it->second;
        auto pts = GetVoxelBoundingPoints(voxel.grid_index_);
        for (auto &x : pts) {
            auto x_trans = rot * x + trans;
            auto uvz = intrinsic * x_trans;
            double z = uvz(2);
            double u = uvz(0) / z;
            double v = uvz(1) / z;
            double d;
            bool within_boundary;
            std::tie(within_boundary, d) = depth_map.FloatValueAt(u, v);
            if (within_boundary && d > 0 && z >= d) {
                carve = false;
                break;
            }
        }
        if (carve)
            it = voxels_.erase(it);
        else
            it++;
        carve = true;
    }
    return *this;
}

VoxelGrid &VoxelGrid::CarveSilhouette(
        const Image &silhouette_mask,
        const camera::PinholeCameraParameters &camera_parameter) {
    if (silhouette_mask.height_ != camera_parameter.intrinsic_.height_ ||
        silhouette_mask.width_ != camera_parameter.intrinsic_.width_) {
        utility::LogWarning(
                "[VoxelGrid] provided silhouette_mask dimensions are not "
                "compatible with the provided camera_parameters\n");
        return *this;
    }

    auto rot = camera_parameter.extrinsic_.block<3, 3>(0, 0);
    auto trans = camera_parameter.extrinsic_.block<3, 1>(0, 3);
    auto intrinsic = camera_parameter.intrinsic_.intrinsic_matrix_;

    // get for each voxel if it projects to a valid pixel and check if the pixel
    // is set (>0).
    bool carve = true;
    for (auto it = voxels_.begin(); it != voxels_.end();) {
        const geometry::Voxel &voxel = it->second;
        auto pts = GetVoxelBoundingPoints(voxel.grid_index_);
        for (auto &x : pts) {
            auto x_trans = rot * x + trans;
            auto uvz = intrinsic * x_trans;
            double z = uvz(2);
            double u = uvz(0) / z;
            double v = uvz(1) / z;
            double d;
            bool within_boundary;
            std::tie(within_boundary, d) = silhouette_mask.FloatValueAt(u, v);
            if (within_boundary && d > 0) {
                carve = false;
                break;
            }
        }
        if (carve)
            it = voxels_.erase(it);
        else
            it++;
        carve = true;
    }
    return *this;
}

}  // namespace geometry
}  // namespace open3d
