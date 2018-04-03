#pragma once
#include <glad\glad.h>
#include <string>
#include <chrono>
#include <iostream>
#include <vector>

class OpenGLTimer
{
private:
    std::vector<GLuint64> mTimestamps;
    std::vector<GLuint> mQueries;
public:
    void setTimestamp();
    std::vector<GLuint64> getElapsedTime();
    void clearAll();
    OpenGLTimer() = default;
    ~OpenGLTimer();
};

