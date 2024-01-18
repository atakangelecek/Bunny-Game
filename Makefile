all:
	g++ -std=c++11 -I/usr/include/glm -I/usr/include/GL -I/usr/include/GLFW -I/usr/include/freetype2 main.cpp -L/usr/lib -lGLEW -lGL -lglfw -lfreetype -o main