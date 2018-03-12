#pragma once
#include <string>

struct FragStruct {
    float position[4];
    float color[4];
    float normal[4];
};

static std::string fragStructShaderCodeString(GLuint level) {
    std::string s = R"(struct FragmentStruct {
    vec4 position;
    vec4 color;
    vec4 normal;
};
layout(binding = )" + std::to_string(level);
    s = s + R"() coherent buffer FragmentListBlock {
    FragmentStruct frag[];
};
)";
    return s;
};