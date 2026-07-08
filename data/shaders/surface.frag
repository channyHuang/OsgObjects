#version 330
uniform sampler2DArray textureArray;
uniform float uvScale = 0.1;
uniform float cellSize = 1.0;

in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vWeights[4];

out vec4 fragColor;

uniform int Material_MAX = 17;

void main()
{
    vec3 N = normalize(vNormal);
    // 混合权重：法线绝对分量，并归一化
    vec3 blend = abs(N);
    blend = blend / (blend.x + blend.y + blend.z);

    // 世界坐标缩放作为纹理坐标基础
    vec3 pos = vWorldPos * uvScale;

    // 三向采样（每个投影单独采样，并注意处理法线方向避免纹理倒置）
    // 平面投影
    vec2 uvX = pos.yz * sign(N.x);   // 可根据法线方向翻转，保持纹理一致朝向
    vec2 uvY = pos.xz * sign(N.y);
    vec2 uvZ = pos.xy * sign(N.z);

    vec3 colX = vec3(0.0);
    vec3 colY = vec3(0.0);
    vec3 colZ = vec3(0.0);
    for (int i = 1; i < Material_MAX; ++i) {
        colX += texture(textureArray, vec3(uvX, float(i))).rgb * vWeights[(i - 1) >> 2][(i - 1) & 3];
        colY += texture(textureArray, vec3(uvY, float(i))).rgb * vWeights[(i - 1) >> 2][(i - 1) & 3];
        colZ += texture(textureArray, vec3(uvZ, float(i))).rgb * vWeights[(i - 1) >> 2][(i - 1) & 3];
    }

    // 加权混合
    vec3 finalColor = colX * blend.x + colY * blend.y + colZ * blend.z;

    fragColor = vec4(finalColor, 1.0);

    fragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0);
}
