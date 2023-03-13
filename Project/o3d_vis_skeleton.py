# ----------------------------------------------------------------------------
# -                        Open3D: www.open3d.org                            -
# ----------------------------------------------------------------------------
# The MIT License (MIT)
#
# Copyright (c) 2018-2021 www.open3d.org
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

# examples/python/visualization/non_blocking_visualization.py

import open3d as o3d
import numpy as np
import copy
import json

if __name__ == "__main__":
    vis = o3d.visualization.Visualizer()
    vis.create_window()
    threshold = 0.05
    icp_iteration = 100
    save_image = True
    filepath = R"C:\Users\dong.yang\code\automatic-rigging-animation\Project\fineEmbeddingAll.json"
    with open(filepath) as f:
        skeleton_vertices = json.load(f)

    filepath = R"C:\Users\dong.yang\code\automatic-rigging-animation\Project\skeleton.json"
    with open(filepath) as f:
        skeleton_maps = json.load(f)

    verts = skeleton_vertices["all"][0][1]

    edges = []
    for i in range(len(skeleton_maps["fPrev"])):
        if skeleton_maps["fPrev"][i] < 0:
            continue
        edges.append([i, skeleton_maps["fPrev"][i]])

    pcd = o3d.geometry.PointCloud()
    # pcd.points = o3d.utility.Vector3dVector(verts)
    pcd = o3d.geometry.LineSet()
    pcd.points = o3d.utility.Vector3dVector(verts)
    pcd.lines = o3d.utility.Vector2iVector(edges)
    color = [0,1,0]
    colors = []
    for i in range(len(pcd.points)):
        colors.append(color)
    # pcd.paint_uniform_color(color)
    pcd.colors = o3d.utility.Vector3dVector(colors)
    vis.add_geometry(pcd)

    faces = []
    for i in range(len(skeleton_vertices["all"])):
        for j in range(len(skeleton_vertices["all"][0])):
            if not skeleton_vertices["all"][i][j]:
                continue
            verts = skeleton_vertices["all"][i][j]
            pcd.points = o3d.utility.Vector3dVector(verts)
            vis.update_geometry(pcd)
            vis.poll_events()
            vis.update_renderer()
            if save_image:
                vis.capture_screen_image("temp_%04d.jpg" % i)
    vis.destroy_window()