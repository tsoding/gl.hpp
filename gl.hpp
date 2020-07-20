#ifndef GL_HPP
#define GL_HPP

#if defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline
#endif

template <typename T>
struct Maybe
{
    bool has_value;
    T unwrap;
};

// TODO: gl.hpp supports only OpenGL 3.0 for now

namespace gl
{
    struct Color4
    {
        GLclampf r, g, b, a;
    };

    enum Clear_Mask
    {
        COLOR_BUFFER_BIT = GL_COLOR_BUFFER_BIT,
        DEPTH_BUFFER_BIT = GL_DEPTH_BUFFER_BIT,
        ACCUM_BUFFER_BIT = GL_ACCUM_BUFFER_BIT,
        STENCIL_BUFFER_BIT = GL_STENCIL_BUFFER_BIT
    };

    ALWAYS_INLINE Clear_Mask operator|(Clear_Mask a, Clear_Mask b)
    {
        return (Clear_Mask)((int) a | (int) b);
    }

    ALWAYS_INLINE void clear(Clear_Mask buffer)
    {
        glClear(buffer);
        assert(glGetError() == GL_NO_ERROR);
    }

    ALWAYS_INLINE void clearColor(Color4 color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
        assert(glGetError() == GL_NO_ERROR);
    }

    enum Shader_Type
    {
        VERTEX_SHADER = GL_VERTEX_SHADER,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
        GEOMETRY_SHADER = GL_GEOMETRY_SHADER
    };

    struct Shader
    {
        GLuint unwrap;
    };

    ALWAYS_INLINE Shader createShader(Shader_Type type)
    {
        auto shader = glCreateShader((GLenum)type);
        assert(glGetError() == GL_NO_ERROR);
        return Shader { shader };
    }

    ALWAYS_INLINE void shaderSource(Shader shader,
                                    GLsizei count,
                                    const GLchar ** string,
                                    const GLint * length)
    {
        glShaderSource(shader.unwrap, count, string, length);
        assert(glGetError() == GL_NO_ERROR);
    }

    template <size_t Max_Length>
    struct Info_Log
    {
        GLchar value[Max_Length];
        GLsizei length;
    };

    ALWAYS_INLINE void compileShader(Shader shader)
    {
        glCompileShader(shader.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    template <GLsizei Max_Length>
    ALWAYS_INLINE void getShaderInfoLog(Shader shader, Info_Log<Max_Length> *infoLog)
    {
        glGetShaderInfoLog(shader.unwrap, Max_Length, &infoLog->length, infoLog->value);
        assert(glGetError() == GL_NO_ERROR);
    }

    template <GLsizei Max_Length>
    ALWAYS_INLINE Info_Log<Max_Length> getShaderInfoLog(Shader shader)
    {
        Info_Log<Max_Length> infoLog = {};
        glGetShaderInfoLog(shader.unwrap, Max_Length, &infoLog.length, infoLog.value);
        assert(glGetError() == GL_NO_ERROR);
        return infoLog;
    }

    ALWAYS_INLINE bool compileStatus(Shader shader)
    {
        GLint param = 0;
        glGetShaderiv(shader.unwrap, GL_COMPILE_STATUS, &param);
        assert(glGetError() == GL_NO_ERROR);
        return (bool) param;
    }

    ALWAYS_INLINE void deleteObject(Shader shader)
    {
        glDeleteShader(shader.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    struct Program
    {
        GLuint unwrap;
    };

    ALWAYS_INLINE Program createProgram(void)
    {
        auto program = glCreateProgram();
        assert(glGetError() == GL_NO_ERROR);
        return Program { program };
    }

    ALWAYS_INLINE void deleteObject(Program program)
    {
        glDeleteProgram(program.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    ALWAYS_INLINE void attachShader(Program program, Shader shader)
    {
        glAttachShader(program.unwrap, shader.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    ALWAYS_INLINE void linkProgram(Program program)
    {
        glLinkProgram(program.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    ALWAYS_INLINE bool linkStatus(Program program)
    {
        GLint linked = 0;
        glGetProgramiv(program.unwrap, GL_LINK_STATUS, &linked);
        assert(glGetError() == GL_NO_ERROR);
        return (bool) linked;
    }

    template <GLsizei Max_Length>
    ALWAYS_INLINE void getProgramInfoLog(Program program, Info_Log<Max_Length> *infoLog)
    {
        glGetProgramInfoLog(program.unwrap, Max_Length, &infoLog->length, infoLog->value);
        assert(glGetError() == GL_NO_ERROR);
    }

    template <GLsizei Max_Length>
    ALWAYS_INLINE Info_Log<Max_Length> getProgramInfoLog(Program program)
    {
        Info_Log<Max_Length> infoLog = {};
        glGetProgramInfoLog(program.unwrap, Max_Length, &infoLog.length, infoLog.value);
        assert(glGetError() == GL_NO_ERROR);
        return infoLog;
    }

    void useProgram(Program program)
    {
        glUseProgram(program.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    enum Mode {
        POINTS                   = GL_POINTS,
        LINE_STRIP               = GL_LINE_STRIP,
        LINE_LOOP                = GL_LINE_LOOP,
        LINES                    = GL_LINES,
        LINE_STRIP_ADJACENCY     = GL_LINE_STRIP_ADJACENCY,
        LINES_ADJACENCY          = GL_LINES_ADJACENCY,
        TRIANGLE_STRIP           = GL_TRIANGLE_STRIP,
        TRIANGLE_FAN             = GL_TRIANGLE_FAN,
        TRIANGLES                = GL_TRIANGLES,
        TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
        TRIANGLES_ADJACENCY      = GL_TRIANGLES_ADJACENCY
    };

    ALWAYS_INLINE void drawArrays(Mode mode, GLint first, GLsizei count)
    {
        glDrawArrays((GLenum) mode, first, count);
        assert(glGetError() == GL_NO_ERROR);
    }

    struct Uniform
    {
        GLint unwrap;
    };

    template <typename T>
    struct Vec2
    {
        T x, y;
    };

    using Vec2f = Vec2<GLfloat>;

    ALWAYS_INLINE Uniform getUniformLocation(Program program, const GLchar *name)
    {
        auto location = glGetUniformLocation(program.unwrap, name);
        assert(glGetError() == GL_NO_ERROR);
        return Uniform {location};
    }

    ALWAYS_INLINE void uniform(Uniform uniform, Vec2<GLfloat> vec)
    {
        glUniform2f(uniform.unwrap, vec.x, vec.y);
        assert(glGetError() == GL_NO_ERROR);
    }

    ALWAYS_INLINE void uniform(Uniform uniform, GLfloat x)
    {
        glUniform1f(uniform.unwrap, x);
        assert(glGetError() == GL_NO_ERROR);
    }

    ALWAYS_INLINE void uniform(Uniform uniform, GLsizei count, GLint *xs)
    {
        glUniform1iv(uniform.unwrap, count, xs);
        assert(glGetError() == GL_NO_ERROR);
    }

    struct Vertex_Array
    {
        GLuint unwrap;
    };

    ALWAYS_INLINE Vertex_Array genVertexArray()
    {
        Vertex_Array result = {};
        glGenVertexArrays(1, &result.unwrap);
        assert(glGetError() == GL_NO_ERROR);
        return result;
    }

    ALWAYS_INLINE void bindVertexArray(Vertex_Array array)
    {
        glBindVertexArray(array.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    struct Buffer
    {
        GLuint unwrap;
    };

    ALWAYS_INLINE Buffer genBuffer()
    {
        Buffer result = {};
        glGenBuffers(1, &result.unwrap);
        assert(glGetError() == GL_NO_ERROR);
        return result;
    }

    // TODO: gl::genBuffer for several buffer

    enum Buffer_Target
    {
        ARRAY_BUFFER         = GL_ARRAY_BUFFER,
        ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
        PIXEL_PACK_BUFFER    = GL_PIXEL_PACK_BUFFER,
        PIXEL_UNPACK_BUFFER  = GL_PIXEL_UNPACK_BUFFER
    };

    enum Buffer_Usage
    {
        STREAM_DRAW   = GL_STREAM_DRAW,
        STREAM_READ   = GL_STREAM_READ,
        STREAM_COPY   = GL_STREAM_COPY,
        STATIC_DRAW   = GL_STATIC_DRAW,
        STATIC_READ   = GL_STATIC_READ,
        STATIC_COPY   = GL_STATIC_COPY,
        DYNAMIC_DRAW  = GL_DYNAMIC_DRAW,
        DYNAMIC_READ  = GL_DYNAMIC_READ,
        DYNAMIC_COPY  = GL_DYNAMIC_COPY
    };

    void bindBuffer(Buffer_Target target, Buffer buffer)
    {
        glBindBuffer((GLenum) target, buffer.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    void bufferData(Buffer_Target  target,
                    GLsizeiptr  size,
                    const GLvoid *data,
                    Buffer_Usage  usage)
    {
        glBufferData((GLenum) target, size, data, (GLenum) usage);
        assert(glGetError() == GL_NO_ERROR);
    }

    struct Attribute_ID
    {
        GLuint unwrap;
    };

    ALWAYS_INLINE void enableVertexAttribArray(Attribute_ID index)
    {
        glEnableVertexAttribArray(index.unwrap);
        assert(glGetError() == GL_NO_ERROR);
    }

    enum Attribute_Size
    {
        ATTRIB_SIZE_1 = 1,
        ATTRIB_SIZE_2 = 2,
        ATTRIB_SIZE_3 = 3,
        ATTRIB_SIZE_4 = 4
    };

    ALWAYS_INLINE Maybe<Attribute_ID> getAttribLocation(Program program,
                                                        const GLchar * name)
    {
        GLint id = glGetAttribLocation(program.unwrap, name);
        assert(glGetError() == GL_NO_ERROR);
        if (id < 0) {
            return {};
        } else {
            return {true, {(GLuint)id}};
        }
    }

    enum Attribute_Type
    {
        HALF_FLOAT                  = GL_HALF_FLOAT,
        FLOAT                       = GL_FLOAT,
        DOUBLE                      = GL_DOUBLE,
        INT_2_10_10_10_REV          = GL_INT_2_10_10_10_REV,
        UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV
    };

    ALWAYS_INLINE void vertexAttribPointer(Attribute_ID index,
                                           Attribute_Size size,
                                           Attribute_Type type,
                                           GLboolean normalized,
                                           GLsizei  stride,
                                           const GLvoid *pointer)
    {
        glVertexAttribPointer(
            index.unwrap,
            (GLint) size,
            (GLenum) type,
            normalized,
            stride,
            pointer);
        assert(glGetError() == GL_NO_ERROR);
    }

    enum Attribute_IType
    {
        BYTE            = GL_BYTE,
        UNSIGNED_BYTE   = GL_UNSIGNED_BYTE,
        SHORT           = GL_SHORT,
        UNSIGNED_SHORT  = GL_UNSIGNED_SHORT,
        INT             = GL_INT,
        UNSIGNED_INT    = GL_UNSIGNED_INT
    };

    ALWAYS_INLINE
    void vertexAttribIPointer(Attribute_ID index,
                              GLint size,
                              Attribute_IType type,
                              GLsizei stride,
                              const GLvoid *pointer)
    {
        glVertexAttribIPointer(index.unwrap,
        size, (Attribute_Type) type, stride, pointer);
        assert(glGetError() == GL_NO_ERROR);
    }

    enum String_Name
    {
        VENDOR                    = GL_VENDOR,
        RENDERER                  = GL_RENDERER,
        VERSION                   = GL_VERSION,
        SHADING_LANGUAGE_VERSION  = GL_SHADING_LANGUAGE_VERSION,
        EXTENSIONS                = GL_EXTENSIONS
    };

    ALWAYS_INLINE
    const GLubyte *getString(String_Name name)
    {
        return glGetString((GLenum)name);
    }

    enum Element_Index_Type
    {
        EIT_UNSIGNED_BYTE  = GL_UNSIGNED_BYTE,
        EIT_UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
        EIT_UNSIGNED_INT   = GL_UNSIGNED_INT,
    };

    void drawElements(Mode mode,
                      GLsizei count,
                      Element_Index_Type type,
                      const GLvoid *indices)
    {
        glDrawElements((GLenum) mode, count, (GLenum) type, indices);
        assert(glGetError() == GL_NO_ERROR);
    }

    void bindAttribLocation(Program program,
                            Attribute_ID index,
                            const GLchar *name)
    {
        glBindAttribLocation(program.unwrap,
                             index.unwrap,
                             name);
        assert(glGetError() == GL_NO_ERROR);
    }

}

#endif  // GL_HPP
