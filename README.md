# OsgObjects
objects using osg and imgui as ui

# imguiOsg
使用imgui做界面，osg做渲染显示。

为了跨平台编译不折腾打算从Qt换成imgui+osg. 测试平台用的是MacBook Pro (13-inch, 2019, Two Thunderbolt 3 ports)，系统用的是macOS Big Sur。

在win10上正常，但在mac上报不支持glsl 130. 试了下120可以，但130就是会报错。
```c++
ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile vertex shader! With GLSL: #version 130

ERROR: 0:1: '' :  version '130' is not supported
```

# imguiShader
使用OpenGL+glfw+imgui做界面及渲染。

# osgReconFrame
[测试数据集-20-thermocolorlab.tar](https://robotik.informatik.uni-wuerzburg.de/telematics/3dscans/)

从点云中单帧重建三维网格模型。

* 输入：
    * 激光姿态.pose文件，包含两行共6个数字，第一行三个数表示激光位置，第二行三个数表示激光旋转角度（相对世界坐标系）
    * 点云.obj文件，激光姿态.pose文件对应的点云 以thermocolorlab.tar为样例输入，包含xyz和RGB值。

* 输出：
    * 非封闭三维网格模型
    * 渲染显示

## dataset & reference
[ASL](https://projects.asl.ethz.ch/datasets/doku.php?id=home)

[3D Scans](http://kos.informatik.uni-osnabrueck.de/3Dscans/)

[tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)

# 其它
osg支持的具体格式不多，如读取ply时不支持同一顶点不同纹理坐标的模型、不支持纹理坐标在面元素中指定的模型等等。

assimp并不属于osg。只因为渲染显示一般需要加载模型，而assimp是个还不错的选择。

# 附录
opengl和glsl版本对应
```c++
//----------------------------------------
// OpenGL    GLSL      GLSL
// version   version   string
//----------------------------------------
//  2.0       110       "#version 110"
//  2.1       120       "#version 120"
//  3.0       130       "#version 130"
//  3.1       140       "#version 140"
//  3.2       150       "#version 150"
//  3.3       330       "#version 330 core"
//  4.0       400       "#version 400 core"
//  4.1       410       "#version 410 core"
//  4.2       420       "#version 410 core"
//  4.3       430       "#version 430 core"
//  ES 2.0    100       "#version 100"      = WebGL 1.0
//  ES 3.0    300       "#version 300 es"   = WebGL 2.0
//----------------------------------------
```

# 参考资料
[Shader Cookbook](https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition)