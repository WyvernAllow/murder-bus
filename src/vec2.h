#ifndef VEC2_H
#define VEC2_H

typedef struct vec2 {
    float x;
    float y;
} vec2;

vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_scale(vec2 v, float s);
vec2 vec2_rotate(vec2 v, float angle_rad);
vec2 vec2_perp(vec2 v);

#endif /* VEC2_H */
