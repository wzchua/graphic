#pragma once
#include "glm\glm.hpp"
#include <string>
#include <vector>

struct LogStruct {
    glm::vec4 position;
    glm::vec4 color;
    unsigned int nodeIndex;
    unsigned int brickIndex;
    unsigned int index1;
    unsigned int index2;
};
static std::string logShaderCodeString(GLuint level) {
    std::string s = R"(struct LogStruct {
    vec4 position;
    vec4 color;
    uint nodeIndex;
    uint brickPtr;
    uint index1;
    uint index2;
};
layout(binding = )" + std::to_string(level);
    s = s + R"() coherent buffer LogBlock{
    LogStruct logList[];
};

void logFragment(vec4 pos, vec4 color, uint nodeIndex, uint brickPtr, uint index1, uint index2) {
    uint index = atomicAdd(logCounter, 1);
    if(index < maxNoOfLogs) {        
        logList[index].position = pos;
        logList[index].color = color;
        logList[index].nodeIndex = nodeIndex;
        logList[index].brickPtr = brickPtr;
        logList[index].index1 = index1;
        logList[index].index2 = index2;
    } else {
        atomicAdd(logCounter, uint(-1));
    }
}
)";
    return s;
}

class ShaderLogger {
public:
    static void getLogs(GLBufferObject<LogStruct> & ssboLogList, int logCount, std::vector<LogStruct> & logs) {
        LogStruct * ptr = ssboLogList.getPtr();

        for (int i = 0; i < logCount; i++) {
            logs.push_back(ptr[i]);
        }
        ssboLogList.unMapPtr();
    }
};