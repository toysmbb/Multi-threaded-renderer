#version 330 core

layout(location = 0) in vec3 a_Position; // unit quad corner
layout(location = 1) in vec2 a_Offset;   // per-instance
layout(location = 2) in vec4 a_Color;    // per-instance
layout(location = 3) in float a_Size;    // per-instance

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 v_Color;

void main()
{
    vec3 worldPosition = a_Position * a_Size + vec3(a_Offset, 0.0);
    gl_Position = u_Projection * u_View * vec4(worldPosition, 1.0);
    v_Color = a_Color;
}
