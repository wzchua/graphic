#include "OpenGLTimer.h"

void OpenGLTimer::setTimestamp()
{
    GLuint query;
    glGenQueries(1, &query);
    glQueryCounter(query, GL_TIMESTAMP);
    mQueries.push_back(query);
}

std::vector<GLuint64> OpenGLTimer::getElapsedTime()
{
    std::vector<GLuint64> elapsedTimes;
    int done = 0;
    auto lastQuery = mQueries.back();
    while (!done) {
        glGetQueryObjectiv(lastQuery,
            GL_QUERY_RESULT_AVAILABLE,
            &done);
    }
    GLuint64 timerStart, timerEnd;
    glGetQueryObjectui64v(mQueries[0], GL_QUERY_RESULT, &timerStart);
    for (int i = 1; i < mQueries.size(); i++) {
        glGetQueryObjectui64v(mQueries[i], GL_QUERY_RESULT, &timerEnd);
        elapsedTimes.push_back(timerEnd - timerStart);
        timerStart = timerEnd;
    }
    return elapsedTimes;
}

void OpenGLTimer::clearAll()
{
    glDeleteQueries(mQueries.size(), mQueries.data());
    mQueries.clear();
}

OpenGLTimer::~OpenGLTimer()
{
    clearAll();
}
