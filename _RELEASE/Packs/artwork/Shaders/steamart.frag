uniform vec2 u_resolution;
uniform float u_time;
uniform float u_rotation;

// These functions were written by patriciogv (https://patriciogonzalezvivo.com).
// He's an absolute beast for writing this. I can't comprehend this.

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
        + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}


void main()
{
    vec2 st = gl_FragCoord.xy / u_resolution.xy;
    st = st * 2.0 - 1.0;
    st.x *= u_resolution.x / u_resolution.y;

    // Rotate the coordinates with the level
    float s = sin(u_rotation);
    float c = cos(u_rotation);
    mat2 rot = mat2(c, s, -s, c);
    st = st * rot;
    
    // Set up the noise and animate it
    vec2 noise_vec = st;
    vec2 pos = vec2(noise_vec / 2.0);

    float DF = 0.0;

    // Add a random position
    float a = 0.0;
    vec2 vel = vec2(u_time*.1);
    DF += snoise(pos+vel)*.25+.25;

    // Add a random position
    a = snoise(pos*vec2(cos(u_time*0.15),sin(u_time*0.1))*0.1)*3.1415;
    vel = vec2(cos(a),sin(a));
    DF += snoise(pos+vel)*.25+.25;

    // Generate the noise
    vec3 n = vec3( smoothstep(1.0, 0.0, fract(DF)) * .8 + .2);

    vec3 color1 = vec3(1.0, 0.0, 113.0/255.0);
    vec3 color2 = vec3(1.0, 45.0 / 255.0, 30.0 / 255.0);
    vec3 color3 = vec3(1.0, 198.0 / 255.0, 47.0 / 255.0);

    // Convert the black and white noise into the gradient colors we want.
    vec3 color_map = mix(mix(color1, color2, n.x/0.5), mix(color2, color3, (n.x - 0.5)/(1.0 - 0.5)), step(0.5, n.x));

    // Apply our shader to the final colors
    // vec4(mix(vec4(u_color1, 1.0), vec4(u_color0, 1.0), st.y))
    // Considering that our style is B&W, blend the shader in a Multiplying style
    gl_FragColor = gl_Color * vec4(color_map, 1.0);
}
