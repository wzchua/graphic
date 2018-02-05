#pragma once
#include "glm\glm.hpp"

struct LogStruct {
    glm::vec4 position;
    glm::vec4 color;
    unsigned int nodeIndex;
    unsigned int brickIndex;
    unsigned int index1;
    unsigned int index2;
};
class ShaderLogger {
public:
    static void getLogs(GLBufferObject<LogStruct> & ssboLogList, int logCount, std::vector<LogStruct> & logs) {
        LogStruct * ptr = ssboLogList.getPtr();

        for (int i = 0; i < logCount; i++) {
            logs.push_back(ptr[i]);
        }
    }
};