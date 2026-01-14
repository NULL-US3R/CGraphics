#version 450
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 texCrd;

layout(location = 0) uniform ivec2 resolution;
layout(location = 3) uniform mat3 rotation;
layout(location = 4) uniform vec3 globalPosition;
layout(location = 5) uniform vec3 camPos;
layout(location = 6) uniform mat3 camRot;

out vec3 oPos;
out vec2 oTexPos;

//x*sin(b)+y*cos(b)
//x*cos(b)-y*sin(b)

// vec2 rot(vec2 uv, float a) {
//     return vec2(uv.x * cos(a) - uv.y * sin(a), uv.x * sin(a) + uv.y * cos(a));
// }

void main() {
    float ar = float(resolution.y) / float(resolution.x);
    vec3 o = vertPos;
    // o.yz = rot(o.yz, rotation.x);
    // o.xz = rot(o.xz, rotation.y);
    // o.xy = rot(o.xy, rotation.z);

    o = rotation * o;

    o += globalPosition - camPos;

    // o.xz = rot(o.xz, -camRot.y);
    // o.yz = rot(o.yz, -camRot.x);

    // o.xy = rot(o.xy, -camRot.z);
    o = o * camRot;
    gl_Position = vec4(o.x, o.y / ar, -tanh(o.z), o.z);
    oPos = o;
    oTexPos = texCrd;
}
