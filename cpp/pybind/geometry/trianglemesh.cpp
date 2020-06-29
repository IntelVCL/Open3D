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

#include "open3d/geometry/TriangleMesh.h"
#include "open3d/geometry/Image.h"
#include "open3d/geometry/PointCloud.h"

#include "pybind/docstring.h"
#include "pybind/geometry/geometry.h"
#include "pybind/geometry/geometry_trampoline.h"

namespace open3d {

void pybind_trianglemesh(py::module &m) {
    py::class_<geometry::TriangleMesh, PyGeometry3D<geometry::TriangleMesh>,
               std::shared_ptr<geometry::TriangleMesh>, geometry::MeshBase>
            trianglemesh(m, "TriangleMesh",
                         "TriangleMesh class. Triangle mesh contains vertices "
                         "and triangles represented by the indices to the "
                         "vertices. Optionally, the mesh may also contain "
                         "triangle normals, vertex normals and vertex colors.");
    py::detail::bind_default_constructor<geometry::TriangleMesh>(trianglemesh);
    py::detail::bind_copy_functions<geometry::TriangleMesh>(trianglemesh);
    trianglemesh
            .def(py::init<const std::vector<Eigen::Vector3d> &,
                          const std::vector<Eigen::Vector3i> &>(),
                 "Create a triangle mesh from vertices and triangle indices",
                 "vertices"_a, "triangles"_a)
            .def("__repr__",
                 [](const geometry::TriangleMesh &mesh) {
                     std::string info = fmt::format(
                             "geometry::TriangleMesh with {} points and {} "
                             "triangles",
                             mesh.vertices_.size(), mesh.triangles_.size());

                     if (mesh.HasTextures()) {
                         info += fmt::format(", and textures of size ");
                         for (auto &tex : mesh.textures_) {
                             info += fmt::format("({}, {}) ", tex.width_,
                                                 tex.height_);
                         }
                     } else {
                         info += ".";
                     }
                     return info;
                 })
            .def(py::self + py::self)
            .def(py::self += py::self)
            .def("compute_triangle_normals",
                 &geometry::TriangleMesh::ComputeTriangleNormals,
                 "Function to compute triangle normals, usually called before "
                 "rendering",
                 "normalized"_a = true)
            .def("compute_vertex_normals",
                 &geometry::TriangleMesh::ComputeVertexNormals,
                 "Function to compute vertex normals, usually called before "
                 "rendering",
                 "normalized"_a = true)
            .def("compute_adjacency_list",
                 &geometry::TriangleMesh::ComputeAdjacencyList,
                 "Function to compute adjacency list, call before adjacency "
                 "list is needed")
            .def("remove_duplicated_vertices",
                 &geometry::TriangleMesh::RemoveDuplicatedVertices,
                 "Function that removes duplicated verties, i.e., vertices "
                 "that have identical coordinates.")
            .def("remove_duplicated_triangles",
                 &geometry::TriangleMesh::RemoveDuplicatedTriangles,
                 "Function that removes duplicated triangles, i.e., removes "
                 "triangles that reference the same three vertices, "
                 "independent of their order.")
            .def("remove_unreferenced_vertices",
                 &geometry::TriangleMesh::RemoveUnreferencedVertices,
                 "This function removes vertices from the triangle mesh that "
                 "are not referenced in any triangle of the mesh.")
            .def("remove_degenerate_triangles",
                 &geometry::TriangleMesh::RemoveDegenerateTriangles,
                 "Function that removes degenerate triangles, i.e., triangles "
                 "that references a single vertex multiple times in a single "
                 "triangle. They are usually the product of removing "
                 "duplicated vertices.")
            .def("remove_non_manifold_edges",
                 &geometry::TriangleMesh::RemoveNonManifoldEdges,
                 "Function that removes all non-manifold edges, by "
                 "successively deleting  triangles with the smallest surface "
                 "area adjacent to the non-manifold edge until the number of "
                 "adjacent triangles to the edge is `<= 2`.")
            .def("merge_close_vertices",
                 &geometry::TriangleMesh::MergeCloseVertices,
                 "Function that will merge close by vertices to a single one. "
                 "The vertex position, "
                 "normal and color will be the average of the vertices. The "
                 "parameter eps "
                 "defines the maximum distance of close by vertices.  This "
                 "function might help to "
                 "close triangle soups.",
                 "eps"_a)
            .def("has_vertices", &geometry::TriangleMesh::HasVertices,
                 "Returns ``True`` if the mesh contains vertices.")
            .def("has_triangles", &geometry::TriangleMesh::HasTriangles,
                 "Returns ``True`` if the mesh contains triangles.")
            .def("has_vertex_normals",
                 &geometry::TriangleMesh::HasVertexNormals,
                 "Returns ``True`` if the mesh contains vertex normals.")
            .def("has_vertex_colors", &geometry::TriangleMesh::HasVertexColors,
                 "Returns ``True`` if the mesh contains vertex colors.")
            .def("has_triangle_normals",
                 &geometry::TriangleMesh::HasTriangleNormals,
                 "Returns ``True`` if the mesh contains triangle normals.")
            .def("has_adjacency_list",
                 &geometry::TriangleMesh::HasAdjacencyList,
                 "Returns ``True`` if the mesh contains adjacency normals.")
            .def("has_triangle_uvs", &geometry::TriangleMesh::HasTriangleUvs,
                 "Returns ``True`` if the mesh contains uv coordinates.")
            .def("has_triangle_material_ids",
                 &geometry::TriangleMesh::HasTriangleMaterialIds,
                 "Returns ``True`` if the mesh contains material ids.")
            .def("has_textures", &geometry::TriangleMesh::HasTextures,
                 "Returns ``True`` if the mesh contains a texture image.")
            .def("normalize_normals", &geometry::TriangleMesh::NormalizeNormals,
                 "Normalize both triangle normals and vertex normals to length "
                 "1.")
            .def("paint_uniform_color",
                 &geometry::TriangleMesh::PaintUniformColor,
                 "Assigns each vertex in the TriangleMesh the same color.")
            .def("euler_poincare_characteristic",
                 &geometry::TriangleMesh::EulerPoincareCharacteristic,
                 "Function that computes the Euler-Poincaré characteristic, "
                 "i.e., V + F - E, where V is the number of vertices, F is the "
                 "number of triangles, and E is the number of edges.")
            .def("get_non_manifold_edges",
                 &geometry::TriangleMesh::GetNonManifoldEdges,
                 "Get list of non-manifold edges.",
                 "allow_boundary_edges"_a = true)
            .def("is_edge_manifold", &geometry::TriangleMesh::IsEdgeManifold,
                 "Tests if the triangle mesh is edge manifold.",
                 "allow_boundary_edges"_a = true)
            .def("get_non_manifold_vertices",
                 &geometry::TriangleMesh::GetNonManifoldVertices,
                 "Returns a list of indices to non-manifold vertices.")
            .def("is_vertex_manifold",
                 &geometry::TriangleMesh::IsVertexManifold,
                 "Tests if all vertices of the triangle mesh are manifold.")
            .def("is_self_intersecting",
                 &geometry::TriangleMesh::IsSelfIntersecting,
                 "Tests if the triangle mesh is self-intersecting.")
            .def("get_self_intersecting_triangles",
                 &geometry::TriangleMesh::GetSelfIntersectingTriangles,
                 "Returns a list of indices to triangles that intersect the "
                 "mesh.")
            .def("is_intersecting", &geometry::TriangleMesh::IsIntersecting,
                 "Tests if the triangle mesh is intersecting the other "
                 "triangle mesh.")
            .def("is_orientable", &geometry::TriangleMesh::IsOrientable,
                 "Tests if the triangle mesh is orientable.")
            .def("is_watertight", &geometry::TriangleMesh::IsWatertight,
                 "Tests if the triangle mesh is watertight.")
            .def("orient_triangles", &geometry::TriangleMesh::OrientTriangles,
                 "If the mesh is orientable this function orients all "
                 "triangles such that all normals point towards the same "
                 "direction.")
            .def("select_by_index", &geometry::TriangleMesh::SelectByIndex,
                 "Function to select mesh from input triangle mesh into output "
                 "triangle mesh. ``input``: The input triangle mesh. "
                 "``indices``: "
                 "Indices of vertices to be selected.",
                 "indices"_a, "cleanup"_a = true)
            .def("crop",
                 (std::shared_ptr<geometry::TriangleMesh>(
                         geometry::TriangleMesh::*)(
                         const geometry::AxisAlignedBoundingBox &) const) &
                         geometry::TriangleMesh::Crop,
                 "Function to crop input TriangleMesh into output TriangleMesh",
                 "bounding_box"_a)
            .def("crop",
                 (std::shared_ptr<geometry::TriangleMesh>(
                         geometry::TriangleMesh::*)(
                         const geometry::OrientedBoundingBox &) const) &
                         geometry::TriangleMesh::Crop,
                 "Function to crop input TriangleMesh into output TriangleMesh",
                 "bounding_box"_a)
            .def("get_surface_area",
                 (double (geometry::TriangleMesh::*)() const) &
                         geometry::TriangleMesh::GetSurfaceArea,
                 "Function that computes the surface area of the mesh, i.e. "
                 "the sum of the individual triangle surfaces.")
            .def("compute_convex_hull",
                 &geometry::TriangleMesh::ComputeConvexHull,
                 "Computes the convex hull of the triangle mesh.")
            .def("cluster_connected_triangles",
                 &geometry::TriangleMesh::ClusterConnectedTriangles,
                 "Function that clusters connected triangles, i.e., triangles "
                 "that are connected via edges are assigned the same cluster "
                 "index.  This function returns an array that contains the "
                 "cluster index per triangle, a second array contains the "
                 "number of triangles per cluster, and a third vector contains "
                 "the surface area per cluster.")
            .def("remove_triangles_by_index",
                 &geometry::TriangleMesh::RemoveTrianglesByIndex,
                 "This function removes the triangles with index in "
                 "triangle_indices.  Call remove_unreferenced_vertices to "
                 "clean up vertices afterwards.",
                 "triangle_indices"_a)
            .def("remove_triangles_by_mask",
                 &geometry::TriangleMesh::RemoveTrianglesByMask,
                 "This function removes the triangles where triangle_mask is "
                 "set to true.  Call remove_unreferenced_vertices to clean up "
                 "vertices afterwards.",
                 "triangle_mask"_a)
            .def("remove_vertices_by_index",
                 &geometry::TriangleMesh::RemoveVerticesByIndex,
                 "This function removes the vertices with index in "
                 "vertex_indices. Note that also all triangles associated with "
                 "the vertices are removed.",
                 "vertex_indices"_a)
            .def("remove_vertices_by_mask",
                 &geometry::TriangleMesh::RemoveVerticesByMask,
                 "This function removes the vertices that are masked in "
                 "vertex_mask. Note that also all triangles associated with "
                 "the vertices are removed.",
                 "vertex_mask"_a)
            .def_readwrite("vertices", &geometry::TriangleMesh::vertices_,
                           "``float64`` array of shape ``(num_vertices, 3)``, "
                           "use ``numpy.asarray()`` to access data: Vertex "
                           "coordinates.")
            .def_readwrite("vertex_normals",
                           &geometry::TriangleMesh::vertex_normals_,
                           "``float64`` array of shape ``(num_vertices, 3)``, "
                           "use ``numpy.asarray()`` to access data: Vertex "
                           "normals.")
            .def_readwrite(
                    "vertex_colors", &geometry::TriangleMesh::vertex_colors_,
                    "``float64`` array of shape ``(num_vertices, 3)``, "
                    "range ``[0, 1]`` , use ``numpy.asarray()`` to access "
                    "data: RGB colors of vertices.")
            .def_readwrite("triangles", &geometry::TriangleMesh::triangles_,
                           "``int`` array of shape ``(num_triangles, 3)``, use "
                           "``numpy.asarray()`` to access data: List of "
                           "triangles denoted by the index of points forming "
                           "the triangle.")
            .def_readwrite("triangle_normals",
                           &geometry::TriangleMesh::triangle_normals_,
                           "``float64`` array of shape ``(num_triangles, 3)``, "
                           "use ``numpy.asarray()`` to access data: Triangle "
                           "normals.")
            .def_readwrite(
                    "adjacency_list", &geometry::TriangleMesh::adjacency_list_,
                    "List of Sets: The set ``adjacency_list[i]`` contains the "
                    "indices of adjacent vertices of vertex i.")
            .def_readwrite("triangle_uvs",
                           &geometry::TriangleMesh::triangle_uvs_,
                           "``float64`` array of shape ``(3 * num_triangles, "
                           "2)``, use "
                           "``numpy.asarray()`` to access data: List of "
                           "uvs denoted by the index of points forming "
                           "the triangle.")
            .def_readwrite("textures", &geometry::TriangleMesh::textures_,
                           "open3d.geometry.Image: The texture images.");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "compute_adjacency_list");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "compute_triangle_normals");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "compute_vertex_normals");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "has_adjacency_list");
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "has_triangle_normals",
            {{"normalized",
              "Set to ``True`` to normalize the normal to length 1."}});
    docstring::ClassMethodDocInject(m, "TriangleMesh", "has_triangles");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "has_triangle_uvs");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "has_triangle_material_ids");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "has_textures");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "has_vertex_colors");
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "has_vertex_normals",
            {{"normalized",
              "Set to ``True`` to normalize the normal to length 1."}});
    docstring::ClassMethodDocInject(m, "TriangleMesh", "has_vertices");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "normalize_normals");
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "paint_uniform_color",
            {{"color", "RGB color for the PointCloud."}});
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "euler_poincare_characteristic");
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "get_non_manifold_edges",
            {{"allow_boundary_edges",
              "If true, than non-manifold edges are defined as edges with more "
              "than two adjacent triangles, otherwise each edge that is not "
              "adjacent to two triangles is defined as non-manifold."}});
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "is_edge_manifold",
            {{"allow_boundary_edges",
              "If true, than non-manifold edges are defined as edges with more "
              "than two adjacent triangles, otherwise each edge that is not "
              "adjacent to two triangles is defined as non-manifold."}});
    docstring::ClassMethodDocInject(m, "TriangleMesh", "is_vertex_manifold");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "get_non_manifold_vertices");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "is_self_intersecting");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "get_self_intersecting_triangles");
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "is_intersecting",
            {{"other", "Other triangle mesh to test intersection with."}});
    docstring::ClassMethodDocInject(m, "TriangleMesh", "is_orientable");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "is_watertight");
    docstring::ClassMethodDocInject(m, "TriangleMesh", "orient_triangles");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "remove_duplicated_vertices");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "remove_duplicated_triangles");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "remove_unreferenced_vertices");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "remove_degenerate_triangles");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "remove_non_manifold_edges");
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "merge_close_vertices",
            {{"eps",
              "Parameter that defines the distance between close vertices."}});
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "select_by_index",
            {{"indices", "Indices of vertices to be selected."},
             {"cleanup",
              "If true calls number of mesh cleanup functions to remove "
              "unreferenced vertices and degenerate triangles"}});
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "crop",
            {{"bounding_box", "AxisAlignedBoundingBox to crop points"}});
    docstring::ClassMethodDocInject(m, "TriangleMesh", "compute_convex_hull");
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "cluster_connected_triangles");
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "remove_triangles_by_index",
            {{"triangle_indices",
              "1D array of triangle indices that should be removed from the "
              "TriangleMesh."}});
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "remove_triangles_by_mask",
                                    {{"triangle_mask",
                                      "1D bool array, True values indicate "
                                      "triangles that should be removed."}});
    docstring::ClassMethodDocInject(
            m, "TriangleMesh", "remove_vertices_by_index",
            {{"vertex_indices",
              "1D array of vertex indices that should be removed from the "
              "TriangleMesh."}});
    docstring::ClassMethodDocInject(m, "TriangleMesh",
                                    "remove_vertices_by_mask",
                                    {{"vertex_mask",
                                      "1D bool array, True values indicate "
                                      "vertices that should be removed."}});
}

void pybind_trianglemesh_methods(py::module &m) {}

}  // namespace open3d
