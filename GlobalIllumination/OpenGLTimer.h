#pragma once
#include <glad\glad.h>
#include <string>
#include <chrono>
#include <iostream>

class OpenGLTimer
{
public:
    static long timeTillGPUIsFree(std::string msg, bool showOnConsole = true);
};

