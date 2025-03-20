in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

uniform vec2 u_resolution;
uniform vec3 u_color0;
uniform vec3 u_color1;
uniform float u_blend;

void main()
{
    vec2 st = gl_FragCoord.xy / u_resolution.xy;

    sf_fragColor = mix(sf_v_color, vec4(mix(vec4(u_color1, 1.0), vec4(u_color0, 1.0), st.y)), u_blend);
}
