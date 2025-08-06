#version 330 core
in vec3 fragColor;
out vec4 FragColor;
uniform float pointAlpha;

void main()
{
    FragColor = vec4(fragColor, pointAlpha);
}