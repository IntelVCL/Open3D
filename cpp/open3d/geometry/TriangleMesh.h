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

#pragma once

#include <Eigen/Core>
#include <memory>
#include <numeric>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "open3d/geometry/Image.h"
#include "open3d/geometry/MeshBase.h"
#include "open3d/utility/Helper.h"

namespace open3d {
namespace geometry {

class PointCloud;
class TetraMesh;

/// \class TriangleMesh
///
/// \brief Triangle mesh contains vertices and triangles represented by the
/// indices to the vertices.
///
/// Optionally, the mesh may also contain triangle normals, vertex normals and
/// vertex colors.
class TriangleMesh : public MeshBase {
public:
    /// \brief Default Constructor.
    TriangleMesh() : MeshBase(Geometry::GeometryType::TriangleMesh) {}
    /// \brief Parameterized Constructor.
    ///
    /// \param vertices list of vertices.
    /// \param triangles list of triangles.
    TriangleMesh(const std::vector<Eigen::Vector3d> &vertices,
                 const std::vector<Eigen::Vector3i> &triangles)
        : MeshBase(Geometry::GeometryType::TriangleMesh, vertices),
          triangles_(triangles) {}
    ~TriangleMesh() override {}

public:
    virtual TriangleMesh &Clear() override;
    virtual TriangleMesh &Transform(
            const Eigen::Matrix4d &transformation) override;
    virtual TriangleMesh &Rotate(const Eigen::Matrix3d &R,
                                 const Eigen::Vector3d &center) override;

public:
    TriangleMesh &operator+=(const TriangleMesh &mesh);
    TriangleMesh operator+(const TriangleMesh &mesh) const;

    /// Returns `true` if the mesh contains triangles.
    bool HasTriangles() const {
        return vertices_.size() > 0 && triangles_.size() > 0;
    }

    /// Returns `true` if the mesh contains triangle normals.
    bool HasTriangleNormals() const {
        return HasTriangles() && triangles_.size() == triangle_normals_.size();
    }

    /// Returns `true` if the mesh contains adjacency normals.
    bool HasAdjacencyList() const {
        return vertices_.size() > 0 &&
               adjacency_list_.size() == vertices_.size();
    }

    bool HasTriangleUvs() const {
        return HasTriangles() && triangle_uvs_.size() == 3 * triangles_.size();
    }

    /// Returns `true` if the mesh has texture.
    bool HasTextures() const {
        bool is_all_texture_valid = std::accumulate(
                textures_.begin(), textures_.end(), true,
                [](bool a, const Image &b) { return a && !b.IsEmpty(); });
        return !textures_.empty() && is_all_texture_valid;
    }

    bool HasMaterials() const { return !materials_.empty(); }

    bool HasTriangleMaterialIds() const {
        return HasTriangles() &&
               triangle_material_ids_.size() == triangles_.size();
    }

    /// Normalize both triangle normals and vertex normals to length 1.
    TriangleMesh &NormalizeNormals() {
        MeshBase::NormalizeNormals();
        for (size_t i = 0; i < triangle_normals_.size(); i++) {
            triangle_normals_[i].normalize();
            if (std::isnan(triangle_normals_[i](0))) {
                triangle_normals_[i] = Eigen::Vector3d(0.0, 0.0, 1.0);
            }
        }
        return *this;
    }

    /// \brief Function to compute triangle normals, usually called before
    /// rendering.
    TriangleMesh &ComputeTriangleNormals(bool normalized = true);

    /// \brief Function to compute vertex normals, usually called before
    /// rendering.
    TriangleMesh &ComputeVertexNormals(bool normalized = true);

    /// \brief Function to compute adjacency list, call before adjacency list is
    /// needed.
    TriangleMesh &ComputeAdjacencyList();

    /// \brief Function that removes duplicated verties, i.e., vertices that
    /// have identical coordinates.
    TriangleMesh &RemoveDuplicatedVertices();

    /// \brief Function that removes duplicated triangles, i.e., removes
    /// triangles that reference the same three vertices, independent of their
    /// order.
    TriangleMesh &RemoveDuplicatedTriangles();

    /// \brief This function removes vertices from the triangle mesh that are
    /// not referenced in any triangle of the mesh.
    TriangleMesh &RemoveUnreferencedVertices();

    /// \brief Function that removes degenerate triangles, i.e., triangles that
    /// reference a single vertex multiple times in a single triangle.
    ///
    /// They are usually the product of removing duplicated vertices.
    TriangleMesh &RemoveDegenerateTriangles();

    /// \brief Function that removes all non-manifold edges, by successively
    /// deleting triangles with the smallest surface area adjacent to the
    /// non-manifold edge until the number of adjacent triangles to the edge is
    /// `<= 2`.
    TriangleMesh &RemoveNonManifoldEdges();

    /// \brief Function that will merge close by vertices to a single one.
    /// The vertex position, normal and color will be the average of the
    /// vertices.
    ///
    /// \param eps defines the maximum distance of close by vertices.
    /// This function might help to close triangle soups.
    TriangleMesh &MergeCloseVertices(double eps);

    /// Function that computes the Euler-Poincaré characteristic, i.e.,
    /// V + F - E, where V is the number of vertices, F is the number
    /// of triangles, and E is the number of edges.
    int EulerPoincareCharacteristic() const;

    /// Function that returns the non-manifold edges of the triangle mesh.
    /// If \param allow_boundary_edges is set to false, then also boundary
    /// edges are returned.
    std::vector<Eigen::Vector2i> GetNonManifoldEdges(
            bool allow_boundary_edges = true) const;

    /// Function that checks if the given triangle mesh is edge-manifold.
    /// A mesh is edge­-manifold if each edge is bounding either one or two
    /// triangles. If allow_boundary_edges is set to false, then this function
    /// returns false if there exists boundary edges.
    bool IsEdgeManifold(bool allow_boundary_edges = true) const;

    /// Function that returns a list of non-manifold vertex indices.
    /// A vertex is manifold if its star is edge‐manifold and edge‐connected.
    /// (Two or more faces connected only by a vertex and not by an edge.)
    std::vector<int> GetNonManifoldVertices() const;

    /// Function that checks if all vertices in the triangle mesh are manifold.
    /// A vertex is manifold if its star is edge‐manifold and edge‐connected.
    /// (Two or more faces connected only by a vertex and not by an edge.)
    bool IsVertexManifold() const;

    /// Function that returns a list of triangles that are intersecting the
    /// mesh.
    std::vector<Eigen::Vector2i> GetSelfIntersectingTriangles() const;

    /// Function that tests if the triangle mesh is self-intersecting.
    /// Tests each triangle pair for intersection.
    bool IsSelfIntersecting() const;

    /// Function that tests if the bounding boxes of the triangle meshes are
    /// intersecting.
    bool IsBoundingBoxIntersecting(const TriangleMesh &other) const;

    /// Function that tests if the triangle mesh intersects another triangle
    /// mesh. Tests each triangle against each other triangle.
    bool IsIntersecting(const TriangleMesh &other) const;

    /// Function that tests if the given triangle mesh is orientable, i.e.
    /// the triangles can be oriented in such a way that all normals point
    /// towards the outside.
    bool IsOrientable() const;

    /// Function that tests if the given triangle mesh is watertight by
    /// checking if it is vertex manifold and edge-manifold with no boundary
    /// edges, but not self-intersecting.
    bool IsWatertight() const;

    /// If the mesh is orientable then this function rearranges the
    /// triangles such that all normals point towards the
    /// outside/inside.
    bool OrientTriangles();

    /// Function that returns a map from edges (vertex0, vertex1) to the
    /// triangle indices the given edge belongs to.
    std::unordered_map<Eigen::Vector2i,
                       std::vector<int>,
                       utility::hash_eigen::hash<Eigen::Vector2i>>
    GetEdgeToTrianglesMap() const;

    /// Function that returns a map from edges (vertex0, vertex1) to the
    /// vertex (vertex2) indices the given edge belongs to.
    std::unordered_map<Eigen::Vector2i,
                       std::vector<int>,
                       utility::hash_eigen::hash<Eigen::Vector2i>>
    GetEdgeToVerticesMap() const;

    /// Function that computes the area of a mesh triangle
    static double ComputeTriangleArea(const Eigen::Vector3d &p0,
                                      const Eigen::Vector3d &p1,
                                      const Eigen::Vector3d &p2);

    /// Function that computes the area of a mesh triangle identified by the
    /// triangle index
    double GetTriangleArea(size_t triangle_idx) const;

    /// Function that computes the surface area of the mesh, i.e. the sum of
    /// the individual triangle surfaces.
    double GetSurfaceArea() const;

    /// Function that computes the surface area of the mesh, i.e. the sum of
    /// the individual triangle surfaces.
    double GetSurfaceArea(std::vector<double> &triangle_areas) const;

    /// Function that computes the plane equation from the three points.
    /// If the three points are co-linear, then this function returns the
    /// invalid plane (0, 0, 0, 0).
    static Eigen::Vector4d ComputeTrianglePlane(const Eigen::Vector3d &p0,
                                                const Eigen::Vector3d &p1,
                                                const Eigen::Vector3d &p2);

    /// Function that computes the plane equation of a mesh triangle identified
    /// by the triangle index.
    Eigen::Vector4d GetTrianglePlane(size_t triangle_idx) const;

    /// Function to select points from \p input TriangleMesh into
    /// output TriangleMesh
    /// Vertices with indices in \p indices are selected.
    /// \param indices defines Indices of vertices to be selected.
    /// \param cleanup If true it automatically calls
    /// TriangleMesh::RemoveDuplicatedVertices,
    /// TriangleMesh::RemoveDuplicatedTriangles,
    /// TriangleMesh::RemoveUnreferencedVertices, and
    /// TriangleMesh::RemoveDegenerateTriangles
    std::shared_ptr<TriangleMesh> SelectByIndex(
            const std::vector<size_t> &indices, bool cleanup = true) const;

    /// Function to crop pointcloud into output pointcloud
    /// All points with coordinates outside the bounding box \param bbox are
    /// clipped.
    /// \param bbox defines the input Axis Aligned Bounding Box.
    std::shared_ptr<TriangleMesh> Crop(
            const AxisAlignedBoundingBox &bbox) const;

    /// Function to crop pointcloud into output pointcloud
    /// All points with coordinates outside the bounding box \param bbox are
    /// clipped.
    /// \param bbox defines the input Oriented Bounding Box.
    std::shared_ptr<TriangleMesh> Crop(const OrientedBoundingBox &bbox) const;

    /// \brief Function that clusters connected triangles, i.e., triangles that
    /// are connected via edges are assigned the same cluster index.
    ///
    /// \return A vector that contains the cluster index per
    /// triangle, a second vector contains the number of triangles per
    /// cluster, and a third vector contains the surface area per cluster.
    std::tuple<std::vector<int>, std::vector<size_t>, std::vector<double>>
    ClusterConnectedTriangles() const;

    /// \brief This function removes the triangles with index in
    /// \p triangle_indices. Call \ref RemoveUnreferencedVertices to clean up
    /// vertices afterwards.
    ///
    /// \param triangle_indices Indices of the triangles that should be
    /// removed.
    void RemoveTrianglesByIndex(const std::vector<size_t> &triangle_indices);

    /// \brief This function removes the triangles that are masked in
    /// \p triangle_mask. Call \ref RemoveUnreferencedVertices to clean up
    /// vertices afterwards.
    ///
    /// \param triangle_mask Mask of triangles that should be removed.
    /// Should have same size as \ref triangles_.
    void RemoveTrianglesByMask(const std::vector<bool> &triangle_mask);

    /// \brief This function removes the vertices with index in
    /// \p vertex_indices. Note that also all triangles associated with the
    /// vertices are removeds.
    ///
    /// \param vertex_indices Indices of the vertices that should be
    /// removed.
    void RemoveVerticesByIndex(const std::vector<size_t> &vertex_indices);

    /// \brief This function removes the vertices that are masked in
    /// \p vertex_mask. Note that also all triangles associated with the
    /// vertices are removed.
    ///
    /// \param vertex_mask Mask of vertices that should be removed.
    /// Should have same size as \ref vertices_.
    void RemoveVerticesByMask(const std::vector<bool> &vertex_mask);

protected:
    // Forward child class type to avoid indirect nonvirtual base
    TriangleMesh(Geometry::GeometryType type) : MeshBase(type) {}

public:
    /// List of triangles denoted by the index of points forming the triangle.
    std::vector<Eigen::Vector3i> triangles_;
    /// Triangle normals.
    std::vector<Eigen::Vector3d> triangle_normals_;
    /// The set adjacency_list[i] contains the indices of adjacent vertices of
    /// vertex i.
    std::vector<std::unordered_set<int>> adjacency_list_;
    /// List of uv coordinates per triangle.
    std::vector<Eigen::Vector2d> triangle_uvs_;

    struct Material {
        struct MaterialParameter {
            float f4[4] = {0};

            MaterialParameter() {
                f4[0] = 0;
                f4[1] = 0;
                f4[2] = 0;
                f4[3] = 0;
            }

            MaterialParameter(const float v1,
                              const float v2,
                              const float v3,
                              const float v4) {
                f4[0] = v1;
                f4[1] = v2;
                f4[2] = v3;
                f4[3] = v4;
            }

            MaterialParameter(const float v1, const float v2, const float v3) {
                f4[0] = v1;
                f4[1] = v2;
                f4[2] = v3;
                f4[3] = 1;
            }

            MaterialParameter(const float v1, const float v2) {
                f4[0] = v1;
                f4[1] = v2;
                f4[2] = 0;
                f4[3] = 0;
            }

            explicit MaterialParameter(const float v1) {
                f4[0] = v1;
                f4[1] = 0;
                f4[2] = 0;
                f4[3] = 0;
            }

            static MaterialParameter CreateRGB(const float r,
                                               const float g,
                                               const float b) {
                return {r, g, b, 1.f};
            }

            float r() const { return f4[0]; }
            float g() const { return f4[1]; }
            float b() const { return f4[2]; }
            float a() const { return f4[3]; }
        };

        MaterialParameter baseColor;
        float baseMetallic = 0.f;
        float baseRoughness = 1.f;
        float baseReflectance = 0.5f;
        float baseClearCoat = 0.f;
        float baseClearCoatRoughness = 0.f;
        float baseAnisotropy = 0.f;

        std::shared_ptr<Image> albedo;
        std::shared_ptr<Image> normalMap;
        std::shared_ptr<Image> ambientOcclusion;
        std::shared_ptr<Image> metallic;
        std::shared_ptr<Image> roughness;
        std::shared_ptr<Image> reflectance;
        std::shared_ptr<Image> clearCoat;
        std::shared_ptr<Image> clearCoatRoughness;
        std::shared_ptr<Image> anisotropy;

        std::unordered_map<std::string, MaterialParameter> floatParameters;
        std::unordered_map<std::string, Image> additionalMaps;
    };

    std::unordered_map<std::string, Material> materials_;

    /// List of material ids.
    std::vector<int> triangle_material_ids_;
    /// Textures of the image.
    std::vector<Image> textures_;
};

}  // namespace geometry
}  // namespace open3d
