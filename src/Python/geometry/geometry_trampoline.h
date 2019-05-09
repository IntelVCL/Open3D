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

#include <Python/open3d_pybind.h>

#include "Open3D/Geometry/Geometry.h"
#include "Open3D/Geometry/Geometry2D.h"
#include "Open3D/Geometry/Geometry3D.h"
#include "Open3D/Geometry/TriangleMesh.h"
#include "Python/geometry/geometry.h"

using namespace open3d;

template <class GeometryBase = geometry::Geometry>
class PyGeometry : public GeometryBase {
public:
    using GeometryBase::GeometryBase;
    void Clear() override { PYBIND11_OVERLOAD_PURE(void, GeometryBase, ); }
    bool IsEmpty() const override {
        PYBIND11_OVERLOAD_PURE(bool, GeometryBase, );
    }
};

template <class Geometry3DBase = geometry::Geometry3D>
class PyGeometry3D : public PyGeometry<Geometry3DBase> {
public:
    using PyGeometry<Geometry3DBase>::PyGeometry;
    Eigen::Vector3d GetMinBound() const override {
        PYBIND11_OVERLOAD_PURE(Eigen::Vector3d, Geometry3DBase, );
    }
    Eigen::Vector3d GetMaxBound() const override {
        PYBIND11_OVERLOAD_PURE(Eigen::Vector3d, Geometry3DBase, );
    }
    Geometry3DBase& Transform(const Eigen::Matrix4d& transformation) override {
        PYBIND11_OVERLOAD_PURE(void, Geometry3DBase, transformation);
    }
};

template <class Geometry2DBase = geometry::Geometry2D>
class PyGeometry2D : public PyGeometry<Geometry2DBase> {
public:
    using PyGeometry<Geometry2DBase>::PyGeometry;
    Eigen::Vector2d GetMinBound() const override {
        PYBIND11_OVERLOAD_PURE(Eigen::Vector2d, Geometry2DBase, );
    }
    Eigen::Vector2d GetMaxBound() const override {
        PYBIND11_OVERLOAD_PURE(Eigen::Vector2d, Geometry2DBase, );
    }
};

template <class TriangleMeshBase = geometry::TriangleMesh>
class PyTriangleMesh : public PyGeometry3D<TriangleMeshBase> {
public:
    using PyGeometry3D<TriangleMeshBase>::PyGeometry3D;
    void RemoveDuplicatedVertices() override {
        PYBIND11_OVERLOAD(void, TriangleMeshBase, RemoveDuplicatedVertices, );
    };
    void RemoveDuplicatedTriangles() override {
        PYBIND11_OVERLOAD(void, TriangleMeshBase, RemoveDuplicatedTriangles, );
    };
    void RemoveNonManifoldVertices() override {
        PYBIND11_OVERLOAD(void, TriangleMeshBase, RemoveNonManifoldVertices, );
    };
    void RemoveNonManifoldTriangles() override {
        PYBIND11_OVERLOAD(void, TriangleMeshBase, RemoveNonManifoldTriangles, );
    };
};
