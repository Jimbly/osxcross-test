#version 140

uniform mat3 mv_inv_trans; // state.matrix.modelview.invtrans
uniform mat4 mvp; // state.matrix.mvp

in  vec3 vpos;
in  vec2 vuvs;
in  vec4 vcolor; 
in  vec3 vnormal;
out vec4 interp_color; 
out vec2 interp_uvs;
out vec3 interp_normal_vs; // normal in camera/eye coordinates

void main(void)
{
  interp_color = vcolor; 
  interp_uvs = vuvs;
  
  interp_normal_vs = mv_inv_trans * vnormal;
  gl_Position = mvp * vec4(vpos, 1.0);
}

// END