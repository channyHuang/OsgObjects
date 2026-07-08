#version 330
uniform sampler2DArray textureArray;
uniform float uvScale = 0.1;
uniform float cellSize = 1.0;

in vec3 vWorldPos;
in vec3 vNormal;
// flat in uint TexIndex;
flat in float TexIndex;

out vec4 fragColor;

// simple random function
float hash(vec3 p)
{
    p  = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

void main()
{
    vec3 N = normalize(vNormal);
    // 混合权重：法线绝对分量，并归一化
    vec3 blend = abs(N);
    blend = blend / (blend.x + blend.y + blend.z);

    // 世界坐标缩放作为纹理坐标基础
    vec3 pos = vWorldPos * uvScale;

    // 为打破重复，对每个“大块”计算随机偏移
    vec3 cell = floor(vWorldPos / cellSize);
    float rnd = hash(cell);
    // 可以用随机偏移扰动 UV，也可用它选择纹理层（如 vMatIndex + 随机偏移）
    float texLayer = float(TexIndex);

    // 三向采样（每个投影单独采样，并注意处理法线方向避免纹理倒置）
    // X 平面投影
    vec2 uvX = pos.yz;
    uvX.x *= sign(N.x);   // 可根据法线方向翻转，保持纹理一致朝向
    vec3 colX = texture(textureArray, vec3(uvX, texLayer)).rgb;

    // Y 平面投影
    vec2 uvY = pos.xz;
    uvY.x *= sign(N.y);
    vec3 colY = texture(textureArray, vec3(uvY, texLayer)).rgb;

    // Z 平面投影
    vec2 uvZ = pos.xy;
    uvZ.x *= sign(N.z);
    vec3 colZ = texture(textureArray, vec3(uvZ, texLayer)).rgb;

    // 加权混合
    vec3 finalColor = colX * blend.x + colY * blend.y + colZ * blend.z;

    fragColor = vec4(finalColor, 1.0);
}