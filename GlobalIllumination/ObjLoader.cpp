#include "ObjLoader.h"

static bool fileExists(const std::string & fileName)
{
    struct stat info;
    int ret = -1;

    ret = stat(fileName.c_str(), &info);
    return 0 == ret;
}

bool ObjLoader::loadObj(const std::string & path, std::vector<Shape>& meshList, std::map<std::string, GLuint> &textureMap)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    auto lastIndex = path.find_last_of('/');
    std::string baseDir = path.substr(0, lastIndex + 1);
    std::cout << baseDir << '\n';
    stbi_set_flip_vertically_on_load(true);

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), baseDir.c_str());

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Load failed: " << path << std::endl;
        return false;
    }

    printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
    printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
    printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
    printf("# of materials = %d\n", (int)materials.size());
    printf("# of shapes    = %d\n", (int)shapes.size());
    materials.push_back(tinyobj::material_t());
    for (size_t i = 0; i < materials.size(); i++) {
        printf("material[%d].diffuse_texname = %s\n", int(i), materials[i].diffuse_texname.c_str());
    }

    std::cout << baseDir << "\n";
    for (auto const& m : materials) {
        if (m.ambient_texname.length() > 0) {
            loadTexture(textureMap, m.ambient_texname, baseDir);
        }
        if (m.diffuse_texname.length() > 0) {
            loadTexture(textureMap, m.diffuse_texname, baseDir);
        }
        if (m.bump_texname.length() > 0) {
            loadTexture(textureMap, m.bump_texname, baseDir);
        }
        if (m.alpha_texname.length() > 0) {
            loadTexture(textureMap, m.alpha_texname, baseDir);
        }
        if (m.specular_texname.length() > 0) {
            loadTexture(textureMap, m.specular_texname, baseDir);
        }
    }


    for (auto const& shape : shapes) {
        std::vector<tinyobj::material_t> matList;
        std::map<int, int> matMap;
        if (shape.mesh.material_ids.size() == 0) {
            matList.push_back(materials[materials.size() - 1]);
        }
        else {
            for (auto const & id : shape.mesh.material_ids) {
                if (matMap.find(id) == matMap.end()) {
                    matMap[id] = matList.size();
                    matList.push_back(materials[id]);
                }
            }
        }
        if (matList.size() > 1) {
            // break different material faces into different shapes
            std::vector<tinyobj::shape_t> newShapes;
            newShapes.reserve(matList.size());
            for (int i = 0; i < matList.size(); i++) {
                newShapes.push_back(tinyobj::shape_t());
                newShapes[i].name = shape.name;
            }
            for (int i = 0; i < shape.mesh.material_ids.size(); i++) {
                int id = shape.mesh.material_ids[i];
                int index = matMap[id];
                newShapes[index].mesh.material_ids.push_back(shape.mesh.material_ids[i]);
                newShapes[index].mesh.num_face_vertices.push_back(shape.mesh.num_face_vertices[i]);
                newShapes[index].mesh.indices.push_back(shape.mesh.indices[i * 3 + 0]);
                newShapes[index].mesh.indices.push_back(shape.mesh.indices[i * 3 + 1]);
                newShapes[index].mesh.indices.push_back(shape.mesh.indices[i * 3 + 2]);
            }

            for (int i = 0; i < matList.size(); i++) {
                meshList.push_back(Shape());
                meshList.back().buildVBO(newShapes[i], attrib, matList[i]);
            }
        }
        else {
            meshList.push_back(Shape());
            meshList.back().buildVBO(shape, attrib, matList[0]);
        }
	}

	return true;
}

void ObjLoader::loadTexture(std::map<std::string, GLuint>& textureMap, const std::string & textureFilename, const std::string & baseDir)
{
    //load if not loaded
    if (textureMap.find(textureFilename) == textureMap.end()) {
        GLuint texture_id;
        int w, h;
        int comp;

        std::string texture_filename = baseDir + textureFilename;
        if (!fileExists(texture_filename)) {
            std::cerr << "Unable to find file " << textureFilename << std::endl;
            exit(-1);
        }
        unsigned char* image = stbi_load(texture_filename.c_str(), &w, &h, &comp, STBI_default);
        if (!image) {
            std::cerr << "Unable to load texture: " << texture_filename << std::endl;
            exit(1);
        }

        std::cout << "Loaded texture: " << texture_filename << ", w = " << w << ", h = " << h << ", comp = " << comp << std::endl;

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (comp == 1) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, image);
        }
        else if (comp == 2) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, w, h, 0, GL_RG, GL_UNSIGNED_BYTE, image);
        } else if (comp == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        }
        else if (comp == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        }
        else {
            assert(0); // TODO
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image);
        textureMap.insert(std::make_pair(textureFilename, texture_id));
    }
}
