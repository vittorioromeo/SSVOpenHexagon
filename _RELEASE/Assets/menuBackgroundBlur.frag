// Menu-background gaussian blur (separable, plain discrete sampling).
//
// Run twice per frame: once with `u_direction = (1, 0)` to blur
// horizontally into an intermediate texture, then once with
// `u_direction = (0, 1)` to blur vertically into the final target.
// Splitting the convolution along axes drops the cost from N*N to 2*N
// samples -- the only practical way to scale the radius without nuking
// the fragment-shader budget.
//
// We use plain discrete sampling at multi-pixel offsets rather than the
// hardware-linear-pair optimization. The pair trick only works when
// consecutive taps are exactly one texel apart; the moment you stretch
// the per-tap step (which we have to do to widen the kernel) the
// pre-baked pair offsets stop matching their intended texel pairs and
// the kernel collapses into ghosted copies of the source. Discrete
// sampling stays correct at any kernel scale at the cost of a few more
// texture fetches (13 per direction = ~26 fps cost for the whole pass).
//
// Weights are a normalized Gaussian with σ = 2.5 in tap-units:
//   w_i = exp(-i² / (2σ²))   then renormalized so Σ w_i = 1.
// `u_blur` (0..1) scales the per-tap pixel step -- at 0 we early-out to
// a passthrough copy.

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

uniform sampler2D sf_u_texture;
uniform vec2  u_resolution;
uniform vec2  u_direction; // (1,0) horizontal, (0,1) vertical
uniform float u_blur;

void main()
{
    vec4 src = texture(sf_u_texture, sf_v_texCoord);

    // Per-tap pixel step at full blur. Increasing this widens the
    // visible radius without changing the kernel shape.
    float stepPx = u_blur * 4.0;

    // Cheap early-out: at the menu root the step is essentially zero
    // and the kernel collapses to the center sample.
    if (stepPx < 0.05)
    {
        sf_fragColor = src * sf_v_color;
        return;
    }

    vec2 stepUV = u_direction * stepPx / u_resolution;

    // 13-tap kernel: 1 center + 6 each side.
    const float wCenter = 0.16101;
    float w[6];
    w[0] = 0.14863;
    w[1] = 0.11691;
    w[2] = 0.07838;
    w[3] = 0.04476;
    w[4] = 0.02178;
    w[5] = 0.00903;

    vec4 col = src * wCenter;

    for (int i = 0; i < 6; ++i)
    {
        float fi = float(i + 1);
        col += texture(sf_u_texture, sf_v_texCoord + stepUV * fi) * w[i];
        col += texture(sf_u_texture, sf_v_texCoord - stepUV * fi) * w[i];
    }

    sf_fragColor = col * sf_v_color;
}
