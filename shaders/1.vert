#version 460
layout(location = 0) in vec3 vertPos;

layout(location = 3) uniform ivec3 rotation;
layout(location = 4) uniform ivec3 globalPosition;

void main() {
    gl_Position = vec4(vertPos, 1.);
}
