# gl.hpp

**WARNING! The project is in an active development state and is not
even alpha yet. Use it at your own risk. Nothing is documented,
anything can be changed at any moment or stop working at all.**

Typesafe wrappers for OpenGL. Heavely inspired by [Haskell OpenGL
bindings](https://github.com/haskell-opengl/OpenGL/).

## Goal

The goal is to establish semantic connections between OpenGL
functions using strong types like `Program`, `Shader`, `Buffer`, etc,
that are not just numbers but separate incompatible with each other
types.

For example, let's consider two functions and two types:

```c++
namespace gl {
    // ...

    enum class Shader_Type
    {
        Vertex = GL_SHADER_VERTEX,
        Fragment = GL_SHADER_FRAGMENT,
        Geometry = GL_SHADER_GEOMETRY
    };

    struct Shader
    {
        GLuint unwrap;
    };

    Shader createShader(Shader_Type type);
    void compileShader(Shader shader);

    // ...
}
```

1. `gl::Shader_Type` and `gl::Shader` make it ~~impossible~~ difficult
to pass incorrect values to `gl::createShader` and
`gl::compileShader`.
2. the signatures of the functions "document" the fact `Shader` comes
from `gl::createShader` and should be put into `gl::compileShader`
later.

One thing to note is that all of the above is probably a non-problem for an
experienced OpenGL programmer since they have probably memorized all of the
semantic function connections and pitfalls of the API. This library is aimed
at experienced programmers who's only learning OpenGL.