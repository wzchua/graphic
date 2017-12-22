#include "GLBufferObject.h"



void GLBufferObject::initialize(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags, bool isPersistent)
{
    if (hasInitialized) {
        return;
    }
    this->isPersistent = isPersistent;
    glGenBuffers(1, &id);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(target, id);
    glBufferStorage(target, size, data, flags);
    if (isPersistent) {
        bufferPtr = glMapBufferRange(target, 0, size, flags);
    }
    glBindBuffer(target, 0);

    hasInitialized = true;
}

void GLBufferObject::bind(GLuint binding)
{
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, binding, id);
}

void * GLBufferObject::getPtr()
{
    if (!isPersistent) {
        throw new std::exception("not persistent");
    }
    return bufferPtr;
}

GLuint GLBufferObject::getId()
{
    return id;
}

GLBufferObject::GLBufferObject()
{
}


GLBufferObject::~GLBufferObject()
{
    if (hasInitialized) {
        glDeleteBuffers(1, &id);
    }
}
