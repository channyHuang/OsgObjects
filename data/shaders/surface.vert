#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aWeights[4];
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;

out vec3 vWorldPos;
out vec3 vNormal;
out vec4 vWeights[4];

void main() {
    vWorldPos = aPos;
    vNormal = aNormal;

    for (int i = 0; i < 4; ++i) {
        vWeights[i] = aWeights[i];
    }

    gl_Position = osg_ModelViewProjectionMatrix * vec4(aPos, 1.0);
}