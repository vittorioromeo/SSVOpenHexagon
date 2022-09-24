#version 130
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
	vec4 color = gl_Color.rgb == color0.rgb / 255.f ? vec4(vec3(1,1,1) - fill_color.rgb, 1.f) : fill_color;
	gl_FragColor = color * texture(font, gl_TexCoord[0].xy);
}
