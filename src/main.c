#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const float HALF_WIDTH = SCREEN_WIDTH / 2.0f;
const float HALF_HEIGHT = SCREEN_HEIGHT / 2.0f;

typedef struct vec2 {
    float x;
    float y;
} vec2;

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
    float cos_theta = cos(angle_rad);
    float sin_theta = sin(angle_rad);
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

float lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow("Murder Bus", SCREEN_WIDTH,
                                          SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Surface *orig = SDL_LoadBMP("res/road.bmp");
    if (!orig) {
        fprintf(stderr, "Failed to load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Surface *surface = SDL_ConvertSurface(orig, SDL_PIXELFORMAT_ARGB8888);

    SDL_Surface *bus = SDL_LoadBMP("res/bus.bmp");
    SDL_Texture *bus_texture = SDL_CreateTextureFromSurface(renderer, bus);

    SDL_Surface *wheel = SDL_LoadBMP("res/wheel.bmp");
    SDL_Texture *wheel_texture = SDL_CreateTextureFromSurface(renderer, wheel);

    SDL_SetTextureScaleMode(bus_texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(wheel_texture, SDL_SCALEMODE_NEAREST);

    SDL_Texture *road = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!road) {
        fprintf(stderr, "Failed to create framebuffer: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetTextureScaleMode(road, SDL_SCALEMODE_NEAREST);

    SDL_Texture *render_target = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
        SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetTextureScaleMode(render_target, SDL_SCALEMODE_NEAREST);

    vec2 pos = {0.5f, 0.0f};
    vec2 vel = {0.0f, 0.0f};
    vec2 dir = {0.0f, 1.0f};
    vec2 plane = vec2_scale(vec2_perp(dir), 0.66);

    const float friction = 0.8f;
    float fric_fac = 1.0f;

    float max_h = 0.14f;
    float height = max_h;
    float wheel_angle = 0.0f;

    float current_time = SDL_GetTicks() / 1000.0f;
    float last_time = current_time;

    uint8_t fog_r = 100;
    uint8_t fog_g = 100;
    uint8_t fog_b = 100;

    float wheel_angle_min = 40.0f;
    float wheel_angle_max = -40.0f;

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        current_time = SDL_GetTicks() / 1000.0f;
        float delta_time = current_time - last_time;
        last_time = current_time;

        const bool *keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_W]) {
            vel.y += 2.0 * delta_time;
        }

        if (vel.y > 0.5f) {
            vel.x += 0.04 * delta_time;
        }

        if (keys[SDL_SCANCODE_A]) {
            vel.x += 0.5 * delta_time * vel.y;
            fric_fac = 1.5f;
            wheel_angle = lerp(wheel_angle, wheel_angle_max, 5.0f * delta_time);
        } else if (keys[SDL_SCANCODE_D]) {
            vel.x -= 0.5 * delta_time * vel.y;
            wheel_angle = lerp(wheel_angle, wheel_angle_min, 5.0f * delta_time);
            fric_fac = 1.5f;
        } else {
            wheel_angle = lerp(wheel_angle, 0.0f, 5.0f * delta_time);
        }

        vec2 friction_force = vec2_scale(vel, -friction * fric_fac);
        vel = vec2_add(vel, vec2_scale(friction_force, delta_time));

        pos = vec2_add(pos, vec2_scale(vel, delta_time));

        SDL_SetRenderDrawColor(renderer, fog_r, fog_g, fog_b, 255);
        SDL_RenderClear(renderer);

        void *pixels;
        int pitch;
        SDL_LockTexture(road, NULL, &pixels, &pitch);

        uint32_t *pixels32 = (uint32_t *)pixels;

        for (int y = HALF_HEIGHT; y < SCREEN_HEIGHT; y++) {
            vec2 ray_dir_0 = vec2_sub(dir, plane);
            vec2 ray_dir_1 = vec2_add(dir, plane);

            int p = y - HALF_HEIGHT;

            float pos_z = height * SCREEN_HEIGHT;

            float row_distance = pos_z / p;

            float floor_step_x =
                row_distance * (ray_dir_1.x - ray_dir_0.x) / SCREEN_WIDTH;

            float floor_step_y =
                row_distance * (ray_dir_1.y - ray_dir_0.y) / SCREEN_WIDTH;

            float floor_x = pos.x + row_distance * ray_dir_0.x;
            float floor_y = pos.y + row_distance * ray_dir_0.y;

            for (int x = 0; x < SCREEN_WIDTH; x++) {
                int cell_x = (int)floor_x;
                int cell_y = (int)floor_y;

                int tx = (int)(surface->w * floor_x);
                int ty =
                    (int)(surface->h * (floor_y - cell_y)) & (surface->h - 1);

                uint32_t *texels = surface->pixels;
                uint32_t texel;

                if (ty >= 0 && ty < surface->h && tx >= 0 && tx < surface->w) {
                    texel = texels[ty * (surface->pitch / 4) + tx];
                } else {
                    texel = 0xFFd9a066;
                }

                uint8_t texel_r = (texel >> 16) & 0xFF;
                uint8_t texel_g = (texel >> 8) & 0xFF;
                uint8_t texel_b = texel & 0xFF;

                float fog_density = 0.3f;
                float fog_factor = 1.0f - exp(-fog_density * row_distance);
                fog_factor = (fog_factor > 1.0f) ? 1.0f : fog_factor;

                uint8_t final_r = (uint8_t)(texel_r * (1.0f - fog_factor) +
                                            fog_r * fog_factor);
                uint8_t final_g = (uint8_t)(texel_g * (1.0f - fog_factor) +
                                            fog_g * fog_factor);
                uint8_t final_b = (uint8_t)(texel_b * (1.0f - fog_factor) +
                                            fog_b * fog_factor);

                uint32_t final_color =
                    (0xFF << 24) | (final_r << 16) | (final_g << 8) | final_b;

                floor_x += floor_step_x;
                floor_y += floor_step_y;

                pixels32[y * (pitch / 4) + x] = final_color;
            }
        }

        SDL_UnlockTexture(road);

        SDL_SetRenderTarget(renderer, render_target);

        SDL_RenderTexture(renderer, road, NULL, NULL);
        SDL_RenderTexture(renderer, bus_texture, NULL, NULL);

        SDL_FRect dest = {SCREEN_WIDTH / 2.0f - wheel->w / 2.0f,
                          SCREEN_HEIGHT - wheel->h / 2.0f, .w = wheel->w,
                          .h = wheel->h};

        SDL_RenderTextureRotated(
            renderer, wheel_texture, NULL, &dest, wheel_angle,
            &(SDL_FPoint){wheel->w / 2.0f, wheel->h / 2.0f}, SDL_FLIP_NONE);

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderTexture(renderer, render_target, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(road);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
