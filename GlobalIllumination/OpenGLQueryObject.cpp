#include "OpenGLQueryObject.h"

void OpenGLQueryObject::getShaderBuffersInfo(int program)
{
    GLint numBlocks = 0;
    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
    const GLenum blockProperties[3] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH, GL_BUFFER_BINDING };
    const GLenum activeUnifProp[1] = { GL_ACTIVE_VARIABLES };
    const GLenum unifProperties[4] = { GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_TOP_LEVEL_ARRAY_STRIDE };
    for (int blockIx = 0; blockIx < numBlocks; ++blockIx)
    {
        GLint numActiveUnifs[3];
        glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, blockIx, 3, blockProperties, 3, NULL, numActiveUnifs);

        if (!numActiveUnifs[0])
            continue;

        std::string blockName;
        glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, blockIx, numActiveUnifs[1], NULL, &blockName[0]);
        std::cout << "SSBO (" << std::to_string(numActiveUnifs[2]) << "): " << blockName.c_str() << " { \n";
        std::vector<GLint> blockUnifs(numActiveUnifs[0]);
        glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, blockIx, 1, activeUnifProp, numActiveUnifs[0], NULL, &blockUnifs[0]);
        std::vector<std::pair<GLuint, std::string>> blockMembers(numActiveUnifs[0]);
        for (int unifIx = 0; unifIx < numActiveUnifs[0]; ++unifIx)
        {
            GLint values[4];
            glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, blockUnifs[unifIx], 4, unifProperties, 4, NULL, values);
            blockMembers[unifIx].first = values[2];
            // Get the name. Must use a std::vector rather than a std::string for C++03 standards issues.
            // C++11 would let you use a std::string directly.
            std::vector<char> nameData(values[0]);
            glGetProgramResourceName(program, GL_BUFFER_VARIABLE, blockUnifs[unifIx], nameData.size(), NULL, &nameData[0]);
            std::string name(nameData.begin(), nameData.end() - 1);
            blockMembers[unifIx].second = "  " + name + ", offset: " + std::to_string(values[2]) + ", top-level-stride: " + std::to_string(values[3]) + "\n";
        }
        std::sort(blockMembers.begin(), blockMembers.end(), [](const std::pair<GLuint, std::string> & a, const std::pair<GLuint, std::string> & b) {
            return a.first < b.first;
        });
        for(auto& p : blockMembers) {
            std::cout << p.second;
        }
        std::cout << "} \n";
    }
}
