uniform vec2 u_resolution;
uniform vec3 u_color0;
uniform vec3 u_color1;
uniform float u_blend;

void main()
{
    vec2 st = gl_FragCoord.xy / u_resolution.xy;

    gl_FragColor = mix(gl_Color, vec4(mix(vec4(u_color1, 1.0), vec4(u_color0, 1.0), st.y)), u_blend);
}
