in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

uniform vec2 u_resolution;
uniform vec3 color1;
uniform vec3 color2;

void main() {

  vec2 st = gl_FragCoord.xy/u_resolution.xy;

  float mixValue = distance(st,vec2(0,1));
  vec3 color = mix(color1,color2,mixValue);

  sf_fragColor = vec4(color,mixValue);
}
