#version 330 core

in vec2 texCoordAttrib;
in vec3 normalAttrib;
in vec3 positionAttrib;

uniform mat4 projection;
uniform mat4 modelView;

void main()
{
	gl_Position = projection * modelView * vec4(positionAttrib, 1.0f);
}
