// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2020 www.open3d.org
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

#include "open3d/ml/tensorflow/TensorFlowHelper.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/lib/core/errors.h"

using namespace tensorflow;

REGISTER_OP("Open3DSparseConv")
        .Attr("TReal: {float, double}")
        .Attr("TIndex: {int32, int64}")
        .Attr("TKernelIndex: {uint8, int16}")
        .Attr("normalize: bool = false")
        .Attr("max_temp_mem_MB: int = 64")
        .Input("filters: TReal")       // [depth, height, width, in_ch, out_ch]
        .Input("inp_features: TReal")  // [num_points_in, in_ch]
        .Input("inp_importance: TReal")                 // [num_points_in]
        .Input("neighbors_index: TIndex")               // [?]
        .Input("neighbors_kernel_index: TKernelIndex")  // [?]
        .Input("neighbors_importance: TReal")           // [?]
        .Input("neighbors_row_splits: int64")           // [num_points_out+1]
        .Output("out_features : TReal")  // [num_points_out, out_ch]
        .SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {
            using namespace ::tensorflow::shape_inference;
            using namespace open3d::ml::op_util;

            ShapeHandle filters_shape = c->input(0);
            ShapeHandle inp_features_shape = c->input(1);
            ShapeHandle inp_importance_shape = c->input(2);
            ShapeHandle neighbors_index_shape = c->input(3);
            ShapeHandle neighbors_kernel_index_shape = c->input(4);
            ShapeHandle neighbors_importance_shape = c->input(5);
            ShapeHandle neighbors_row_splits_shape = c->input(6);

            Dim num_out("num_out");
            Dim num_inp("num_inp");
            Dim num_kernel_elements("num_kernel_elements");
            Dim in_channels("in_channels");
            Dim out_channels("out_channels");
            Dim num_neighbors("num_neighbors");

            CHECK_SHAPE_HANDLE_COMBINE_FIRST_DIMS(c, filters_shape,
                                                  num_kernel_elements,
                                                  in_channels, out_channels);
            CHECK_SHAPE_HANDLE(c, neighbors_row_splits_shape, num_out + 1);
            CHECK_SHAPE_HANDLE(c, inp_features_shape, num_inp, in_channels);
            CHECK_SHAPE_HANDLE(c, inp_importance_shape, 0 || num_inp);
            CHECK_SHAPE_HANDLE(c, neighbors_index_shape, num_neighbors);
            CHECK_SHAPE_HANDLE(c, neighbors_kernel_index_shape, num_neighbors);
            CHECK_SHAPE_HANDLE(c, neighbors_importance_shape,
                               0 || num_neighbors);

            ShapeHandle out_features_shape =
                    MakeShapeHandle(c, num_out, out_channels);
            c->set_output(0, out_features_shape);
            return Status::OK();
        })
        .Doc(R"doc(
Sparse convolution of two pointclouds.

normalize:
  If True the output feature values will be normalized using the sum for 
  'neighbors_importance' for each output point.  


max_temp_mem_MB:
  Defines the maximum temporary memory in megabytes to be used for the GPU 
  implementation. More memory means fewer kernel invocations. Note that the 
  a minimum amount of temp memory will always be allocated even if this
  variable is set to 0.


filters:
  The filter parameters. 
  The shape of the filter is [depth, height, width, in_ch, out_ch].
  The dimensions 'depth', 'height', 'width' define the spatial resolution of
  the filter. The spatial size of the filter is defined by the parameter
  'extents'.
    

inp_features:
  A 2D tensor which stores a feature vector for each input point.


inp_importance:
  An optional scalar importance for each input point. The features of each point
  will be multiplied with the corresponding value. The shape is [num input points].
  Use a zero length Tensor to disable.


neighbors_index:
  The neighbors_index stores a list of indices of neighbors for each output point as nested lists.
  The start and end of each list can be computed using 'neighbors_row_splits'.


neighbors_kernel_index:   
  Defines which kernel element to use for each neighbor. This array has the same length as neighbors_index.


neighbors_importance:
  Tensor of the same shape as 'neighbors_index' with a scalar value that is used to scale
  the features of each neighbor. Use a zero length Tensor to weigh each neighbor
  with 1.


neighbors_row_splits:
  The exclusive prefix sum of the neighbor count for the output points including
  the total neighbor count as the last element. The size of this array is the 
  number of output points + 1.


out_features:
  A Tensor with the output feature vectors for each output point.

)doc");
