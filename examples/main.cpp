#include <cassert>
#include <cstdio>
#include <cstdlib>
#define GL_GLEXT_PROTOTYPES 2
#include <GLFW/glfw3.h>
#include <unistd.h>
#define GL_HPP_ASSERT_GL_ERRORS
#include "gl.hpp"

void platformSleep(float secs)
{
    usleep((useconds_t) (secs * 1e+6f));
}

const size_t INFO_LOG_CAPACITY = 512;

template <GLsizei Max_Length>
void print_info_log(FILE *stream, gl::Info_Log<Max_Length> *log)
{
    fwrite(log->value, 1, log->length, stderr);
    fprintf(stderr, "\n");
}

// TODO: read_whole_file does not check all the errors
const char *read_whole_file(const char *file_path)
{
    FILE *f = fopen(file_path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long m = ftell(f);
    char *result = (char *) malloc(m + 1);
    fseek(f, 0, SEEK_SET);
    fread(result, 1, m, f);
    result[m] = '\0';

    fclose(f);

    return result;
}

gl::Program link_program(gl::Shader vert, gl::Shader frag)
{
    gl::Program program = gl::createProgram();
    gl::attachShader(program, vert);
    gl::attachShader(program, frag);
    gl::linkProgram(program);
    if (!gl::linkStatus(program)) {
        auto log = gl::getProgramInfoLog<INFO_LOG_CAPACITY>(program);
        fprintf(stderr, "Program link error: ");
        print_info_log<INFO_LOG_CAPACITY>(stderr, &log);
        gl::deleteObject(program);
        abort();
    }
    return program;
}

gl::Shader read_and_compile_shader(gl::Shader_Type shaderType, const char *file_path)
{
    const char *shader_source = read_whole_file(file_path);
    assert(shader_source);

    auto shader = gl::createShader(shaderType);
    gl::shaderSource(shader, 1, &shader_source, NULL);
    gl::compileShader(shader);
    if (!gl::compileStatus(shader)) {
        auto infoLog = gl::getShaderInfoLog<INFO_LOG_CAPACITY>(shader);
        fprintf(stderr, "Shader compilation failed `%s`: ", file_path);
        print_info_log<INFO_LOG_CAPACITY>(stderr, &infoLog);
        gl::deleteObject(shader);
        abort();
    }

    free((void*) shader_source);
    return shader;
}

void oopsie_doopsie(int code, const char* description)
{
    fprintf(stderr, "GLFW did an oopsie-doopsie: %s\n", description);
    abort();
}

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 640;

// TODO: VAO to remove redundancy in the tiles array (I guess I'm thinking about VBO)
GLint tiles[] = {
    1, 1, 1, 1,
    2, 2, 2, 2,
    3, 3, 3, 3,
    4, 4, 4, 4,
};

void funcname(GLenum source, GLenum type, GLuint id,
              GLenum severity, GLsizei length,
              const GLchar* message,
              const void* userParam)
{
    fprintf(stderr, "%s\n", message);
}

int main(int argc, char *argv[])
{
    glfwSetErrorCallback(oopsie_doopsie);

    glfwInit();

    GLFWwindow *window =
        glfwCreateWindow(
            WINDOW_WIDTH, WINDOW_HEIGHT,
            "C++ in 2020",
            NULL, NULL);

    glfwMakeContextCurrent(window);

    printf("OpenGL Version: %s\n", gl::getString(gl::VERSION));

    glDebugMessageCallback(funcname, NULL);

    auto vert = read_and_compile_shader(gl::Shader_Type::Vertex, "shader.vert");
    auto frag = read_and_compile_shader(gl::Shader_Type::Fragment, "shader.frag");
    auto program = link_program(vert, frag);
    gl::useProgram(program);

    auto vao = gl::genVertexArray();
    gl::bindVertexArray(vao);

    auto tile_buffer = gl::genBuffer();
    gl::bindBuffer(gl::Buffer_Target::ARRAY, tile_buffer);
    gl::bufferData(gl::Buffer_Target::ARRAY, sizeof(tiles), tiles, gl::STATIC_DRAW);

    gl::Attribute_ID tileAttrib = {0};
    gl::bindAttribLocation(program, tileAttrib, "tile");
    gl::enableVertexAttribArray(tileAttrib);
    gl::vertexAttribIPointer(tileAttrib,
                             1,
                             gl::INT,
                             0,
                             nullptr);

    auto u_resolution = gl::getUniformLocation(program, "u_resolution");
    auto u_time = gl::getUniformLocation(program, "u_time");

    const float delta_time = 1.0f / 60.0f;
    float time = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        gl::clearColor({0.0f, 0.0f, 0.0f, 1.0f});
        gl::clear(gl::Buffer_Bit::COLOR | gl::Buffer_Bit::DEPTH);

        if (u_resolution.has_value) {
            gl::uniform(u_resolution.unwrap, gl::Vec2f {(GLfloat) width, (GLfloat) height});
        }

        if (u_time.has_value) {
            gl::uniform(u_time.unwrap, time);
        }

        gl::bindVertexArray(vao);
        gl::drawArrays(gl::Draw_Mode::TRIANGLE_STRIP, 0, sizeof(tiles) / sizeof(tiles[0]));

        glfwSwapBuffers(window);
        glfwPollEvents();
        platformSleep(delta_time);
        time += delta_time;
    }

    glfwTerminate();

    return 0;
}
