#version 140

uniform vec4 screen_transform;

in  vec2 vpos;
in  vec2 vuvs;
in  vec4 vcolor; 
out vec4 interp_color; 
out vec2 interp_uvs;
void main(void)
{
  interp_color = vcolor; 
  interp_uvs = vuvs;
  gl_Position = vec4(vpos.x * screen_transform.x - 1.0, -vpos.y * screen_transform.y + 1, 0.0, 1.0); 
}
