#version 140

uniform sampler2D tex0;

#pragma glov_param(param0 ambient)
#pragma glov_param(param1 light_diffuse)
#pragma glov_param(param2 lightdir_vs)
#pragma glov_param(param3 texmod0)
#pragma glov_param(param4 texmod1)

uniform vec4 ambient;
uniform vec4 light_diffuse;
uniform vec4 lightdir_vs;
uniform vec4 texmod0;
uniform vec4 texmod1;

in vec4 interp_color;
in vec2 interp_uvs;
in vec3 interp_normal_vs;
out vec4 result;

void main(void)
{
	vec4 albedo = texture(tex0, interp_uvs);
	vec3 normal_vs = normalize(interp_normal_vs);
	float diffuse = max(0, dot(normal_vs, -lightdir_vs.rgb));
	diffuse = diffuse * 0.75 + 0.25;

	vec3 color = diffuse * light_diffuse.rgb + ambient.rgb;
	result.rgb = color * albedo.rgb * interp_color.rgb;
	result.a = interp_color.a * albedo.a;
	
	//result = vec4(diffuse, diffuse, diffuse, 1.0);
	//result = vec4(normal_vs.rgb * 0.5 + 0.5, 1.0);
	//result = vec4(lightdir_vs.rgb * 0.5 + 0.5, 1.0);
	//result = vec4(gl_FragCoord.rgb, 1);
	//result = vec4(interp_normal_os * 0.5 + 0.5, 1);
}
