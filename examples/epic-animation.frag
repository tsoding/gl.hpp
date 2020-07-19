uniform vec2 u_resolution;
uniform float u_time;

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233))) *
        43758.5453123);
}

#define TILE_SIZE_INVERSED 50.0
#define VELOCITY_FACTOR 5.0
#define PADDING 0.2

#define VERTICAL

void main() {
    vec2 st = (gl_FragCoord.xy / u_resolution);
    st *= TILE_SIZE_INVERSED;

#ifdef VERTICAL
    vec2 offset = vec2(
        0.0,
        u_time * random(vec2(floor(st.x), 0)) * VELOCITY_FACTOR);
    vec3 color = vec3(0.0, step(random(floor(st + offset)), 0.5) * 0.6, 0.0);
    color *= step(PADDING, fract(st.x));
#else
    vec2 offset = vec2(
        u_time * random(vec2(0, floor(st.y))) * VELOCITY_FACTOR,
        0.0);
    vec3 color = vec3(0.0, step(0.5, random(floor(st + offset))) * 0.6, 0.0);
    color *= step(PADDING, fract(st.y));
#endif // VERTICAL

    gl_FragColor = vec4(color, 1.0);
}
