import numpy as np
import json

"""
了解如何定义和创建网格对于在 Blender 中编写几何脚本至关重要。该过程相当简单，需要用户定义以下网格属性：

顶点（由 X、Y 和 Z 定义的点）
边（由顶点索引定义的线框曲线）
面（由顶点索引定义的 3D 表面）
在下面这个例子中，我们将定义一个简单的平面来演示顶点和面索引之间的关系。平面将由四个点组成。

首先我们定义顶点和面变量……这些被定义为数组。
"""

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

faces = []
# faces = skeleton_maps["fGraphV"]["edges"]

import bpy

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False, confirm=False)

def add_camera(location=(0.5, 0.5, 3), rotation=(0, 0, 0)):
    bpy.ops.object.camera_add(enter_editmode=False, align='VIEW', location=location, rotation=rotation)
    camera = bpy.context.object
    return camera

def add_light(location=(0.5, 0.5, 3)):
    bpy.ops.object.light_add(type='SUN', location=location)
    sun = bpy.context.object
    return sun

"""
我们现在需要为网格和场景对象定义变量……
"""
#Define mesh and object
mymesh = bpy.data.meshes.new("Plane")
 
#the mesh variable is then referenced by the object variable
myobject = bpy.data.objects.new("Plane", mymesh)

"""
我们现在来定义网格的创建位置。我们可以设置要在光标位置创建的网格……
"""
#Set location and scene of object
myobject.location = bpy.context.scene.cursor.location
bpy.context.collection.objects.link(myobject)

camera = add_camera()
bpy.context.scene.camera = camera
camera.select_set(True)
light = add_light()

"""
现在我们可以创建网格了……
"""
#Create mesh
# this method has an optional 'edge' array input. This is left as an empty array
mymesh.from_pydata(verts,edges,faces) 
mymesh.update(calc_edges=True) #so the edges display properly...

bpy.context.scene.render.image_settings.file_format = 'PNG'
bpy.context.scene.render.filepath = "/tmp/temp_"
bpy.context.scene.frame_end = 2

bpy.ops.render.render(animation=True, use_viewport=True)
