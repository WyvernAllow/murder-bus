#include "vec2.h"

#include <math.h>

vec2 vec2_add(vec2 a, vec2 b) {
    vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

vec2 vec2_sub(vec2 a, vec2 b) {
    vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

vec2 vec2_scale(vec2 v, float s) {
    vec2 result;
    result.x = v.x * s;
    result.y = v.y * s;
    return result;
}

vec2 vec2_rotate(vec2 v, float angle_rad) {
    vec2 result;
    float cos_theta = cosf(angle_rad);
    float sin_theta = sinf(angle_rad);
    result.x = v.x * cos_theta - v.y * sin_theta;
    result.y = v.x * sin_theta + v.y * cos_theta;
    return result;
}

vec2 vec2_perp(vec2 v) {
    vec2 result;
    result.x = -v.y;
    result.y = v.x;
    return result;
}