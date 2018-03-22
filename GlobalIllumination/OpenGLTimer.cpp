#include "OpenGLTimer.h"

long OpenGLTimer::timeTillGPUIsFree(std::string msg, bool showOnConsole)
{
    using Clock = std::chrono::high_resolution_clock;
    auto timeStart = Clock::now();
    auto syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    GLenum waitReturn = GL_UNSIGNALED;
    while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
    {
        waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 2);
    }
    glDeleteSync(syncObj);
    auto timeAfterRender = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
    if (showOnConsole) {
        std::cout << msg << ", time: " << std::to_string(timeAfterRender) << " ms\n";
    }
    return timeAfterRender;
}
