objects = main.o ObjLoader.o Shape.o Camera.o glad.o tiny_obj_loader.o

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
tiny_obj_loader.o : tiny_obj_loader.cc
	g++ -c $< -I$(IDIR)

glad.o : glad.c
	g++ -c $< -I$(IDIR)

