// Menu UI post-process: replaces every magenta-saturated pixel in the
// rendered UI texture with an animated noise gradient. Magenta is the
// accent color used for selection pills and text outlines, so this shader
// turns those into the eye-candy pass without touching white text or row
// backgrounds. Adapted from the steamart shader.

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

uniform sampler2D sf_u_texture;
uniform vec2  u_resolution;
uniform float u_time;

vec3 mod289v3(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289v2(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute (vec3 x) { return mod289v3(((x * 34.0) + 1.0) * x); }

float snoise(vec2 v)
{
    const vec4 C = vec4( 0.211324865405187,
                         0.366025403784439,
                        -0.577350269189626,
                         0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289v2(i);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
                   + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    vec3 x  = 2.0 * fract(p * C.www) - 1.0;
    vec3 h  = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

void main()
{
    vec4 src = texture(sf_u_texture, sf_v_texCoord) * sf_v_color;

    // Steamart-style gradient sampled at this pixel's screen position.
    vec2 st = gl_FragCoord.xy / u_resolution.xy;
    st = st * 2.0 - 1.0;
    st.x *= u_resolution.x / u_resolution.y;

    vec2  pos = st * 0.5;
    float DF  = 0.0;

    vec2 vel = vec2(u_time * 0.1);
    DF += snoise(pos + vel) * 0.25 + 0.25;

    float a = snoise(pos * vec2(cos(u_time * 0.15), sin(u_time * 0.1)) * 0.1) * 3.1415;
    vel = vec2(cos(a), sin(a));
    DF += snoise(pos + vel) * 0.25 + 0.25;

    float n = smoothstep(1.0, 0.0, fract(DF)) * 0.8 + 0.2;

    vec3 c1 = vec3(1.0,   0.0     , 113.0 / 255.0);
    vec3 c2 = vec3(1.0,  45.0/255.0,  30.0 / 255.0);
    vec3 c3 = vec3(1.0, 198.0/255.0,  47.0 / 255.0);
    vec3 gradient = mix(mix(c1, c2, n / 0.5),
                        mix(c2, c3, (n - 0.5) / 0.5),
                        step(0.5, n));

    // Detect magenta-saturated pixels: red & blue high, green low. The
    // metric is "min(R,B) − G" so a white-fill blend (1, 0.5, 1) and a
    // pure-magenta pixel (1, 0, 1) score very differently -- only the
    // latter qualifies as "magenta enough". The smoothstep lower bound is
    // intentionally low so that small-font outlines, whose pixels are
    // essentially all antialiased fringe (e.g. magenta at 30% alpha
    // composited onto an opaque black row → (0.3, 0, 0.3)) still get
    // remapped. Anything truly white / gray / black has min(R,B) ≈ G
    // and falls below the threshold.
    float magCh = clamp(min(src.r, src.b) - src.g, 0.0, 1.0);
    float isMag = smoothstep(0.05, 0.4, magCh);

    vec3 outRgb = mix(src.rgb, gradient, isMag);
    sf_fragColor = vec4(outRgb, src.a);
}
