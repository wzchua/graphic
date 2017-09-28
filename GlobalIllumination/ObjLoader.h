#pragma once
#include <string>
#include <iostream>
#include <map>
#include <sys/stat.h>

#include <tiny_obj_loader.h>
#include "stb_image.h"

#include "Shape.h"

class ObjLoader {
public:
	static bool loadObj(const std::string & path, std::vector<Shape>& meshList, std::map<std::string, GLuint> &textureMap, glm::vec3 & min, glm::vec3 & max);
    static void loadTexture(std::map<std::string, GLuint> &textureMap, const std::string & textureFilename, const std::string &baseDir);
};