#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in vec3 a_Offset; // per-instance (divisor = 1)

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Normal;
out vec2 v_UV;

void main()
{
    vec4 worldPosition = u_Model * vec4(a_Position, 1.0) + vec4(a_Offset, 0.0);
    gl_Position = u_Projection * u_View * worldPosition;

    v_Normal = mat3(u_Model) * a_Normal; // rotate the normal together with the cube
    v_UV = a_UV;
}
