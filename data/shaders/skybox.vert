#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix; // 用于去除平移分量

out vec3 vTexCoord; // 3D纹理坐标

void main()
{
    vec4 pos = osg_ModelViewMatrix * vec4(aPos, 1.0);
    // 天空盒一般需要去掉模型矩阵的平移，仅保留旋转，这里直接用原始位置也可以
    vTexCoord = aPos; // 模型空间坐标即为方向向量
    gl_Position = osg_ModelViewProjectionMatrix * vec4(aPos, 1.0);
}
