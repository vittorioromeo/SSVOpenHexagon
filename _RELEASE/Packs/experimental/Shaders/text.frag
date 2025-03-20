in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

uniform vec4 color0;
uniform sampler2D font;
uniform float time;

vec4 getColorFromHue(const float hue) {
    int i = int(hue * 6.f);

    float f = (hue * 6.f) - i;
    float q = 1.f - f;
    float t = f;

    switch(i)
    {
        case 0: return vec4(1.f, t, 0.f, 1.f);
        case 1: return vec4(q, 1.f, 0.f, 1.f);
        case 2: return vec4(0.f, 1.f, t, 1.f);
        case 3: return vec4(0.f, q, 1.f, 1.f);
        case 4: return vec4(t, 0.f, 1.f, 1.f);
    }

    return vec4(1.f, 0.f, q, 1.f);
}

void main() {
	vec4 fill_color = getColorFromHue(mod(time * 360.f + gl_FragCoord.x + gl_FragCoord.y, 360.f) / 360.f);
	vec4 color = sf_v_color.rgb == color0.rgb / 255.f ? vec4(vec3(1,1,1) - fill_color.rgb, 1.f) : fill_color;
	sf_fragColor = color * texture(font, sf_v_texCoord.xy);
}
