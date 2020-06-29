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

#include "open3d/pipelines/mesh_subdivision/TriangleMeshSubdivision.h"
#include "pybind/docstring.h"

namespace open3d {

void pybind_mesh_subdivision(py::module &m) {
    py::module m_sub = m.def_submodule("mesh_subdivision", "Mesh subdivision.");

    m_sub.def("subdivide_midpoint",
              &pipelines::mesh_subdivision::SubdivideMidpoint,
              "Function subdivide mesh using midpoint algorithm.", "mesh"_a,
              "number_of_iterations"_a = 1);
    m_sub.def("subdivide_loop", &pipelines::mesh_subdivision::SubdivideLoop,
              "Function subdivide mesh using Loop's algorithm. Loop, \"Smooth "
              "subdivision surfaces based on triangles\", 1987.",
              "mesh"_a, "number_of_iterations"_a = 1);

    docstring::FunctionDocInject(
            m_sub, "subdivide_midpoint",
            {{"mesh", "The input mesh."},
             {"number_of_iterations",
              "Number of iterations. A single iteration splits each triangle "
              "into four triangles that cover the same surface."}});
    docstring::FunctionDocInject(
            m_sub, "subdivide_loop",
            {{"mesh", "The input mesh."},
             {" number_of_iterations ",
              "Number of iterations. A single iteration splits each triangle "
              "into four triangles."}});
}

}  // namespace open3d
