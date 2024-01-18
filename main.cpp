#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[2];
GLuint groundProgram;
GLuint cubeProgram;
int gWidth, gHeight;

GLint modelingMatrixLoc[2];
GLint viewingMatrixLoc[2];
GLint projectionMatrixLoc[2];
GLint eyePosLoc[2];

GLint groundModelingMatrixLoc;
GLint groundViewingMatrixLoc;
GLint groundProjectionMatrixLoc;
GLint groundEyePosLoc;
// vert3.glsl parameter argument locations in the GPU memory
GLint groundScaleLocation;
GLint groundOffsetLocation;
GLint groundColor1Location;
GLint groundColor2Location;


// cube global variables
GLint cubeModelingMatrixLoc;
GLint cubeViewingMatrixLoc;
GLint cubeProjectionMatrixLoc;
GLint cubeEyePosLoc;

GLint cubeScaleLocation;
GLint cubeOffsetLocation;
GLint cubeColor1Location;
GLint cubeLightColorLocation;
GLint cubeLightPosLocation;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);
glm::vec3 offset(0.0, -2.0, -2.0);
glm::vec3 cubeOffset(0.0,-2.0,-10.0);

glm::mat4 groundProjectionMatrix;
glm::mat4 groundViewingMatrix;
glm::mat4 groundModelingMatrix;

glm::mat4 cubeProjectionMatrix;
glm::mat4 cubeViewingMatrix;
glm::mat4 cubeModelingMatrix;

GLuint vaoBunny;
GLuint vaoGround;
GLuint vaoCube1;
GLuint vaoCube2;
GLuint vaoCube3;

// GLADNESS ROTATION
int isRotating = false;
int rotationFrameNumber = 0;
int ROTATION_DEGREE_PER_FRAME = 10;
int GLADNESS_ROTATION_CALL_AMOUNT = 360 / ROTATION_DEGREE_PER_FRAME;
// GLADNESS ROTATION

int activeProgramIndex = 0;
float DISTANCE_BETWEEN_CONSECUTIVE_CUBES = 50.0f;
float GAME_ACCELARATION = 0.0001;
int frameNumber = 0;
bool isGameOver = false;
int isYellowArray[3];
// Randomly initialize elements to 0 or 1

int hittedCube;
bool isRestart = false;

float isMovingRight = false;
float isMovingLeft = false;
float leftSpeed = 0;
float rightSpeed = 0;


// Game Speed
float GAME_SPEED_INCREASE_COEFFICIENT = 0.03f;
float gameSpeed = 1.f;
// Game Speed

int score;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct Object {
	float positionX, positionY, positionZ;
	vector<Vertex> gVertices;
	vector<Texture> gTextures;
	vector<Normal> gNormals;
	vector<Face> gFaces;

	GLuint gVertexAttribBuffer, gIndexBuffer;
	GLint gInVertexLoc, gInNormalLoc;
	int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
};
Object bunny;
Object quad;
Object cube1, cube2, cube3;

float getHorizontalSpeed() {
	cout << "right: " << rightSpeed << endl;
	cout << "left: " << leftSpeed << endl;
	if (bunny.positionX >= 2.8) isMovingRight = false;
	if (bunny.positionX <= -2.8) isMovingLeft = false;
	if (isMovingRight) {
		rightSpeed += 0.05;
	}
	if (isMovingLeft) {
		leftSpeed += 0.05;
	}
	return rightSpeed - leftSpeed;
}

bool ParseObject(const string& fileName, Object& object) {
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						object.gTextures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						object.gNormals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						object.gVertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					object.gFaces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}
	assert(object.gVertices.size() == object.gNormals.size());

	return true;
}


// fileName: < [in]  Name of the shader file
// data: [out] The contents of the file
bool ReadDataFromFile(const string& fileName, string& data) {
	fstream myfile;

	// Open the input l
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

// Load GLSL file
// No need to Modify
GLuint createVS(const char* shaderName) {
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource)) {
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

// No need to Modify
GLuint createFS(const char* shaderName) {
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

GLuint createAndLoadProgram(const char* vertexShaderFileName, const char* fragmentShaderFileName, const bool isText) {
	GLuint programLocation = glCreateProgram();

	// create shader
	GLuint vertexShaderProgramLocation = createVS(vertexShaderFileName);
	GLuint fragmentShaderProgramLocation = createFS(fragmentShaderFileName);

	if (isText) glBindAttribLocation(programLocation, 2, "vertex");

	// attach created shaders to program
	glAttachShader(programLocation, vertexShaderProgramLocation);
	glAttachShader(programLocation, fragmentShaderProgramLocation);

	// Link the program to gpu
	glLinkProgram(programLocation);

	// check the status of link process
	GLint status;
	glGetProgramiv(programLocation, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "Program link failed" << endl;
		exit(-1);
	}

	return programLocation;
}

void initShaders() {

	gProgram[0] = createAndLoadProgram("vert.glsl", "frag.glsl", false);
	gProgram[1] = createAndLoadProgram("vert2.glsl", "frag2.glsl", false);
	groundProgram = createAndLoadProgram("vert3.glsl", "frag3.glsl", false);
	cubeProgram = createAndLoadProgram("cubeVert.glsl", "cubeFrag.glsl", false);


	// Get the GPU memory locations of the uniform variables first two programs
	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}

	// ###################### GROUND ############################
	// Get the GPU locations of the parameters for ground program (Bind parameter names)
	// groundModelingMatrixLoc: The GPU memory location of the parameter named modeling matrix in the ground program (glsl)
	// We are going to need it.
	groundModelingMatrixLoc = glGetUniformLocation(groundProgram, "modelingMatrix");
	groundViewingMatrixLoc = glGetUniformLocation(groundProgram, "viewingMatrix");
	groundProjectionMatrixLoc = glGetUniformLocation(groundProgram, "projectionMatrix");
	groundEyePosLoc = glGetUniformLocation(groundProgram, "eyePos");

	groundScaleLocation = glGetUniformLocation(groundProgram, "scale");
	groundOffsetLocation = glGetUniformLocation(groundProgram, "offset");
	groundColor1Location = glGetUniformLocation(groundProgram, "color1");
	groundColor2Location = glGetUniformLocation(groundProgram, "color2");

	// ######################## CUBE ############################3
	// Get the GPU locations of the parameters for cube program (Bind parameter names)
	// cubeModelingMatrixLoc: The GPU memory location of the parameter named modeling matrix in the cube program (glsl)
	// We are going to need it.
	cubeModelingMatrixLoc = glGetUniformLocation(cubeProgram, "modelingMatrix");
	cubeViewingMatrixLoc = glGetUniformLocation(cubeProgram, "viewingMatrix");
	cubeProjectionMatrixLoc = glGetUniformLocation(cubeProgram, "projectionMatrix");
	cubeEyePosLoc = glGetUniformLocation(cubeProgram, "eyePos");

	cubeScaleLocation = glGetUniformLocation(cubeProgram, "scale");
	cubeOffsetLocation = glGetUniformLocation(cubeProgram, "offset");
	cubeColor1Location = glGetUniformLocation(cubeProgram, "objectColor");
	// cubeColor2Location = glGetUniformLocation(cubeProgram, "color2");
	cubeLightPosLocation = glGetUniformLocation(cubeProgram, "lightPosition");
	cubeLightColorLocation = glGetUniformLocation(cubeProgram, "lightColor");

	// ##################### SCORE TEXT ###########################

}


// No need to change the definition of this method.
// Params:
// GLuint& vao: VAO of the object that you want to get its VBO 
// Object& object: Data in the struct of the related object
void initVBO(GLuint& vao, Object& object) {

	glGenVertexArrays(1, &vao);
	assert(vao > 0);
	glBindVertexArray(vao);
	//cout << "vao = " << vao << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &object.gVertexAttribBuffer);
	glGenBuffers(1, &object.gIndexBuffer);

	assert(object.gVertexAttribBuffer > 0 && object.gIndexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, object.gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.gIndexBuffer);

	object.gVertexDataSizeInBytes = object.gVertices.size() * 3 * sizeof(GLfloat);
	object.gNormalDataSizeInBytes = object.gNormals.size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = object.gFaces.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat[object.gVertices.size() * 3];
	GLfloat* normalData = new GLfloat[object.gNormals.size() * 3];
	GLuint* indexData = new GLuint[object.gFaces.size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < object.gVertices.size(); ++i) {
		vertexData[3 * i] = object.gVertices[i].x;
		vertexData[3 * i + 1] = object.gVertices[i].y;
		vertexData[3 * i + 2] = object.gVertices[i].z;

		minX = std::min(minX, object.gVertices[i].x);
		maxX = std::max(maxX, object.gVertices[i].x);
		minY = std::min(minY, object.gVertices[i].y);
		maxY = std::max(maxY, object.gVertices[i].y);
		minZ = std::min(minZ, object.gVertices[i].z);
		maxZ = std::max(maxZ, object.gVertices[i].z);
	}

	//std::cout << "minX = " << minX << std::endl;
	//std::cout << "maxX = " << maxX << std::endl;
	//std::cout << "minY = " << minY << std::endl;
	//std::cout << "maxY = " << maxY << std::endl;
	//std::cout << "minZ = " << minZ << std::endl;
	//std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < object.gNormals.size(); ++i)
	{
		normalData[3 * i] = object.gNormals[i].x;
		normalData[3 * i + 1] = object.gNormals[i].y;
		normalData[3 * i + 2] = object.gNormals[i].z;
	}

	for (int i = 0; i < object.gFaces.size(); ++i)
	{
		indexData[3 * i] = object.gFaces[i].vIndex[0];
		indexData[3 * i + 1] = object.gFaces[i].vIndex[1];
		indexData[3 * i + 2] = object.gFaces[i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, object.gVertexDataSizeInBytes + object.gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, object.gVertexDataSizeInBytes, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, object.gVertexDataSizeInBytes, object.gNormalDataSizeInBytes, normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying to GPU memory; can free now from CPU memory
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(object.gVertexDataSizeInBytes));
}

void init() {
	// Parse the bunny object
	ParseObject("bunny.obj", bunny);
	ParseObject("quad.obj", quad);
	ParseObject("cube.obj", cube1);
	ParseObject("cube.obj", cube2);
	ParseObject("cube.obj", cube3);

	glEnable(GL_DEPTH_TEST);
	
	// initialize shaders
	initShaders();

	// initialize vertex buffer objects
	initVBO(vaoBunny, bunny);
	initVBO(vaoGround, quad);
	initVBO(vaoCube1, cube1);
	initVBO(vaoCube2, cube1);
	initVBO(vaoCube3, cube1);
}

void drawModel(GLuint vao, Object& object) {
	//glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
	glBindVertexArray(vao); // ilgili vao'yu çizer
	//glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_TRIANGLES, object.gFaces.size() * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0); // ilgili vao'yu unbind'lar
}

static float horizontalSpeed = 0;

glm::mat4 getRotationMatrixAroundY(float degree) {
	return glm::rotate(glm::mat4(1.0), glm::radians(degree), glm::vec3(0, 1, 0));
}

glm::mat4 getRotationMatrixAroundZ(float degree) {
	return glm::rotate(glm::mat4(1.0), glm::radians(degree), glm::vec3(0, 0, 1));
}

glm::mat4 getGladnessRotationMatrix() {
	// update variables
	ROTATION_DEGREE_PER_FRAME = (int) (100.f * (gameSpeed / 3.f) - 20.f) + 1;
	GLADNESS_ROTATION_CALL_AMOUNT = 360 / ROTATION_DEGREE_PER_FRAME;
	//cout << "Rotation Degree per Frame: " << ROTATION_DEGREE_PER_FRAME << endl;
	if (isRotating && (rotationFrameNumber < GLADNESS_ROTATION_CALL_AMOUNT)) {
		rotationFrameNumber++;
		if (rotationFrameNumber == GLADNESS_ROTATION_CALL_AMOUNT) {
			isRotating = false;
			rotationFrameNumber = 0;
		}
		return getRotationMatrixAroundY(rotationFrameNumber * ROTATION_DEGREE_PER_FRAME);
	}
	return glm::mat4(1.0);
}

glm::mat4 generateModelingMatrixOfGround() {
	float angleRad = (float)(10 / 180.0) * M_PI;
	glm::mat4 matTra = glm::translate(glm::mat4(1.0), glm::vec3(0.f, -2.f, -2.f));
	glm::mat4 matSca = glm::scale(glm::mat4(1.0), glm::vec3(3.0, 1.0, 1000.0));
	glm::mat4 matRot = glm::rotate<float>(glm::mat4(1.0), (-180. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 matRz = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(1.0, 0.0, 0.0));

	// Note: Starting from right side, rotate around Y to turn back, then rotate around Z some more at each frame, then translate.
	return matTra * matSca * matRot * matRz;
}

glm::mat4 generateModelingMatrixOfCube() {
	float angleRad = (float)(10 / 180.0) * M_PI;
	glm::mat4 matTra = glm::translate(glm::mat4(1.0), glm::vec3(2.4f, -0.4f, -(DISTANCE_BETWEEN_CONSECUTIVE_CUBES)));
	glm::mat4 matSca = glm::scale(glm::mat4(1.0), glm::vec3(0.4f, 2.0f, 0.5f));
	
	//glm::mat4 matRot = glm::rotate<float>(glm::mat4(1.0), (-180. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	//glm::mat4 matRz = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(1.0, 0.0, 0.0));

	// Note: Starting from right side, rotate around Y to turn back, then rotate around Z some more at each frame, then translate.
	return matTra * matSca; // * matRot * matRz;
}

void setTheArgumentsOfGroundProgram() {
	// Give the arguments (parameter values) of the fragment shader program (frag3.glsl) of the ground program
	glUniform3f(groundColor1Location, 1.f, 0.f, 1.f);
	glUniform3f(groundColor2Location, 0.f, 1.f, 0.f);
	glUniform1f(groundScaleLocation, 1);
	glUniform3f(groundOffsetLocation, offset.x, offset.y, offset.z);

	// Give the arguments (parameter values) of the vertex shader program (vert3.glsl) of the ground program
	glUniformMatrix4fv(groundProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(groundViewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(groundModelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(groundModelingMatrix));
	glUniform3fv(groundEyePosLoc, 0.7, glm::value_ptr(eyePos));
}

void setTheArgumentsOfCubeProgram(int isYellow) {
	// Give the arguments (parameter values) of the fragment shader program (frag3.glsl) of the Cube program
	isYellow == 0 ? glUniform3f(cubeColor1Location, 1.f, 0.f, 0.f) : glUniform3f(cubeColor1Location, 1.f, 1.f, 0.f);
	//glUniform3f(cubeColor1Location, 1.f, 0.f, 0.f);
	glUniform1f(cubeScaleLocation, 1);
	glUniform3f(cubeOffsetLocation, cubeOffset.x, cubeOffset.y, cubeOffset.z);
	glUniform3f(cubeLightPosLocation, 5.f,5.f,5.f);
    glUniform3f(cubeLightColorLocation, 1.f,1.f,1.f);
	
	// Give the arguments (parameter values) of the vertex shader program (vert3.glsl) of the ground program
	glUniformMatrix4fv(cubeProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(cubeViewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(cubeModelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(cubeModelingMatrix));
	glUniform3fv(cubeEyePosLoc, 0.7, glm::value_ptr(eyePos));
}

void cubeColorGenerator() {
	while (true) {
		// Randomly initialize elements to 0 or 1
		int count = 0;
		for (int i = 0; i < 3; i++) {
			isYellowArray[i] = rand() % 2; // rand() % 2 generates either 0 or 1
		}
		for (int i = 0; i < 3; i++) {
			if (isYellowArray[i] == 1) {
				count++;
			}
		}
		if (count == 1) {
			break;
		}
		else {
			continue;
		}
	}
}

void restartScene() {
	bunny.positionX = 0.f;
	bunny.positionY = -1.1f;
	bunny.positionZ = -3.0f;
	offset.z = 0;
	cubeOffset.z = 0;
	gameSpeed = 1.f;
	score = 0;
	cubeColorGenerator();
}

void display() {
	if (isRestart) {
		//cout << "Restarting Scene while playing" << endl;
		restartScene();
	}
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	if (frameNumber == 0) {
		score = 0;
		cubeColorGenerator();
	}
	// ######################## Transform Bunny Start ############################
	static float jumpCoefficient = 0.4;
	static float jumpAngle = 0;
	static float forwardMoveAmount = 0;
	static float rotationDegree = 0;
	// Compute the modeling matrix
	float jumpHeight = jumpCoefficient * sin(gameSpeed * jumpAngle);
	// both jump and go forward
	glm::mat4 matGladnessrotation = getGladnessRotationMatrix();
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.35, 0.35, 0.35));
	glm::mat4 matR = getRotationMatrixAroundY(270);
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(0.f, -1.5f + jumpHeight, -3.f));
	bunny.positionX = 0.f;
	bunny.positionY = -1.4f + jumpHeight;
	bunny.positionZ = -3.0;
	modelingMatrix = matT * matR * matS * matGladnessrotation;

	// Go left/right
	glm::mat4 leftTranslationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(getHorizontalSpeed(), 0.f, 0.f));
	bunny.positionX += getHorizontalSpeed();
	modelingMatrix = leftTranslationMatrix * modelingMatrix;

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[activeProgramIndex]);

	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));
	// Draw the bunny
	drawModel(vaoBunny, bunny); // draw model elleme la!
	jumpAngle += 0.2;
	forwardMoveAmount += 0.05;
	rotationDegree += 5;
	// cout << "bunny.x: " << bunny.positionX << endl;
	// ######################## Transform Bunny End ############################


	// ######################### GROUND Start ##################################
	groundModelingMatrix = generateModelingMatrixOfGround();
	// Switch to the Ground Program in the GPU
	glUseProgram(groundProgram);
	setTheArgumentsOfGroundProgram();
	// Draw the bunny
	// Note: Color is set within the shader or via vertex colors
	drawModel(vaoGround, quad);
	offset.z -= gameSpeed - 0.8f;
	// ########################## GROUND END ###################################


	// ######################### CUBE Start ##################################
	
	cubeModelingMatrix = generateModelingMatrixOfCube();

	cube1.positionX = 2.4f;
	cube1.positionY = -0.4f;
	cube1.positionZ = -(DISTANCE_BETWEEN_CONSECUTIVE_CUBES);
	glUseProgram(cubeProgram);
	setTheArgumentsOfCubeProgram(isYellowArray[0]);
	drawModel(vaoCube1, cube1);

	cubeModelingMatrix = glm::translate(cubeModelingMatrix, glm::vec3(-6.0f, 0.f, 0.f));
	cube2.positionX = -1.5f;
	cube2.positionY = -0.4f;
	cube2.positionZ = -(DISTANCE_BETWEEN_CONSECUTIVE_CUBES);
	glUseProgram(cubeProgram);
	setTheArgumentsOfCubeProgram(isYellowArray[1]);
	drawModel(vaoCube2, cube2);

	cubeModelingMatrix = glm::translate(cubeModelingMatrix, glm::vec3(-6.0f, 0.f, 0.f));
	cube3.positionX = -5.0f;
	cube3.positionY = -0.4f;
	cube3.positionZ = -(DISTANCE_BETWEEN_CONSECUTIVE_CUBES);
	glUseProgram(cubeProgram);
	setTheArgumentsOfCubeProgram(isYellowArray[2]);
	drawModel(vaoCube3, cube3);

	cubeOffset.z += gameSpeed - 0.8;
	cube1.positionZ += cubeOffset.z;
	cube2.positionZ += cubeOffset.z;
	cube3.positionZ += cubeOffset.z;
	// ######################### CUBE End ##################################

	// if bunny passes through the level of cubes
	if (cube1.positionZ >= bunny.positionZ) {
		cubeOffset.z = 0;
		// if the bunny hit the rightmost cube
		if (bunny.positionX <= 3.5 && bunny.positionX > 1.5) {
			// if the cube is red
			if (isYellowArray[0] == 0) {
				// kill the bunny
				//cout << "Game Over!" << endl;
				isGameOver = true;
				hittedCube = 1;
				//lastScene(isYellow);
				//exit(0);
			}
			// if the cube is yellow
			else {
				// rotate the bunny...
				//cout << "Rightmost Yellow" << endl;
				isRotating = true;
				gameSpeed += GAME_SPEED_INCREASE_COEFFICIENT;
				score += 1000;
				// offset.z -= gameSpeed - 0.8;
				// cubeOffset.z += gameSpeed - 0.8;
				//cout << "Game Speed: " << gameSpeed << endl;
				//cout << "Offset: " << offset.z << endl;
				//cout << "Cube Offset: " << cubeOffset.z << endl;
				cubeColorGenerator();
			}
		}
		// if the bunny hit the middle cube
		else if (bunny.positionX < 0.9 && bunny.positionX >= -0.9) {
			// if the cube is red
			if (isYellowArray[1] == 0) {
				// kill the bunny
				//cout << "Game Over!" << endl;
				isGameOver = true;
				hittedCube = 2;
				//lastScene(isYellow);
				//exit(0);
			}
			// if the cube is yellow
			else {
				// rotate the bunny...
				//cout << "Middle Yellow" << endl;
				isRotating = true;
				gameSpeed += GAME_SPEED_INCREASE_COEFFICIENT;
				score += 1000;
				// offset.z -= gameSpeed - 0.8;
				// cubeOffset.z += gameSpeed - 0.8;
				//cout << "Game Speed: " << gameSpeed << endl;
				//cout << "Offset: " << offset.z << endl;
				//cout << "Cube Offset: " << cubeOffset.z << endl;
				cubeColorGenerator();
			}
		}
		// if the bunny hit the leftmost cube
		else if (bunny.positionX < -1.5 && bunny.positionX >= -3.5) {
			// if the cube is red
			if (isYellowArray[2] == 0) {
				// kill the bunny
				//cout << "Game Over!" << endl;
				isGameOver = true;
				hittedCube = 3;
				//lastScene(isYellow);
				//exit(0);
			}
			// if the cube is yellow
			else {
				//cout << "Leftmost Yellow" << endl;
				isRotating = true;
				gameSpeed += GAME_SPEED_INCREASE_COEFFICIENT;
				score += 1000;
				// offset.z -= gameSpeed - 0.8;
				// cubeOffset.z += gameSpeed - 0.8;
				//cout << "Game Speed: " << gameSpeed << endl;
				//cout << "Offset: " << offset.z << endl;
				//cout << "Cube Offset: " << cubeOffset.z << endl;
				cubeColorGenerator();
			}
		}
		else {
			//cout << "Küplerin arasindan geçtik!" << endl;
		}
	}
}

void lastScene(int hittedCube) {
	if (isRestart) {
		//cout << "Restarting Scene after dead" << endl;
		restartScene();
		return;
	}
	cout << "Score: " << score << endl;
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//cout << "Last Scene" << endl;
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.35, 0.35, 0.35));
	glm::mat4 matfirstR = getRotationMatrixAroundY(270);
	glm::mat4 matR = getRotationMatrixAroundZ(-90);
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(bunny.positionX, -2, -2.3f));
	modelingMatrix = matT * matR * matfirstR * matS;
	glUseProgram(gProgram[activeProgramIndex]);

	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));
	// Draw the bunny
	drawModel(vaoBunny, bunny);

	// ######################### GROUND Start ##################################
	offset.z = 0;
	groundModelingMatrix = generateModelingMatrixOfGround();
	// Switch to the Ground Program in the GPU
	glUseProgram(groundProgram);
	setTheArgumentsOfGroundProgram();
	// Draw the bunny
	// Note: Color is set within the shader or via vertex colors
	drawModel(vaoGround, quad);
	// ########################## GROUND END ###################################
	// Deneme için ilk cube'u çizdirmiyom
	glm::mat4 matTra = glm::translate(glm::mat4(1.0), glm::vec3(2.4f, -0.4f, -3.0f));
	glm::mat4 matSca = glm::scale(glm::mat4(1.0), glm::vec3(0.4f, 2.0f, 0.5f));
	cubeModelingMatrix = matTra * matSca;
	if (hittedCube != 1) {
		glUseProgram(cubeProgram);
		setTheArgumentsOfCubeProgram(isYellowArray[0]);
		drawModel(vaoCube1, cube1);
	}
	cubeModelingMatrix = glm::translate(cubeModelingMatrix, glm::vec3(-6.0f, 0.f, 0.f));

	if (hittedCube != 2) {
		glUseProgram(cubeProgram);
		setTheArgumentsOfCubeProgram(isYellowArray[1]);
		drawModel(vaoCube2, cube2);
	}

	cubeModelingMatrix = glm::translate(cubeModelingMatrix, glm::vec3(-6.0f, 0.f, 0.f));

	if (hittedCube != 3) {
		glUseProgram(cubeProgram);
		setTheArgumentsOfCubeProgram(isYellowArray[2]);
		drawModel(vaoCube3, cube3);
	}

	cubeOffset.z = 0;
	//cube1.positionZ += cubeOffset.z;
	//cube2.positionZ += cubeOffset.z;
	//cube3.positionZ += cubeOffset.z;
}
// This is called before the main loop
void reshape(GLFWwindow* window, int w, int h) {
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	// Use perspective projection
	float fovyRad = (float)(90.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)
	// 
	//viewingMatrix = glm::mat4(1);
	viewingMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		activeProgramIndex = 0;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		activeProgramIndex = 1;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		glShadeModel(GL_FLAT);
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		glShadeModel(GL_SMOOTH);
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (bunny.positionX <= -2.8) isMovingLeft = false;
		else isMovingLeft = true;
	}
	else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (bunny.positionX >= 2.8) isMovingRight = false;
		else isMovingRight = true;
	}
	else if (key == GLFW_KEY_A && (action == GLFW_RELEASE|| action == GLFW_REPEAT)) {
		isMovingLeft = false;
	}
	else if (key == GLFW_KEY_D && (action == GLFW_RELEASE|| action == GLFW_REPEAT)) {
		isMovingRight = false;
	}
	else if ( key == GLFW_KEY_R && action == GLFW_PRESS){
        isRestart = true;
		isGameOver = false;
		display();
    }
	else if ( key == GLFW_KEY_R && action == GLFW_RELEASE){
        isRestart = false;
    }
}

void mainLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window)) {
		if (isGameOver == false){
			display();
			glfwSwapBuffers(window);
			glfwPollEvents();
			frameNumber++;
		}
		else {
			lastScene(hittedCube);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
		
	}
}

// Create Main Function For Bringing It All Together
int main(int argc, char** argv) {
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	int width = 1000, height = 800;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	// If the windows cannot be opened, exit the program
	if (!window) {
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	// get the renderer info specific to the computer
	// GL_RENDERER: Name of the renderer
	// renrererInfo: GL_RENDERER - GL_VERSION
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
	glfwSetWindowTitle(window, rendererInfo);

	// initializes all vertex buffer objects
	init();

	// some configurations
	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);
	// need to call this once ourselves
	reshape(window, width, height);

	// main loop does not return unless the window is closed
	mainLoop(window);

	// The game is finished, close everything
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
