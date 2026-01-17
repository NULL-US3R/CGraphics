#version 450
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 texCrd;
layout(location = 2) in ivec4 bone_id;
layout(location = 3) in vec4 bone_weight;

layout(std430, binding = 1) buffer bones {
    layout(row_major) mat4 bone_mat[];
};

layout(location = 0) uniform ivec2 resolution;
layout(location = 3) uniform mat3 rotation;
layout(location = 4) uniform vec3 globalPosition;
layout(location = 5) uniform vec3 camPos;
layout(location = 6) uniform mat3 camRot;
layout(location = 7) uniform int hasBones;

out vec3 oPos;
out vec2 oTexPos;

void main() {
    float ar = float(resolution.y) / float(resolution.x);
    vec3 o = vertPos;

    if (hasBones != 0) {
        mat4 skin = mat4(0);

        vec4 tw = bone_weight;
        //tw = normalize(tw);
        if (bone_id.x >= 0) {
            skin += tw.x * bone_mat[bone_id.x];
        }
        if (bone_id.y >= 0) {
            skin += tw.y * bone_mat[bone_id.y];
        }
        if (bone_id.z >= 0) {
            skin += tw.z * bone_mat[bone_id.z];
        }
        if (bone_id.w >= 0) {
            skin += tw.w * bone_mat[bone_id.w];
        }
        vec4 to = vec4(o, 1);
        to = skin * to;
        o = to.xyz;
    }

    o = rotation * o;

    o += globalPosition - camPos;

    o = o * camRot;
    gl_Position = vec4(o.x, o.y / ar, -tanh(o.z), o.z);
    oPos = o;
    oTexPos = texCrd;
}
