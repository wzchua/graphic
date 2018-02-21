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
    GLsizeiptr mSize;
    T* bufferPtr;
    GLenum mTarget;
public:
    void initialize(GLenum target, GLsizeiptr size, const void* data, GLbitfield flag, GLbitfield access);
    void bind(GLuint binding);
    void clearData();
    T* getPtr();
    void unMapPtr();
    GLuint getId();

    GLBufferObject();
    ~GLBufferObject();
    //prevent copying
    GLBufferObject(const GLBufferObject&) = delete;
    GLBufferObject& operator = (const GLBufferObject &) = delete;
};


template<class T>
inline void GLBufferObject<T>::initialize(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags, GLbitfield access)
{
    if (hasInitialized) {
        return;
    }
    mTarget = target;
    mSize = size;
    isPersistent = (access & GL_MAP_PERSISTENT_BIT) != 0;
    glGenBuffers(1, &id);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(target, id);
    glBufferStorage(target, size, data, flags);
    if (isPersistent) {
        bufferPtr = (T*)glMapBufferRange(target, 0, size, access);
    }
    glBindBuffer(target, 0);

    hasInitialized = true;
}

template<class T>
inline void GLBufferObject<T>::bind(GLuint binding)
{
    glBindBufferBase(mTarget, binding, id);
}

template<class T>
inline void GLBufferObject<T>::clearData()
{
    glClearNamedBufferData(id, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
}

template<class T>
T * GLBufferObject<T>::getPtr()
{
    if (!isPersistent) {        
        T* ptr = (T*)glMapNamedBufferRange(id, 0, mSize, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        return ptr;
    }
    return bufferPtr;
}

template<class T>
inline void GLBufferObject<T>::unMapPtr()
{
    if (isPersistent) {
        return;
    }
    glUnmapNamedBuffer(id);
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