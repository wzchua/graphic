#pragma once
#include <exception>
#include <glad/glad.h>
template <class T>
class GLBufferObject
{
private:
    bool hasInitialized = false;
    bool isPersistent = false;
    GLuint id;
    T* bufferPtr;
    GLenum mTarget;
public:
    void initialize(GLenum target, GLsizeiptr size, const void* data, GLbitfield flag, bool isPersistent = false);
    void bind(GLuint binding);
    T* getPtr();
    GLuint getId();
    GLBufferObject();
    ~GLBufferObject();
    //prevent copying
    GLBufferObject(const GLBufferObject&) = delete;
    GLBufferObject& operator = (const GLBufferObject &) = delete;
};


template<class T>
void GLBufferObject<T>::initialize(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags, bool isPersistent)
{
    if (hasInitialized) {
        return;
    }
    mTarget = target;
    this->isPersistent = isPersistent;
    glGenBuffers(1, &id);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(target, id);
    glBufferStorage(target, size, data, flags);
    if (isPersistent) {
        bufferPtr = (T*)glMapBufferRange(target, 0, size, flags);
    }
    glBindBuffer(target, 0);

    hasInitialized = true;
}

template<class T>
void GLBufferObject<T>::bind(GLuint binding)
{
    glBindBufferBase(mTarget, binding, id);
}

template<class T>
T * GLBufferObject<T>::getPtr()
{
    if (!isPersistent) {
        throw new std::exception("not persistent");
    }
    return bufferPtr;
}

template<class T>
GLuint GLBufferObject<T>::getId()
{
    return id;
}

template<class T>
GLBufferObject<T>::GLBufferObject()
{
}


template<class T>
GLBufferObject<T>::~GLBufferObject()
{
    if (hasInitialized) {
        glDeleteBuffers(1, &id);
    }
}