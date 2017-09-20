#version 140

precision highp float; // needed only for version 1.30

uniform sampler2D tex0;

in vec4 interp_color;
in vec2 interp_uvs;
out vec4 result;

void main(void)
{
	result = texture(tex0, interp_uvs) * interp_color;
	//result = vec4(interp_uvs, 0.0, 1.0);
}
