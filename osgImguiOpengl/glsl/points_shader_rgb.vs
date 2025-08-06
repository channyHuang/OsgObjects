#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aColor;

out vec3 fragColor;

uniform mat4 projection_mul_view;
uniform float pointSize;

void main()
{
    gl_Position = projection_mul_view * vec4(aPos, 1.0);
    int color_int = int(aColor);  // I Don't know why here need a cast
    fragColor = vec3( ( ( color_int >> 16 ) & 0xff ), 
                      ( ( color_int >> 8 ) & 0xff  ),
                      ( ( color_int >> 0 ) & 0xff ) ) / 255.0; 
    gl_PointSize = pointSize;
}