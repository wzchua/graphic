#include "OpenGLQueryObject.h"

void OpenGLQueryObject::getShaderBuffersInfo(int program)
{
    GLint numBlocks = 0;
    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
    const GLenum blockProperties[3] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH, GL_BUFFER_BINDING };
    const GLenum activeUnifProp[1] = { GL_ACTIVE_VARIABLES };
    const GLenum unifProperties[3] = { GL_NAME_LENGTH, GL_TYPE, GL_OFFSET };
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

        for (int unifIx = 0; unifIx < numActiveUnifs[0]; ++unifIx)
        {
            GLint values[3];
            glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, blockUnifs[unifIx], 3, unifProperties, 3, NULL, values);

            // Get the name. Must use a std::vector rather than a std::string for C++03 standards issues.
            // C++11 would let you use a std::string directly.
            std::vector<char> nameData(values[0]);
            glGetProgramResourceName(program, GL_BUFFER_VARIABLE, blockUnifs[unifIx], nameData.size(), NULL, &nameData[0]);
            std::string name(nameData.begin(), nameData.end() - 1);
            std::cout << "  " << name << ", offset: " << values[2] << "\n";
        }
        std::cout << "} \n";
    }
}
