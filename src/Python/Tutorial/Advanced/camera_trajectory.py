# Open3D: www.open3d.org
# The MIT License (MIT)
# See license file or visit www.open3d.org for details

import os
import numpy as np
from open3d import *

if __name__ == "__main__":

    print("Testing camera in open3d ...")
    intrinsic = PinholeCameraIntrinsic(
            PinholeCameraIntrinsicParameters.PrimeSenseDefault)
    print(intrinsic.intrinsic_matrix)
    print(PinholeCameraIntrinsic())
    x = PinholeCameraIntrinsic(640, 480, 525, 525, 320, 240)
    print(x)
    print(x.intrinsic_matrix)
    write_pinhole_camera_intrinsic("test.json", x)
    y = read_pinhole_camera_intrinsic("test.json")
    print(y)
    print(np.asarray(y.intrinsic_matrix))

    print("Read a trajectory and combine all the RGB-D images.")
    pcds = []
    data_dir = "../../TestData/RGBD"
    trajectory = read_pinhole_camera_trajectory(os.path.join(data_dir, "trajectory.log"))
    write_pinhole_camera_trajectory("test.json", trajectory)
    print(trajectory)
    print(trajectory.extrinsic)
    print(np.asarray(trajectory.extrinsic))
    for i in range(5):
        color = read_image(os.path.join(data_dir, "color/{:05d}.jpg".format(i)))
        depth = read_image(os.path.join(data_dir, "depth/{:05d}.png".format(i)))
        im = create_rgbd_image_from_color_and_depth(color, depth, 1000.0, 5.0, False)
        pcd = create_point_cloud_from_rgbd_image(im,
                trajectory.intrinsic, trajectory.extrinsic[i])
        pcds.append(pcd)
    draw_geometries(pcds)
    print("")
