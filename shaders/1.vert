#version 460
layout(location = 0) in vec3 vertPos;

layout(location = 3) uniform vec3 rotation;
layout(location = 4) uniform vec3 globalPosition;

//x*sin(b)+y*cos(b)
//x*cos(b)-y*sin(b)

vec2 rot(vec2 uv, float a) {
    return vec2(uv.x * cos(a) - uv.y * sin(a), uv.x * sin(a) + uv.y * cos(a));
}

void main() {
    vec3 o = vertPos;
    o.yz = rot(o.yz, rotation.x);
    o.xz = rot(o.xz, rotation.y);
    o.xy = rot(o.xy, rotation.z);

    o += globalPosition;

    gl_Position = vec4(o.x, o.y, 1., o.z);
}
