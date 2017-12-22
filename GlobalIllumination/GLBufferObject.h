#pragma once
#include <exception>
#include <glad/glad.h>
class GLBufferObject
{
private:
    bool hasInitialized = false;
    bool isPersistent = false;
    GLuint id;
    void* bufferPtr;
public:
    void initialize(GLenum target, GLsizeiptr size, const void* data, GLbitfield flag, bool isPersistent = false);
    void bind(GLuint binding);
    void* getPtr();
    GLuint getId();
    GLBufferObject();
    ~GLBufferObject();
    //prevent copying
    GLBufferObject(const GLBufferObject&) = delete;
    GLBufferObject& operator = (const GLBufferObject &) = delete;
};

