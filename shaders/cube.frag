#version 330 core

in vec3 v_Normal;
in vec2 v_UV;

out vec4 FragColor;

void main()
{
    FragColor = vec4(normalize(v_Normal) * 0.5 + 0.5, 1.0);
}
