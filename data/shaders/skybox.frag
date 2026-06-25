#version 330 core
in vec3 vTexCoord;
uniform samplerCube skybox; // 必须与纹理单元对应
out vec4 FragColor;

void main()
{
    FragColor = texture(skybox, vTexCoord);
}
