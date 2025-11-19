#version 460

precision highp float;

out vec4 fragCol;

layout(location = 0) uniform ivec2 resolution;
layout(location = 1) uniform float time;

void main() {
    vec2 uv = (gl_FragCoord.xy * 2 - resolution.xy) / min(resolution.x, resolution.y);

    vec4 col = vec4(1., 0., 0., 0.);
    fragCol = col;
}
