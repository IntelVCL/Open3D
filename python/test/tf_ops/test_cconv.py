# ----------------------------------------------------------------------------
# -                        Open3D: www.open3d.org                            -
# ----------------------------------------------------------------------------
# The MIT License (MIT)
#
# Copyright (c) 2020 www.open3d.org
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
# ----------------------------------------------------------------------------
"""Tests the reference python implementation of the continuous conv"""

import open3d as o3d
import numpy as np
np.set_printoptions(linewidth=600)
np.set_printoptions(threshold=np.inf)
import pytest
import mark_helper
from check_gradients import check_gradients
# from cconv_python import *

# the tests in this file require the tensorflow ops
pytest.mark.skipif(not o3d._build_config['BUILD_TENSORFLOW_OPS'],
                   reason='tf ops not built'),


# yapf: disable
# @pytest.mark.skip()
@pytest.mark.parametrize("filter_size, out_channels, in_channels, with_inp_importance, with_normalization",[
                             ([3,5,1],            2,           7,                True,              False),
                             ([3,3,3],            1,           1,               False,              False),
                             ([5,5,5],            5,           3,               False,               True),
                        ])
# yapf: enable
@mark_helper.devices
@pytest.mark.parametrize('dtype', [np.float32])
def test_compare_to_conv3d(dtype, device_name, filter_size, out_channels,
                           in_channels, with_inp_importance,
                           with_normalization):
    """Compares to the 3D convolution in tensorflow"""
    import tensorflow as tf
    import open3d.ml.tf as ml3d
    np.random.seed(0)

    conv_attrs = {
        'align_corners': False,
        'coordinate_mapping': 'identity',
        'normalize': with_normalization,
        'interpolation': 'nearest_neighbor',
        'max_temp_mem_MB': 0,
    }

    filters = np.random.random(size=(*filter_size, in_channels,
                                     out_channels)).astype(dtype)

    max_grid_extent = 10
    inp_positions = np.unique(np.random.randint(0, max_grid_extent,
                                                (256, 3)).astype(dtype),
                              axis=0)
    inp_positions_int = inp_positions.astype(np.int32)
    if (with_inp_importance):
        inp_importance = np.random.rand(
            inp_positions.shape[0]).astype(dtype) - 0.5
    else:
        inp_importance = np.empty((0,))
    out_positions = np.unique(np.random.randint(
        np.max(filter_size) // 2, max_grid_extent - np.max(filter_size) // 2,
        (5, 3)).astype(dtype),
                              axis=0)
    out_positions_int = out_positions.astype(np.int32)

    voxel_size = np.array([1, 1, 1], dtype=dtype)
    voxel_offset = np.array([0, 0, 0], dtype=dtype)
    extent = voxel_size[np.newaxis, :] * np.array(filter_size[::-1])
    offset = np.array([0.0, 0.0, 0.0], dtype=dtype)

    inp_features = np.random.uniform(size=inp_positions.shape[0:1] +
                                     (in_channels,)).astype(dtype)
    fixed_radius_search = ml3d.layers.FixedRadiusSearch(metric='Linf')
    neighbors_index, neighbors_row_splits, _ = fixed_radius_search(
        inp_positions / extent, out_positions / extent,
        voxel_size[0] / 2 + 0.01)
    neighbors_index = neighbors_index.numpy()
    neighbors_row_splits = neighbors_row_splits.numpy()

    neighbors_importance = np.empty((0,))

    with tf.device(device_name):
        y = ml3d.ops.continuous_conv(filters, out_positions, extent, offset,
                                     inp_positions, inp_features,
                                     inp_importance, neighbors_index,
                                     neighbors_importance, neighbors_row_splits,
                                     **conv_attrs)
        assert device_name in y.device
        y = y.numpy()

    # Compare the output to a standard 3d conv
    # store features in a volume to use standard 3d convs
    inp_volume = np.zeros(
        (1, max_grid_extent, max_grid_extent, max_grid_extent, in_channels))

    if with_inp_importance:
        inp_features *= inp_importance[:, np.newaxis]
    inp_volume[0, inp_positions_int[:, 2], inp_positions_int[:, 1],
               inp_positions_int[:, 0], :] = inp_features

    y_conv3d = tf.nn.conv3d(
        inp_volume,
        filters,
        strides=[1] * 5,
        padding='SAME',
    ).numpy()

    # extract result at output positions
    y_conv3d = np.ascontiguousarray(
        y_conv3d[0, out_positions_int[:, 2], out_positions_int[:, 1],
                 out_positions_int[:, 0], :])

    if with_normalization:
        for i, v in enumerate(y_conv3d):
            num_neighbors = neighbors_row_splits[i +
                                                 1] - neighbors_row_splits[i]
            v /= dtype(num_neighbors)

    np.testing.assert_allclose(y, y_conv3d, rtol=1e-5, atol=1e-8)


# yapf: disable
# @pytest.mark.skip()
@pytest.mark.parametrize("filter_size, out_channels, in_channels, with_inp_importance, with_neighbors_importance, with_individual_extent, with_normalization, align_corners, coordinate_mapping, interpolation",[
                             ([3,5,1],            2,           7,                True,                     False,                  False,              False,          True,        'identity', 'nearest_neighbor'),
                             ([3,3,3],            1,           1,               False,                     False,                   True,              False,         False, 'ball_to_cube_radial',       'linear'),
                             ([5,5,5],            5,           3,               False,                      True,                  False,              True, False, 'ball_to_cube_volume_preserving', 'linear_border'),
                             ([5,1,3],            3,           4,               False,                      True,                  False,              False,         False,           'identity',        'linear'),
                        ])
# yapf: enable
@mark_helper.devices
@pytest.mark.parametrize('dtype', [np.float32])
def test_cconv_gradient(dtype, device_name, filter_size, out_channels,
                        in_channels, with_inp_importance,
                        with_neighbors_importance, with_individual_extent,
                        with_normalization, align_corners, coordinate_mapping,
                        interpolation):
    import tensorflow as tf
    import open3d.ml.tf as ml3d

    if dtype == np.float64:
        tolerance = {'atol': 1e-5, 'rtol': 1e-2, 'epsilon': 1e-6}
    elif dtype == np.float32:
        tolerance = {'atol': 1e-2, 'rtol': 1e-1, 'epsilon': 1e-3}

    np.random.seed(0)

    conv_attrs = {
        'align_corners': align_corners,
        'coordinate_mapping': coordinate_mapping,
        'normalize': with_normalization,
        'interpolation': interpolation,
    }

    filters = np.random.random(size=(*filter_size, in_channels,
                                     out_channels)).astype(dtype)

    inp_positions = np.random.rand(128, 3).astype(dtype)
    if (with_inp_importance):
        inp_importance = np.random.rand(inp_positions.shape[0]).astype(dtype)
    else:
        inp_importance = np.empty((0,))

    out_positions = np.random.rand(5, 3).astype(dtype)

    if with_individual_extent:
        extent = 0.4 + 0.01 * (np.random.rand(out_positions.shape[0], 1) - 0.5)
    else:
        extent = np.array([[0.4]], dtype=dtype)
    offset = np.array([0.0, 0.0, 0.0], dtype=dtype)

    inp_features = np.random.uniform(size=inp_positions.shape[0:1] +
                                     (in_channels,)).astype(dtype)
    fixed_radius_search = ml3d.layers.FixedRadiusSearch(metric='Linf')
    neighbors_index, neighbors_row_splits, _ = fixed_radius_search(
        inp_positions, out_positions, extent[0, 0] / 2)
    neighbors_index = neighbors_index.numpy()
    neighbors_row_splits = neighbors_row_splits.numpy()

    if (with_neighbors_importance):
        neighbors_importance = np.random.rand(
            neighbors_index.shape[0]).astype(dtype) - 0.5

        neighbors_importance_sum = ml3d.ops.reduce_subarrays_sum(
            neighbors_importance, neighbors_row_splits)
    else:
        neighbors_importance = np.empty((0,), dtype=dtype)
        neighbors_importance_sum = np.empty((0,), dtype=dtype)

    inverted_neighbors_index, inverted_neighbors_row_splits, inverted_neighbors_importance = ml3d.ops.invert_neighbors_list(
        inp_positions.shape[0], neighbors_index, neighbors_row_splits,
        neighbors_importance)

    inverted_neighbors_index = inverted_neighbors_index.numpy()
    inverted_neighbors_row_splits = inverted_neighbors_row_splits.numpy()
    inverted_neighbors_importance = inverted_neighbors_importance.numpy()

    # print(neighbors_row_splits, inverted_neighbors_row_splits)
    # print(neighbors_index, inverted_neighbors_index)

    # define functions for the gradient checker
    def conv_infeats(inp_features):
        with tf.device(device_name):
            y = ml3d.ops.continuous_conv(filters, out_positions, extent, offset,
                                         inp_positions, inp_features,
                                         inp_importance, neighbors_index,
                                         neighbors_importance,
                                         neighbors_row_splits, **conv_attrs)
            assert device_name in y.device
            return y.numpy()

    def conv_filter(filter):
        with tf.device(device_name):
            y = ml3d.ops.continuous_conv(filter, out_positions, extent, offset,
                                         inp_positions, inp_features,
                                         inp_importance, neighbors_index,
                                         neighbors_importance,
                                         neighbors_row_splits, **conv_attrs)
            assert device_name in y.device
            return y.numpy()

    def conv_filter_backprop(out_features_gradient, filter):
        x = tf.constant(filter)
        with tf.GradientTape() as tape:
            tape.watch(x)
            y = ml3d.ops.continuous_conv(x, out_positions, extent, offset,
                                         inp_positions, inp_features,
                                         inp_importance, neighbors_index,
                                         neighbors_importance,
                                         neighbors_row_splits, **conv_attrs)
            dy_dx = tape.gradient(y, x, out_features_gradient)
        return dy_dx.numpy()

    def conv_infeat_backprop(out_features_gradient, inp_features):
        x = tf.constant(inp_features)
        with tf.GradientTape() as tape:
            tape.watch(x)
            y = ml3d.ops.continuous_conv(filters, out_positions, extent, offset,
                                         inp_positions, x, inp_importance,
                                         neighbors_index, neighbors_importance,
                                         neighbors_row_splits, **conv_attrs)
            dy_dx = tape.gradient(y, x, out_features_gradient)
        return dy_dx.numpy()

    def conv_transpose_filter(filter):
        with tf.device(device_name):
            y = ml3d.ops.continuous_conv_transpose(
                filter, inp_positions, inp_importance, extent, offset,
                out_positions, y_arr, neighbors_index, neighbors_importance_sum,
                neighbors_row_splits, inverted_neighbors_index,
                inverted_neighbors_importance, inverted_neighbors_row_splits,
                **conv_attrs)
            assert device_name in y.device
            return y.numpy()

    def conv_transpose_infeats(inp_features):
        with tf.device(device_name):
            y = ml3d.ops.continuous_conv_transpose(
                filters.transpose([0, 1, 2, 4, 3]), inp_positions,
                inp_importance, extent, offset, out_positions, inp_features,
                neighbors_index, neighbors_importance_sum, neighbors_row_splits,
                inverted_neighbors_index, inverted_neighbors_importance,
                inverted_neighbors_row_splits, **conv_attrs)
            assert device_name in y.device
            return y.numpy()

    def conv_transpose_filter_backprop(out_features_gradient, filter):
        x = tf.constant(filter)
        with tf.GradientTape() as tape:
            tape.watch(x)
            y = ml3d.ops.continuous_conv_transpose(
                x, inp_positions, inp_importance, extent, offset, out_positions,
                y_arr, neighbors_index, neighbors_importance_sum,
                neighbors_row_splits, inverted_neighbors_index,
                inverted_neighbors_importance, inverted_neighbors_row_splits,
                **conv_attrs)
            dy_dx = tape.gradient(y, x, out_features_gradient)
        return dy_dx.numpy()

    def conv_transpose_infeat_backprop(out_features_gradient, inp_features):
        x = tf.constant(inp_features)
        with tf.GradientTape() as tape:
            tape.watch(x)
            y = ml3d.ops.continuous_conv_transpose(
                filters.transpose([0, 1, 2, 4, 3]), inp_positions,
                inp_importance, extent, offset, out_positions, x,
                neighbors_index, neighbors_importance_sum, neighbors_row_splits,
                inverted_neighbors_index, inverted_neighbors_importance,
                inverted_neighbors_row_splits, **conv_attrs)
            dy_dx = tape.gradient(y, x, out_features_gradient)
        return dy_dx.numpy()

    y_arr = conv_infeats(inp_features)

    dbg = {}
    filter_gradient_OK = check_gradients(filters,
                                         conv_filter,
                                         conv_filter_backprop,
                                         debug_outputs=dbg,
                                         **tolerance)
    assert filter_gradient_OK

    feature_gradient_OK = check_gradients(inp_features,
                                          conv_infeats,
                                          conv_infeat_backprop,
                                          debug_outputs=dbg,
                                          **tolerance)
    assert feature_gradient_OK

    transpose_filter_gradient_OK = check_gradients(
        filters.transpose([0, 1, 2, 4, 3]),
        conv_transpose_filter,
        conv_transpose_filter_backprop,
        debug_outputs=dbg,
        **tolerance)
    assert transpose_filter_gradient_OK

    transpose_feature_gradient_OK = check_gradients(
        y_arr,
        conv_transpose_infeats,
        conv_transpose_infeat_backprop,
        debug_outputs=dbg,
        **tolerance)
    assert transpose_feature_gradient_OK
