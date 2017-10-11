objects = main.o Scene.o Renderer.o ShaderProgram.o Voxelizer.o ObjLoader.o Shape.o Camera.o glad.o tiny_obj_loader.o

LIBS = -lGL -lGLU -lglfw -ldl

IDIR = include
LDIR = lib64

gi: $(objects) $(LIBS)
	g++ -o $@ $^ -I$(IDIR)

main.o : main.cpp
	g++ -c $< -I$(IDIR)
ObjLoader.o : ObjLoader.cpp
	g++ -c $< -I$(IDIR)
Shape.o : Shape.cpp
	g++ -c $< -I$(IDIR)
Camera.o : Camera.cpp
	g++ -c $< -I$(IDIR)
Scene.o : Scene.cpp
	g++ -c $< -I$(IDIR)
Renderer.o : Renderer.cpp
	g++ -c $< -I$(IDIR)
ShaderProgram.o : ShaderProgram.cpp
	g++ -c $< -I$(IDIR)
Voxelizer.o : Voxelizer.cpp
	g++ -c $< -I$(IDIR)
tiny_obj_loader.o : tiny_obj_loader.cc
	g++ -c $< -I$(IDIR)

glad.o : glad.c
	g++ -c $< -I$(IDIR)

