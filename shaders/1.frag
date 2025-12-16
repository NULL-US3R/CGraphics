#version 460

precision highp float;

in vec3 oPos;
in vec2 oTexPos;

out vec4 fragCol;

layout(location = 0) uniform ivec2 resolution;
layout(location = 1) uniform float time;

uniform sampler2D tex1;

void main() {
    vec2 uv = (gl_FragCoord.xy * 2 - resolution.xy) / min(resolution.x, resolution.y);

    fragCol = texture(tex1, oTexPos).yzxw;
}
