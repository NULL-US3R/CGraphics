#version 460

precision highp float;

out vec4 fragCol;

layout(location = 0) uniform ivec2 resolution;
layout(location = 1) uniform float time;

vec2 rot(vec2 x, float a) {
    return x * mat2(cos(a), sin(a), -sin(a), cos(a));
}

vec2 cmul(vec2 c1, vec2 c2) {
    return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

// vec2 cdiv(vec2 c1, vec2 c2){
//     return cmul(c1,vec2(c2.x,-c2.y))/(c2.x*c2.x+c2.y*c2.y);
// }

// vec4 qmul(vec4 q1, vec4 q2){
//     return vec4(q1.x*q2.x-q1.y*q2.y-q1.z*q2.z-q1.w*q2.w,q1.x*q2.y+q1.y*q2.x+q1.z*q2.w-q1.w*q2.z,q1.x*q2.z-q1.y*q2.w+q1.z*q2.x+q1.w*q2.y,q1.x*q2.w+q1.y*q2.z-q1.z*q2.y+q1.w*q2.x);
// }

float mdb(vec4 uv) {
    vec2 c = uv.zw;
    vec2 z = uv.xy;
    vec2 dz = vec2(0.);
    int i = 0, mi = 30;
    for (i = 0; i < mi; i++) {
        z = cmul(z, z) + c;
        if (length(z) > 6400.) break;
    }
    return log(length(z)) / pow(2.5, float(i));
}

vec3 raym(vec2 uv) {
    vec3 rd = normalize(vec3(uv, 1.));
    vec3 ro = vec3(0., 0., -3.);
    float d = 0., dd;
    for (int i = 0; i < 70; i++) {
        vec4 crd = vec4(0.1, ro + rd * d);
        crd.xz = rot(crd.xz, time);
        crd.yz = rot(crd.yz, .7 * time);
        dd = mdb(crd);
        d += dd;
    }

    vec4 crd = vec4(0.0, ro + rd * d);
    crd.xz = rot(crd.xz, time);
    crd.yz = rot(crd.yz, .7 * time);
    float eps = 0.001;
    vec3 nrm = normalize(vec3(
                mdb(crd + vec4(eps, 0., 0., 0.)) - mdb(crd - vec4(eps, 0., 0., 0.)),
                mdb(crd + vec4(0., eps, 0., 0.)) - mdb(crd - vec4(0., eps, 0., 0.)),
                mdb(crd + vec4(0., 0., eps, 0.)) - mdb(crd - vec4(0., 0., eps, 0.))
            ));

    return (dd > .2) ? vec3(0.) : (vec3(0.3 + 0.7 * clamp(dot(nrm, normalize(vec3(1.))), 0., 1.)));
}

void main() {
    vec2 uv = (gl_FragCoord.xy * 2 - resolution.xy) / min(resolution.x, resolution.y);

    vec3 col = raym(uv);
    fragCol = vec4(col, 1.);
}
