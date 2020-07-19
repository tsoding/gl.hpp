#version 130

in int tile;
out vec4 color;

#define TILE_SIZE 0.1

vec4 color_table[4] = vec4[4](
    vec4(1, 0, 0, 1),
    vec4(0, 1, 0, 1),
    vec4(0, 0, 1, 1),
    vec4(1, 1, 0, 1));

void main(void)
{
    int instanceID = gl_VertexID / 4;
    int vertexID = gl_VertexID % 4;

    // TODO: make visible size of the tiles independant from the size of the window
    int gray = vertexID ^ (vertexID >> 1);
    gl_Position = vec4(
        // 2 * (gray / 2) - 1,
        0.0 + (gray / 2) * TILE_SIZE + instanceID * TILE_SIZE,
        // 2 * (gray % 2) - 1,
        0.0  + (gray % 2) * TILE_SIZE,
        0.0,
        1.0);

    color = color_table[tile - 1];
}
