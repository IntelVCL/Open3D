import open3d as o3d
import open3d.open3d_pybind as open3d_pybind
import numpy as np


def _numpy_dtype_to_dtype(numpy_dtype):
    if numpy_dtype == np.float32:
        return o3d.Dtype.Float32
    elif numpy_dtype == np.float64:
        return o3d.Dtype.Float64
    elif numpy_dtype == np.int32:
        return o3d.Dtype.Int32
    elif numpy_dtype == np.int64:
        return o3d.Dtype.Int64
    elif numpy_dtype == np.uint8:
        return o3d.Dtype.UInt8
    elif numpy_dtype == np.bool:
        return o3d.Dtype.Bool
    else:
        raise ValueError("Unsupported numpy dtype:", numpy_dtype)


class SizeVector(open3d_pybind.SizeVector):

    def __init__(self, values=None):
        if values is None:
            values = []
        # TODO: determine whether conversion can be done in C++ as well.
        if isinstance(values, tuple) or isinstance(values, list):
            values = np.array(values)
        if not isinstance(values, np.ndarray) or values.ndim != 1:
            raise ValueError(
                "SizeVector only takes 1-D list, tuple or Numpy array.")
        super(SizeVector, self).__init__(values.astype(np.int64))


def cast_to_py_tensor(func):
    """
    Args:
        func: function returning a `o3d.open3d_pybind.Tensor`.

    Return:
        A function which returns a python object `o3d.Tensor`.
    """

    def wrapped_func(self, *args, **kwargs):
        result = func(self, *args, **kwargs)
        wrapped_result = Tensor([])
        wrapped_result.shallow_copy_from(result)
        return wrapped_result

    return wrapped_func


def _to_o3d_tensor_key(key):

    if isinstance(key, int):
        return o3d.open3d_pybind.TensorKey.index(key)
    elif isinstance(key, slice):
        return o3d.open3d_pybind.TensorKey.slice(
            o3d.none if key.start == None else key.start,
            o3d.none if key.stop == None else key.stop,
            o3d.none if key.step == None else key.step)
    else:
        raise TypeError(f"Invalid key type {type(key)}.")


class Tensor(open3d_pybind.Tensor):
    """
    Open3D Tensor class. A Tensor is a view of data blob with shape, strides
    and etc. Tensor can be used to perform numerical operations.
    """

    def __init__(self, data, dtype=None, device=None):
        if isinstance(data, tuple) or isinstance(data, list):
            data = np.array(data)
        if not isinstance(data, np.ndarray):
            raise ValueError("data must be a list, tuple, or Numpy array.")
        if dtype is None:
            dtype = _numpy_dtype_to_dtype(data.dtype)
        if device is None:
            device = o3d.Device("CPU:0")
        super(Tensor, self).__init__(data, dtype, device)

    @cast_to_py_tensor
    def __getitem__(self, key):
        t = self
        if isinstance(key, tuple) or isinstance(key, list):
            t = super(Tensor, self)._getitem_vector(
                [_to_o3d_tensor_key(k) for k in key])
        elif isinstance(key, int) or isinstance(key, slice):
            t = super(Tensor, self)._getitem(_to_o3d_tensor_key(key))
        else:
            raise TypeError(f"Invalid type {type(key)} for getitem.")
        return t

    @cast_to_py_tensor
    def __setitem__(self, key, value):
        t = self.__getitem__(key)
        super(Tensor, t)._setitem(value)
        return self

    @cast_to_py_tensor
    def cuda(self, device_id=0):
        """
        Returns a copy of this tensor in CUDA memory.

        Args:
            device_id: CUDA device id.
        """
        return super(Tensor, self).cuda(device_id)

    @cast_to_py_tensor
    def cpu(self):
        """
        Returns a copy of this tensor in CPU.

        If the Tensor is already in CPU, then no copy is performed.
        """
        return super(Tensor, self).cpu()

    def numpy(self):
        """
        Returns this tensor as a NumPy array. This tensor must be a CPU tensor,
        and the returned NumPy array shares the same memory as this tensor.
        Changes to the NumPy array will be reflected in the original tensor and
        vice versa.
        """
        return super(Tensor, self).numpy()

    @staticmethod
    @cast_to_py_tensor
    def from_numpy(np_array):
        """
        Returns a Tensor from NumPy array. The resulting tensor is a CPU tensor
        that shares the same memory as the NumPy array. Changes to the tensor
        will be reflected in the original NumPy array and vice versa.

        Args:
            np_array: The Numpy array to be converted from.
        """
        return super(Tensor, Tensor).from_numpy(np_array)

    def to_dlpack(self):
        """
        Returns a DLPack PyCapsule representing this tensor.
        """
        return super(Tensor, self).to_dlpack()

    @staticmethod
    @cast_to_py_tensor
    def from_dlpack(dlpack):
        """
        Returns a tensor converted from DLPack PyCapsule.
        """
        return super(Tensor, Tensor).from_dlpack(dlpack)

    @cast_to_py_tensor
    def add(self, value):
        """
        Adds a tensor and returns the resulting tensor.
        """
        return super(Tensor, self).add(value)

    @cast_to_py_tensor
    def add_(self, value):
        """
        Inplace version of Tensor.add.
        """
        return super(Tensor, self).add_(value)

    @cast_to_py_tensor
    def sub(self, value):
        """
        Substracts a tensor and returns the resulting tensor.
        """
        return super(Tensor, self).sub(value)

    @cast_to_py_tensor
    def sub_(self, value):
        """
        Inplace version of Tensor.sub.
        """
        return super(Tensor, self).sub_(value)

    @cast_to_py_tensor
    def mul(self, value):
        """
        Multiplies a tensor and returns the resulting tensor.
        """
        return super(Tensor, self).mul(value)

    @cast_to_py_tensor
    def mul_(self, value):
        """
        Inplace version of Tensor.mul.
        """
        return super(Tensor, self).mul_(value)

    @cast_to_py_tensor
    def div(self, value):
        """
        Divides a tensor and returns the resulting tensor.
        """
        return super(Tensor, self).div(value)

    @cast_to_py_tensor
    def div_(self, value):
        """
        Inplace version of Tensor.div.
        """
        return super(Tensor, self).div_(value)

    @cast_to_py_tensor
    def abs(self):
        """
        Returns element-wise absolute value of a tensor.
        """
        return super(Tensor, self).abs()

    @cast_to_py_tensor
    def abs_(self):
        """
        Inplace version of Tensor.abs.
        """
        return super(Tensor, self).abs_()

    @cast_to_py_tensor
    def logical_and(self, value):
        """
        Element-wise logical and operation.

        If the tensor is not boolean, zero will be treated as False, while
        non-zero values will be treated as True.
        """
        return super(Tensor, self).logical_and(value)

    @cast_to_py_tensor
    def logical_and_(self, value):
        """
        Inplace version of Tensor.logical_and.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).logical_and_(value)

    @cast_to_py_tensor
    def logical_or(self, value):
        """
        Element-wise logical or operation.

        If the tensor is not boolean, zero will be treated as False, while
        non-zero values will be treated as True.
        """
        return super(Tensor, self).logical_or(value)

    @cast_to_py_tensor
    def logical_or_(self, value):
        """
        Inplace version of Tensor.logical_or.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).logical_or_(value)

    @cast_to_py_tensor
    def logical_xor(self, value):
        """
        Element-wise logical exclusive-or operation.

        If the tensor is not boolean, zero will be treated as False, while
        non-zero values will be treated as True.
        """
        return super(Tensor, self).logical_xor(value)

    @cast_to_py_tensor
    def logical_xor_(self, value):
        """
        Inplace version of Tensor.logical_xor.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).logical_xor_(value)

    @cast_to_py_tensor
    def gt(self, value):
        """
        Element-wise greater than operation, returning a new boolean tensor.
        """
        return super(Tensor, self).gt(value)

    @cast_to_py_tensor
    def gt_(self, value):
        """
        Inplace version of Tensor.gt.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).gt_(value)

    @cast_to_py_tensor
    def lt(self, value):
        """
        Element-wise less than operation, returning a new boolean tensor.
        """
        return super(Tensor, self).lt(value)

    @cast_to_py_tensor
    def lt_(self, value):
        """
        Inplace version of Tensor.lt.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).lt_(value)

    @cast_to_py_tensor
    def ge(self, value):
        """
        Element-wise greater-than-or-equals-to operation, returning a new
        boolean tensor.
        """
        return super(Tensor, self).ge(value)

    @cast_to_py_tensor
    def ge_(self, value):
        """
        Inplace version of Tensor.ge.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).ge_(value)

    @cast_to_py_tensor
    def le(self, value):
        """
        Element-wise less-than-or-equals-to than operation, returning a new
        boolean tensor.
        """
        return super(Tensor, self).le(value)

    @cast_to_py_tensor
    def le_(self, value):
        """
        Inplace version of Tensor.le.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).le_(value)

    @cast_to_py_tensor
    def eq(self, value):
        """
        Element-wise equal operation, returning a new boolean tensor.
        """
        return super(Tensor, self).eq(value)

    @cast_to_py_tensor
    def eq_(self, value):
        """
        Inplace version of Tensor.eq.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).eq_(value)

    @cast_to_py_tensor
    def ne(self, value):
        """
        Element-wise not-equal operation, returning a new boolean tensor.
        """
        return super(Tensor, self).ne(value)

    @cast_to_py_tensor
    def ne_(self, value):
        """
        Inplace version of Tensor.ne.

        This operation won't change the tensor's dtype.
        """
        return super(Tensor, self).ne_(value)

    @cast_to_py_tensor
    def to(self, dtype, copy=False):
        """
        Returns a tensor with the specified dtype.

        Args:
            dtype: The targeted dtype to convert to.
            copy: If true, a new tensor is always created; if false, the copy
                  is avoided when the original tensor already have the targeted
                  dtype.
        """
        return super(Tensor, self).to(dtype, copy)

    def __add__(self, value):
        return self.add(value)

    def __iadd__(self, value):
        return self.add_(value)

    def __sub__(self, value):
        return self.sub(value)

    def __isub__(self, value):
        return self.sub_(value)

    def __mul__(self, value):
        return self.mul(value)

    def __imul__(self, value):
        return self.mul_(value)

    def __truediv__(self, value):
        # True div and floor div are the same for Tensor.
        return self.div(value)

    def __itruediv__(self, value):
        # True div and floor div are the same for Tensor.
        return self.div_(value)

    def __floordiv__(self, value):
        # True div and floor div are the same for Tensor.
        return self.div(value)

    def __ifloordiv__(self, value):
        # True div and floor div are the same for Tensor.
        return self.div_(value)

    def __lt__(self, value):
        return self.lt(value)

    def __le__(self, value):
        return self.le(value)

    def __eq__(self, value):
        return self.eq(value)

    def __ne__(self, value):
        return self.ne(value)

    def __gt__(self, value):
        return self.gt(value)

    def __ge__(self, value):
        return self.ge(value)


class TensorList(open3d_pybind.TensorList):
    """
    Open3D TensorList class. A TensorList is an extendable tensor at the 0-th dimension.
    It is similar to python list, but uses Open3D's tensor memory management system.
    """

    def __init__(self, shape, dtype=None, device=None, size=None):
        if isinstance(shape, list) or isinstance(shape, tuple):
            shape = o3d.SizeVector(shape)
        elif isinstance(shape, o3d.SizeVector):
            pass
        else:
            raise ValueError('shape must be a list, tuple, or o3d.SizeVector')

        if dtype is None:
            dtype = _numpy_dtype_to_dtype(data.dtype)
        if device is None:
            device = o3d.Device("CPU:0")
        if size is None:
            size = 0

        super(TensorList, self).__init__(shape, dtype, device, size)

    @staticmethod
    def from_tensor(tensor, inplace=False):
        """
        Returns a TensorList from an existing tensor.
        Args:
            tensor: The internal o3d.Tensor to construct from, whose 0-th dimension
                    corresponds to the size of the tensorlist.
            inplace: Reuse tensor memory in place if True, else make a copy.
        """

        if not isinstance(tensor, o3d.Tensor):
            raise ValueError('tensor must be a o3d.Tensor')

        return super(TensorList, TensorList).from_tensor(tensor, inplace)

    @staticmethod
    def from_tensors(tensors, device=None):
        """
        Returns a TensorList from a list of existing tensors.
        Args:
            tensors: The list of o3d.Tensor to construct from.
                     The tensors' shapes should be compatible.
            device: The device where the tensorlist is targeted.
        """

        if not isinstance(tensors, list) and not isinstance(tensors, tuple):
            raise ValueError('tensors must be a list or tuple')

        for tensor in tensors:
            if not isinstance(tensor, o3d.Tensor):
                raise ValueError(
                    'every element of the input list must be a valid tensor')
        if device is None:
            device = o3d.Device("CPU:0")

        return super(TensorList, TensorList).from_tensors(tensors, device)
