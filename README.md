<h4 align="center">Automatic 3D model rigging and real-time Linear Mesh Blending avatar animation in C++ and Kinect.</h4>

<h1 align="center">
    <img src="https://raw.githubusercontent.com/DomainFlag/automatic_rigging_animation/master/Assets/animation.gif" alt="LBS animation" width=60%>
</h1>

## Key Features

* Kinect animation and caching
* 3D model animation based on LBS
* Automatic skeleton and bone weights generation
* Support for various 3D models: .obj .fbx .off, .ply and .stl file formats.
* Cross platform
  - Windows (only Kinect), macOS and Linux ready.

## Dependencies

The project was built / run on Windows, Visual Studio 2019. The [Desktop development with C++ bundle](https://visualstudio.microsoft.com/downloads/)
must include MSVC v142 - VS 2019 build tools, Windows 10 SDK, C++ MFC for MSVC v142.

- [FLTK](https://www.fltk.org) - library v1.3.5 is needed for rendering. Download and unzip it and follow
                              the [instructions](http://www.c-jump.com/bcc/common/Talk2/Cxx/FltkInstallVC/FltkInstallVC.html), set the appropriate env `FLTK_DIR`.

- [STB](https://github.com/nothings/stb) - is required for image texture loading. Clone the [project](https://github.com/nothings/stb),
                                           as it is a header library, set only the appropriate env `STB_DIR`.

- [Eigen](https://gitlab.com/libeigen/eigen) - is used for linear algebra related problems. Clone the [project](https://gitlab.com/libeigen/eigen),
                                               as it is a header library, set only the appropriate env `EIGEN_DIR`.

- [Kinect for Windows SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278) - allows skeleton tracking to cache animations and live animate a 3D model.
    Download and install the distribution and set `KINECTSDK10_DIR` if it's not defined to the installation path.

- [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2019-0) - is useful to load complex 3D models that contain textures, skeleton...
    Download and install the distribution for Windows and set `FBX_DIR` to the installation path or follow the following [instructions](http://help.autodesk.com/view/FBX/2017/ENU/?guid=__files_GUID_6E7B1A2E_584C_49C6_999E_CD8367841B7C_htm).

Older releases may work too...

## How To Use

Loading a 3D model that is automatically rigged based on the joints.out and animate live based on Kinect:

`data/human.obj data/joints.out data/skeleton.out non-rigged live`

`./Release/Project.exe data/maynard.obj data/joints.out data/skeleton.out non-rigged live`

```
../Release/Project.exe data/human.obj data/joints.out data/skeleton.out non-rigged live # failed Couldn't connect to the Kinect device...

../Release/Project.exe data/human.obj data/joints.out data/skeleton.out non-rigged non-live # success
Reading data/human.obj
Successfully read data/human.obj: 24461 vertices, 146754 edges
Reading data/skeleton.out
Done fullSplit 16777 8
Medial axis points = 9532
Extrem, fat verts 114 50
Matching!
Reached 62
Reached 181
Reached 411
Found: residual = 0.437984
E = 38.7441
E = 19.8095
E = 17.9145
E = 16.5821
E = 14.6483
E = 13.9067
E = 13.8859
E = 13.7791
E = 13.8767
E = 13.8129
Factoring size = 24461
Perm computed
Available Textures: 0

../Release/Project.exe data/maynard.obj data/joints.out data/skeleton.out non-rigged non-live
Reading data/maynard.obj
Successfully read data/maynard.obj: 18453 vertices, 110706 edges
Reading data/skeleton.out
Done fullSplit 18433 8
Medial axis points = 9664
Extrem, fat verts 192 50
Matching!
Reached 71
Reached 163
Reached 404
Reached 1107
Reached 2993
Reached 8122
Reached 22087
Reached 59884
Reached 162770
Reached 442460
Found: residual = 0.624948
E = 44.3175
E = 26.0662
E = 25.4368
E = 25.2084
E = 25.283
E = 24.9731
E = 24.8503
E = 24.7952
E = 24.6023
E = 24.6085
Factoring size = 18453
Perm computed
Available Textures: 0


../Release/Project.exe data/maynard-scale-x10-zoffset-12.1.obj data/joints.out data/skeleton.out non-rigged non-live
Reading data/maynard-scale-x10-zoffset-12.1.obj
Successfully read data/maynard-scale-x10-zoffset-12.1.obj: 18453 vertices, 110706 edges
Reading data/skeleton.out
Done fullSplit 18433 8
Medial axis points = 9664
Extrem, fat verts 192 50
Matching!
Reached 71
Reached 163
Reached 404
Reached 1107
Reached 2993
Reached 8122
Reached 22087
Reached 59884
Reached 162770
Reached 442460
Found: residual = 0.624948
E = 44.3175
E = 26.0662
E = 25.4368
E = 25.2084
E = 25.283
E = 24.9731
E = 24.8503
E = 24.7952
E = 24.6023
E = 24.6085
Factoring size = 18453
Perm computed
Available Textures: 0

Reading data/maynard-scale-x10-zoffset-12.1.obj
Successfully read data/maynard-scale-x10-zoffset-12.1.obj: 18453 vertices, 110706 edges
Reading data/skeleton.out
Done fullSplit 18433 8
Medial axis points = 9664
Extrem, fat verts 192 50
Matching!
Reached 69
Reached 159
Reached 406
Found: residual = 0.0530789
E = 34.6912
E = 23.7481
E = 22.5811
E = 22.4618
E = 22.4859
E = 22.4715
E = 22.2369
E = 22.0798
E = 22.1313
E = 21.9861
Factoring size = 18453
Perm computed
Available Textures: 0
Available Textures: 0
Available Textures: 0


../Release/Project.exe data/Body_f137.obj data/joints.out data/skeleton.out non-rigged non-live
Reading data/Body_f137.obj
Successfully read data/Body_f137.obj: 217642 vertices, 1305840 edges
Reading data/skeleton.out
Done fullSplit 17481 8
Medial axis points = 8773
Extrem, fat verts 105 50
Matching!
Reached 64
Reached 162
Found: residual = 0.0119537
E = 25.9419
E = 17.3615
E = 16.8642
E = 16.5936
E = 16.4988
E = 16.3269
E = 16.261
E = 16.3199
E = 16.2613
E = 16.3153
Factoring size = 217642
Perm computed
Available Textures: 0

../Release/Project.exe data/Ch46_nonPBR.fbx data/joints.out data/skeleton.out non-rigged non-live
Reading data/Ch46_nonPBR.fbx
Autodesk FBX SDK version 2019.0 Release (b92b15b23)
FBX file format version for this FBX SDK is 7.5.0
FBX file format version for file 'data/Ch46_nonPBR.fbx' is 7.7.0

Animation Stack Information
    Number of Animation Stacks: 2
    Current Animation Stack: "mixamo.com"

    Animation Stack 0
         Name: "Take 001"
         Description: ""
         Import Name: "Take 001"
         Import State: true

    Animation Stack 1
         Name: "mixamo.com"
         Description: ""
         Import Name: "mixamo.com"
         Import State: true

Geometry count: 1
Material count: 1
0. Control points count - 12456
0. Polygon count - 24564
0. Material count - 1
material name: Ch46_body
texture name: C:\Users\dong.yang\code\automatic-rigging-animation\Project\data\Ch46_nonPBR.fbm/Ch46_1001_Diffuse.png
Deformer count: 1
Cluster count: 65
    Cluster 0
    Mode: Normalize - mixamorig:Hips
    Cluster 0
    Mode: Normalize - mixamorig:Spine
    Cluster 0
    Mode: Normalize - mixamorig:Spine1
    Cluster 0
    Mode: Normalize - mixamorig:Spine2
    Cluster 0
    Mode: Normalize - mixamorig:Neck
    Cluster 0
    Mode: Normalize - mixamorig:Head
    Cluster 0
    Mode: Normalize - mixamorig:HeadTop_End


../Release/Project.exe data/mixamo-Ch14_nonPBR.fbx-clean-only-mesh.obj data/joints.out data/skeleton.out non-rigged non-live
../Release/Project.exe data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh.obj data/joints.out data/skeleton.out non-rigged non-live
../Release/Project.exe data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh.obj data/joints.out data/skeleton.out non-rigged non-live
../Release/Project.exe data/smpl.obj data/joints.out data/skeleton.out non-rigged non-live
../Release/Project.exe data/smpl_std.obj data/joints.out data/skeleton.out non-rigged non-live
../Release/Project.exe data/smpl_alvin.obj data/joints.out data/skeleton.out non-rigged non-live
Reading data/smpl_alvin.obj
Mesh integrity error: edges[i].twin >= 0 && edges[i].twin < es in line 693
Somehow read data/smpl_alvin.obj: 10475 vertices, 62724 edges
Reading data/skeleton.out

../Release/Project.exe data/doll.obj data/joints.out data/skeleton.out non-rigged non-live
PS C:\Users\dong.yang\code\automatic-rigging-animation\Project> ../Release/Project.exe data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh.obj data/joints.out data/skeleton.out non-rigged non-live
Reading data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh.obj
Mesh integrity error: edges[i].twin >= 0 && edges[i].twin < es in line 693
Somehow read data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh.obj: 12456 vertices, 73692 edges
Reading data/skeleton.out
PS C:\Users\dong.yang\code\automatic-rigging-animation\Project> ../Release/Project.exe data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh2.obj data/joints.out data/skeleton.out non-rigged non-live
Reading data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh2.obj
Error: duplicate edge detected: 10274 to 10220
Successfully read data/mixamo-Ch46_nonPBR.fbx-clean-only-mesh2.obj: 0 vertices, 0 edges
Reading data/skeleton.out
Bad mesh: should be a single connected component
PS C:\Users\dong.yang\code\automatic-rigging-animation\Project> ../Release/Project.exe data/Ch46_nonPBR.fbx data/joints.out data/skeleton.out non-rigged non-live
Reading data/Ch46_nonPBR.fbx
Autodesk FBX SDK version 2019.0 Release (b92b15b23)
FBX file format version for this FBX SDK is 7.5.0
FBX file format version for file 'data/Ch46_nonPBR.fbx' is 7.7.0

Animation Stack Information
    Number of Animation Stacks: 2
    Current Animation Stack: "mixamo.com"

    Animation Stack 0
         Name: "Take 001"
         Description: ""
         Import Name: "Take 001"
         Import State: true

    Animation Stack 1
         Name: "mixamo.com"
         Description: ""
         Import Name: "mixamo.com"
         Import State: true

Geometry count: 1
Material count: 1
0. Control points count - 12456
0. Polygon count - 24564
0. Material count - 1
material name: Ch46_body
texture name: C:\Users\dong.yang\code\automatic-rigging-animation\Project\data\Ch46_nonPBR.fbm/Ch46_1001_Diffuse.png
Deformer count: 1
Cluster count: 65
    Cluster 0
    Mode: Normalize - mixamorig:Hips
    Cluster 0
    Mode: Normalize - mixamorig:Spine
    Cluster 0
    Mode: Normalize - mixamorig:Spine1
    Cluster 0
    Mode: Normalize - mixamorig:Spine2
    Cluster 0
    Mode: Normalize - mixamorig:Neck
    Cluster 0
    Mode: Normalize - mixamorig:Head
    Cluster 0
    Mode: Normalize - mixamorig:HeadTop_End
PS C:\Users\dong.yang\code\automatic-rigging-animation\Project> ../Release/Project.exe data/Ch46_nonPBR_remove_amature.fbx data/joints.out data/skeleton.out non-rigged non-live
Reading data/Ch46_nonPBR_remove_amature.fbx
Autodesk FBX SDK version 2019.0 Release (b92b15b23)
FBX file format version for this FBX SDK is 7.5.0
FBX file format version for file 'data/Ch46_nonPBR_remove_amature.fbx' is 7.4.0

Animation Stack Information
    Number of Animation Stacks: 0
    Current Animation Stack: ""

Geometry count: 1
Material count: 1
0. Control points count - 12456
0. Polygon count - 24564
0. Material count - 1
material name: Ch46_body
texture name: C:\Users\dong.yang\code\automatic-rigging-animation\Project\data\Ch46_1001_Diffuse.png
Mesh integrity error: edges[i].twin >= 0 && edges[i].twin < es in line 693
Somehow read data/Ch46_nonPBR_remove_amature.fbx: 12456 vertices, 73692 edges
Reading data/skeleton.out
```

Loading a 3D model that is rigged and animated based on cached animation skeleton.out:

`data/model.fbx data/joints.out data/skeleton.out rigged non-live`

Note: Run in release mode to speed up things. To set up the command arguments in VS2019, select `Project project > Properties > Configuration Properties > Debugging > Command Arguments`

## Interaction

- Press `s` to toggle skeleton, `g` for floor, `f` for flat shading rendering.
- Use mouse wheel or by mouse dragging to move the camera.

## Miscellaneous

The file `data/joints.out` contain the definition of the desired base skeleton (human, monkey, koala...). The loaded 3D model
is embedded / scaled into [-1, 1] world space, thus each line contains an unique joint, its desired / expected 3D location,
and his parent joint. Specify -1 if the corresponding joint is the root.

The motion file `data/skeleton.out` is based on our custom motion format, where the skeleton
is tracked each frame with first line representing [20 joints (Kinect v1)](https://de.mathworks.com/help/supportpkg/kinectforwindowsruntime/ug/use-skeleton-viewer-for-kinect-v1-skeletal-data.html)
world positions and 20 new lines each representing the `index of parent joint`, `index of child joint`, `absolute quaternion rotation w.t.r Y-axis for joints world orientation`

## Credits

The rigging is based on:

- [Pinocchio](https://github.com/elrond79/Pinocchio) - please refer also the paper for extra information about rigging process.
