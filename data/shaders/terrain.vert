#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
// layout(location = 2) in uint aTexIndex;
layout(location = 2) in float aTexIndex;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;

out vec3 vWorldPos;
out vec3 vNormal;
// flat out uint TexIndex; 
flat out float TexIndex;

void main() {
    vWorldPos = aPos;
    vNormal = aNormal;
    TexIndex = aTexIndex;
    gl_Position = osg_ModelViewProjectionMatrix * vec4(aPos, 1.0);
}