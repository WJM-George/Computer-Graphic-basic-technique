/*
  CSCI 420 Computer Graphics, Computer Science, USC
  Assignment 1: Height Fields with Shaders.
  C/C++ starter code

  Student username: <kammanwu@usc.edu>
*/
// Name: Wu Kam Man, USC ID: 3344-7164-09

#include "openGLHeader.h"
#include "glutHeader.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "pipelineProgram.h"
#include "vbo.h"
#include "vao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <glm/glm.hpp> 
#include <iostream>
#include <cstring>
#include <memory>
#include <glm/gtc/matrix_transform.hpp> //
#include <glm/gtc/type_ptr.hpp> 
#include <algorithm>
#include <chrono>
#include <thread>

#if defined(WIN32) || defined(_WIN32)
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper";
#endif

using namespace std;

int mousePos[2]; // x,y screen coordinates of the current mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// Transformations of the terrain.
float terrainRotate[3] = { 0.0f, 0.0f, 0.0f };
// terrainRotate[0] gives the rotation around x-axis (in degrees)
// terrainRotate[1] gives the rotation around y-axis (in degrees)
// terrainRotate[2] gives the rotation around z-axis (in degrees)
float terrainTranslate[3] = { 0.0f, 0.0f, 0.0f };
float terrainScale[3] = { 1.0f, 1.0f, 1.0f };

// Width and height of the OpenGL window, in pixels.
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 Homework 2";

// Number of vertices in the single triangle (starter code).
//int numVerticesPoint;
int numVerticesLine;
//int numVerticesTriangle;
// Setting My using global variable
int renderingMode = 0; // '0' for mode 1, '1' for smooth mode2
int picMode = 0; // '0' for point, '1' for line, '2' for triangle
// '3' for mode 2 
float scale = 1.0f; // constants are shader uniform variables
float exponent = 1.0f;
const int PPOINTT = 0;
const int LINE = 1;
const int TRIANGLE = 2;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
PipelineProgram pipelineProgram;

// level4
GLuint texHandle;
// need to create a new pipelineProgram:
PipelineProgram pipelineProgramTexture;
VBO vboTexturePos;
VBO vboTextureUV;
VAO vaoTextureMap;
glm::vec3 p0_tex = glm::vec3(-400.0f, -30.0f, -400.0f);  // Bottom-left
glm::vec3 p1_tex = glm::vec3(400.0f, -30.0f, -400.0f);   // Bottom-right
glm::vec3 p2_tex = glm::vec3(-400.0f, -30.0f, 400.0f);   // Top-left
glm::vec3 p3_tex = glm::vec3(400.0f, -30.0f, 400.0f);    // Top-right
std::vector<float> triVec;
std::vector<float> uvsVec;
int vertCount = 0;

VBO vboVertices;
VBO vboColors;
// for line and triangle mode
VBO vboLine;
VBO vboColorLine;
VBO vboTriangle;
VBO vboColorTriangle;
//
// 
// for cross section
VBO vboTube;
VBO vboColorTube;
VAO vaoTube;
//
//
//
//
//
VAO vaoPoint;
VAO vaoLine;
VAO vaoTriangle;

//
//

//
//
// for saving pics and animation:
int save = 0; // 0 for not save; 1 for saved as ss
int frameCount = 0; // To count the frames saved
const int maxFrame = 900;
//const int fps = 15; 
//const int frameDelayMs = 1000 / fps; // Delay between frames in milliseconds

// Write a screenshot to the specified filename.
void saveScreenshot(const char* filename)
{
	std::unique_ptr<unsigned char[]> screenshotData = std::make_unique<unsigned char[]>(windowWidth * windowHeight * 3);
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData.get());

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData.get());

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;
}

void idleFunc()
{
	// Do some stuff... 
	// For example, here, you can save the screenshots to disk (to make the animation).
	// for saving the pics
   // save if save option is valid.
	// Save one frame if saving is enabled and we haven't reached the maximum frame count.
	if (save == 1 && frameCount < maxFrame) {
		// 001.jpg, 002.jpg, etc.
		char fileName[16];
		snprintf(fileName, sizeof(fileName), "%03d.jpg", frameCount);

		//one jpeg per frame
		saveScreenshot(fileName);
		frameCount++;
		save = 0;
	}
	glutPostRedisplay();
}
// start animation
void startCapturing() {
	save = 1; //able to save
	idleFunc(); // video record.
}

int initTexture(const char* imageFilename, GLuint textureHandle)
{
	// Read the texture image.
	ImageIO img;
	ImageIO::fileFormatType imgFormat;
	ImageIO::errorType err = img.load(imageFilename, &imgFormat);

	if (err != ImageIO::OK)
	{
		printf("Loading texture from %s failed.\n", imageFilename);
		return -1;
	}

	// Check that the number of bytes is a multiple of 4.
	if (img.getWidth() * img.getBytesPerPixel() % 4)
	{
		printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
		return -1;
	}

	// Allocate space for an array of pixels.
	int width = img.getWidth();
	int height = img.getHeight();
	unsigned char* pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

	// Fill the pixelsRGBA array with the image pixels.
	memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++)
		{
			// assign some default byte values (for the case where img.getBytesPerPixel() < 4)
			pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
			pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
			pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
			pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

			// set the RGBA channels, based on the loaded image
			int numChannels = img.getBytesPerPixel();
			for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
				pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
		}

	// Bind the texture.
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	// Initialize the texture.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

	// Generate the mipmaps for this texture.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set the texture parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Query support for anisotropic texture filtering.
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	printf("Max available anisotropic samples: %f\n", fLargest);
	// Set anisotropic texture filtering.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

	// Query for any errors.
	GLenum errCode = glGetError();
	if (errCode != 0)
	{
		printf("Texture initialization error. Error code: %d.\n", errCode);
		return -1;
	}

	// De-allocate the pixel array -- it is no longer needed.
	delete[] pixelsRGBA;

	return 0;
}




// projection matrix can be computed here.
void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);

	// When the window has been resized, we need to re-set our projection matrix.
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.LoadIdentity();
	// You need to be careful about setting the zNear and zFar. 
	// Anything closer than zNear, or further than zFar, will be culled.
	const float zNear = 0.001f;
	const float zFar = 1000.0f;
	const float humanFieldOfView = 60.0f;
	matrix.Perspective(humanFieldOfView, 1.0f * w / h, zNear, zFar);
}

void mouseMotionDragFunc(int x, int y)
{
	// Mouse has moved, and one of the mouse buttons is pressed (dragging).

	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState)
	{
		// translate the terrain
		// maybe you can add a extra one key to strench the width(or length of ...)
	case TRANSLATE:
		if (leftMouseButton)
		{
			// control x,y translation via the left mouse button
			terrainTranslate[0] += mousePosDelta[0] * 0.01f;
			terrainTranslate[1] -= mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z translation via the middle mouse button
			terrainTranslate[2] += mousePosDelta[1] * 0.01f;
		}
		break;

		// rotate the terrain
	case ROTATE:
		if (leftMouseButton)
		{
			// control x,y rotation via the left mouse button
			terrainRotate[0] += mousePosDelta[1];
			terrainRotate[1] += mousePosDelta[0];
		}
		if (middleMouseButton)
		{
			// control z rotation via the middle mouse button
			terrainRotate[2] += mousePosDelta[1];
		}
		break;

		// scale the terrain
	case SCALE:
		if (leftMouseButton)
		{
			// control x,y scaling via the left mouse button
			terrainScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
			terrainScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z scaling via the middle mouse button
			terrainScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
	// Mouse has moved.
	// Store the new mouse position.
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
	// A mouse button has has been pressed or depressed.
	// Keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables.
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		leftMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_MIDDLE_BUTTON:
		middleMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_RIGHT_BUTTON:
		rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// Keep track of whether CTRL and SHIFT keys are pressed.
	switch (glutGetModifiers())
	{
		//case GLUT_ACTIVE_CTRL:
			//controlState = TRANSLATE;
			//break;

		//case GLUT_ACTIVE_SHIFT:
			//controlState = SCALE;
			//break;

			// If CTRL and SHIFT are not pressed, we are in rotate mode.
	default:
		controlState = ROTATE;
		break;
	}

	// Store the new mouse position.
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	case ' ':
		cout << "You pressed the spacebar." << endl;
		break;

	case 'j':
		// Take a screenshot.
		// change this using above setting
		startCapturing();
		break;
	case 'k':
		startCapturing();
		break;
	}
	glutPostRedisplay();
}

// extra function in hw2: 
// Represents one spline control point.
struct Point
{
	float x, y, z;

	Point operator+(const Point& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	Point operator-(const Point& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}

	Point operator*(float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}
	// Conversion to glm::vec3
	operator glm::vec3() const {
		return glm::vec3(x, y, z);
	}
};

// Contains the control points of the spline.
struct Spline
{
	int numControlPoints;
	Point* points;
} spline;

void loadSpline(char* argv)
{
	FILE* fileSpline = fopen(argv, "r");
	if (fileSpline == NULL)
	{
		printf("Cannot open file %s.\n", argv);
		exit(1);
	}

	// Read the number of spline control points.
	// original:
	// fscanf(fileSpline, "%d\n", &spline.numControlPoints);

	if (fscanf(fileSpline, "%d\n", &spline.numControlPoints) != 1) {
		// Handle the error, e.g., print an error message or exit
		fprintf(stderr, "Error reading number of control points\n");
		exit(1);
	}
	printf("Detected %d control points.\n", spline.numControlPoints);

	// Allocate memory.
	spline.points = (Point*)malloc(spline.numControlPoints * sizeof(Point));
	// Load the control points.
	for (int i = 0; i < spline.numControlPoints; i++)
	{
		if (fscanf(fileSpline, "%f %f %f",
			&spline.points[i].x,
			&spline.points[i].y,
			&spline.points[i].z) != 3)
		{
			printf("Error: incorrect number of control points in file %s.\n", argv);
			exit(1);
		}
		//printf("Control Point %d: (%f, %f, %f)\n", i, spline.points[i].x, spline.points[i].y, spline.points[i].z);
	}
}

// Multiply C = A * B, where A is a m x p matrix, and B is a p x n matrix.
// All matrices A, B, C must be pre-allocated (say, using malloc or similar).
// The memory storage for C must *not* overlap in memory with either A or B. 
// That is, you **cannot** do C = A * C, or C = C * B. However, A and B can overlap, and so C = A * A is fine, as long as the memory buffer for A is not overlaping in memory with that of C.
// Very important: All matrices are stored in **column-major** format.
// Example. Suppose 
//      [ 1 8 2 ]
//  A = [ 3 5 7 ]
//      [ 0 2 4 ]
//  Then, the storage in memory is
//   1, 3, 0, 8, 5, 2, 2, 7, 4. 
void MultiplyMatrices(int m, int p, int n, const float* A, const float* B, float* C)
{
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			float entry = 0.0;
			for (int k = 0; k < p; k++)
				entry += A[k * m + i] * B[j * p + k];
			C[m * j + i] = entry;
		}
	}
}

// Catmull-Rom Points
Point generateCatmullRomSpline(const Point& p0, const Point& p1, const Point& p2, const Point& p3, float u) {
	const float s = 0.5f; // s val
	// C
	glm::mat3x4 control = glm::mat3x4(
		p0.x, p1.x, p2.x, p3.x,
		p0.y, p1.y, p2.y, p3.y,
		p0.z, p1.z, p2.z, p3.z
	);

	// B
	glm::mat4 basis = glm::mat4(
		-s, 2 * s, -s, 0,
		2 - s, s - 3, 0, 1,
		s - 2, 3 - 2 * s, s, 0,
		s, -s, 0, 0
	);

	// U
	glm::vec4 uu(u * u * u, u * u, u, 1.0f);

	// spline point
	glm::vec3 point = glm::transpose(control) * glm::transpose(basis) * uu;
	return { point.x, point.y, point.z };
}

glm::vec3 splineDerivative(const Point& p0, const Point& p1, const Point& p2, const Point& p3, float u) {
	const float s = 0.5f;
	glm::mat4 basis = glm::mat4(
		-s, 2 * s, -s, 0,
		2 - s, s - 3, 0, 1,
		s - 2, 3 - 2 * s, s, 0,
		s, -s, 0, 0
	);

	glm::vec4 uVec(3 * u * u, 2 * u, 1.0f, 0.0f);
	glm::mat3x4 control(p0.x, p1.x, p2.x, p3.x,
		p0.y, p1.y, p2.y, p3.y,
		p0.z, p1.z, p2.z, p3.z);

	return glm::transpose(control) * glm::transpose(basis) * uVec;
}

// make struc for sloan
struct FrenetFrame {
	glm::vec3 T;
	glm::vec3 N;
	glm::vec3 B;
};

// make Frenet Frame using Sloan’s method
FrenetFrame sloanGenerate(const glm::vec3& T0, const glm::vec3& B0, const glm::vec3& T1) {
	FrenetFrame frame;
	frame.T = glm::normalize(T1); // Tangent
	frame.N = glm::normalize(glm::cross(B0, frame.T)); // normal vec
	frame.B = glm::normalize(glm::cross(frame.T, frame.N)); // binormal vec

	return frame;
}

void PrintCameraPosition(const glm::vec3& position) {
	std::cout << "Camera Position: ("
		<< position.x << ", "
		<< position.y << ", "
		<< position.z << ")" << std::endl;
}

// ride part
std::vector<Point> splinePoints;
std::vector<glm::vec3> tangents;
std::vector<glm::vec3> normals;
std::vector<glm::vec3> binormals;
glm::vec3 cameraPos;      // current cam pos
glm::vec3 cameraTangent;  // current tan vec
glm::vec3 cameraNormal;   // current norm vec
glm::vec3 cameraBinormal; // current binorm vec
int currentSegment = 0;
float u = 0.0f;
float gravity = 9.8f;
float maxSpeed = 50.0f;
float minSpeed = 18.0f;
float speed = 25.0f;

void updateCamera(float deltaTime) {
	glm::vec3 currentPoint = splinePoints[currentSegment];
	glm::vec3 nextPoint = splinePoints[currentSegment + 1];

	float heightDifference = nextPoint.y - currentPoint.y;
	float distance = glm::length(nextPoint - currentPoint);
	float slope = 0.0f;
	slope = heightDifference / glm::length(nextPoint - currentPoint);
	if (slope < 0) {
		slope = glm::clamp(slope, -10.0f, -0.01f);
	}
	else if (slope > 0) {
		slope = glm::clamp(slope, 0.13f, 10.0f);
	}
	else {
		slope = 0.0001f;
	}
	//std::cout << "Slope after clamp: " << slope << std::endl;
	// simulate gravity
	if (slope < 0) {
		speed += 0.9 * gravity * glm::abs(slope) * deltaTime;
	}
	else if (slope > 0) {
		speed -= 1.5 * gravity * glm::abs(slope) * deltaTime;
	}

	speed = glm::clamp(speed, minSpeed, maxSpeed);
	//std::cout << "Speed: " << speed << std::endl;
	u += speed * deltaTime;
	// below one is not that applicable
	//(abs(sqrt(2 * gravity * (16.5f - currentPoint.y)) / slope)) * deltaTime;

	// can not change 1.0f here
	if (u > 1.0f) {
		// go next set of control points
		currentSegment += 1;
		u = 0.0f; // reset u for the new segment
		// exceeding check:
		if (currentSegment >= splinePoints.size() - 1) {
			currentSegment = 0;
		}
	}

	// valid segment
	// updating all the things:
	if (currentSegment < splinePoints.size()) {
		glm::vec3 trackPos = glm::vec3(splinePoints[currentSegment].x, splinePoints[currentSegment].y, splinePoints[currentSegment].z);
		float heightAboveTrack = 0.2f;

		//PrintCameraPosition(cameraPos); // to debug
		cameraTangent = tangents[currentSegment];
		cameraNormal = normals[currentSegment];
		cameraBinormal = binormals[currentSegment];

		if (cameraNormal.y < 0.0f) {
			cameraNormal = -cameraNormal; //facing up
		}

		cameraPos = trackPos + (heightAboveTrack * cameraNormal);

		matrix.SetMatrixMode(OpenGLMatrix::ModelView);
		matrix.LoadIdentity();
		matrix.LookAt(cameraPos.x, cameraPos.y, cameraPos.z,
			cameraPos.x + cameraTangent.x, cameraPos.y + cameraTangent.y, cameraPos.z + cameraTangent.z,
			cameraNormal.x, cameraNormal.y, cameraNormal.z);


		// still have viewLightDirection and lightPosition to load:
		// we change the original light into the view version Light:
		glm::mat4 viewMatrix;
		matrix.GetMatrix(glm::value_ptr(viewMatrix));

		glm::vec3 lightDirection = glm::vec3(0.0f, 1.0f, 0.0f); // directional light
		glm::vec4 lightDirectionVec4 = glm::vec4(lightDirection, 0.0f);
		glm::vec4 viewLightDirection = viewMatrix * lightDirectionVec4;

		glm::vec3 finalViewLightDirection = glm::vec3(viewLightDirection);

		GLint h_viewLightDirection = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "viewLightDirection");
		glUniform3fv(h_viewLightDirection, 1, glm::value_ptr(finalViewLightDirection));

		glm::vec3 lightPos = glm::vec3(-1.0f, 1.0f, 1.0f); // position of the light in view space
		GLint h_lightPosition = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "lightPosition");
		glUniform3fv(h_lightPosition, 1, glm::value_ptr(lightPos));

	}
	else {
		return;
	}
	//startCapturing();
}

// level4:

void setTextureUnit(GLint unit)
{
	glActiveTexture(unit); // select texture unit affected by subsequent texture calls 
	// get a handle to the “textureImage” shader variable 
	GLint h_textureImage = glGetUniformLocation(pipelineProgramTexture.GetProgramHandle(), "textureImage");
	// deem the shader variable “textureImage” to read from texture unit “unit” 
	glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}

// cross section part:
std::vector<glm::vec3> globalVertices;
std::vector<glm::vec4> globalTubeColors;
size_t numVerticesCrossSec = 0;
size_t numVerticesColor = 0;

// int numVerticesLines;
// let say v0, v1, v2, we should separate the point inside the v0 to v1 and v1 to v2. 
// using catmull-ROM inside. What matters is how you structured your spline draw.
void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get the translate
	matrix.Translate(terrainTranslate[0], terrainTranslate[1], terrainTranslate[2]);
	matrix.Rotate(terrainRotate[0] / 2, 1.0, 0.0, 0.0); //x-axis
	matrix.Rotate(terrainRotate[1] / 2, 0.0, 1.0, 0.0); //y-axis
	matrix.Rotate(terrainRotate[2] / 2, 0.0, 0.0, 1.0); //z-axis
	matrix.Scale(terrainScale[0], terrainScale[1], terrainScale[2]);

	float modelViewMatrix[16];
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.GetMatrix(modelViewMatrix);

	float projectionMatrix[16];
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.GetMatrix(projectionMatrix);
	// level4:
//glUseProgram(pipelineProgramTexture.GetProgramHandle());
	pipelineProgramTexture.Bind();
	pipelineProgramTexture.SetUniformVariableMatrix4fv("modelViewMatrix", GL_FALSE, modelViewMatrix);
	pipelineProgramTexture.SetUniformVariableMatrix4fv("projectionMatrix", GL_FALSE, projectionMatrix);
	// bind texture after bind vao
	vaoTextureMap.Bind();
	setTextureUnit(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// draw cross section?
	// draw rail tube using triangle indices

	pipelineProgram.Bind();
	pipelineProgram.SetUniformVariableMatrix4fv("modelViewMatrix", GL_FALSE, modelViewMatrix);
	pipelineProgram.SetUniformVariableMatrix4fv("projectionMatrix", GL_FALSE, projectionMatrix);

	// render for to fragment shader:
	glm::vec4 ambientLight = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // RGBA for ambient light
	glm::vec4 diffuseLight = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // white light
	glm::vec4 specularLight = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // white specular highlight

	glm::vec4 ambientCoefficient = glm::vec4(0.4f, 0.3f, 0.2f, 1.0f); // ambient coefficient
	glm::vec4 diffuseCoefficient = glm::vec4(0.7f, 0.4f, 0.3f, 1.0f); // diffuse coefficient
	glm::vec4 specularCoefficient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // specular coefficient
	float shininess = 200.0f; // shininess exponent

	// for ambient diffuse and specular
	GLint h_La = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "La");
	glUniform4fv(h_La, 1, glm::value_ptr(ambientLight));
	GLint h_Ld = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "Ld");
	glUniform4fv(h_Ld, 1, glm::value_ptr(diffuseLight));
	GLint h_Ls = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "Ls");
	glUniform4fv(h_Ls, 1, glm::value_ptr(specularLight));
	GLint h_ka = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "ka");
	glUniform4fv(h_ka, 1, glm::value_ptr(ambientCoefficient));
	GLint h_kd = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "kd");
	glUniform4fv(h_kd, 1, glm::value_ptr(diffuseCoefficient));
	GLint h_ks = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "ks");
	glUniform4fv(h_ks, 1, glm::value_ptr(specularCoefficient));
	GLint h_alpha = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "alpha");
	glUniform1f(h_alpha, shininess);


	//glUseProgram(pipelineProgram.GetProgramHandle());
	vaoTube.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 41688);
	vaoLine.Bind();
	glDrawArrays(GL_LINES, 0, numVerticesLine);
	glBindVertexArray(0);

	updateCamera(0.016);

	glutSwapBuffers();
}

// here is to generate the points that we need to generate triangles in the later section.
void generateSquareCrossSection(const glm::vec3& p,
	const glm::vec3& T, const glm::vec3& N, const glm::vec3& B,
	float halfSize, std::vector<glm::vec3>& vertices) {
	// Calculate the four corners of the square
	// just generate point for one section:
	glm::vec3 normalN = glm::normalize(N);
	glm::vec3 normalB = glm::normalize(B);
	vertices.push_back(p - halfSize * 0.3f * normalN + halfSize * normalB); // bottom right
	vertices.push_back(p + halfSize * 0.3f * normalN + halfSize * normalB); // top right
	vertices.push_back(p + halfSize * 0.3f * normalN - halfSize * normalB); // top left
	vertices.push_back(p - halfSize * 0.3f * normalN - halfSize * normalB); // bottom left
	// if want T shape can add here. And connect them similarly in the CreateRailTube part:
}

// idea of doing this :
// we load in two cross section 1 and 2 for the purpose of constructing the triangles
// Then we use the two set of vertices, in total 8 vertices to make 8 triangles at the side surfaces.
// For two CrossSection: for 1: sequence: bottom right 0, top right 1, top left 2, bottom left 3.
// for 2: sequence: bottom right 4, top right 5, top left 6, bottom left 7.
// Similar for the second one, connect them with specific orders.
// like : 1,2,6 / 1,5,6 / 0,1,5 / 0,4,5 / 0,3,7 / 0,4,7 / 3,2,6 / 3,7,6
// Once you connect them up,we should have the triangles.
// for the color of the vertices: 
// !!!: for each triangle they have 8 different normals, And we just load the normal to the color:
// But remember the color loading should also similar to the sequence of this:
// like : 1,2,6 / 1,5,6 / 0,1,5 / 0,4,5 / 0,3,7 / 0,4,7 / 3,2,6 / 3,7,6
// we just simply assign the color together with the triangle generating:
// push the color whenever we push a vertices.
// maybe we need to add parameters inside the createRailTube, like normal and binormal for crossSection1 and crossSection2:

void createRailTube(const std::vector<glm::vec3>& crossSection1, const std::vector<glm::vec3>& crossSection2,
	std::vector<glm::vec3>& vertices, std::vector<glm::vec4>& colors)
{
	int numVertices = crossSection1.size(); //4 for a square cross-section

	for (int i = 0; i < numVertices; ++i) {
		int nextIndex = (i + 1) % numVertices;

		// triangle 1
		vertices.push_back(crossSection1[i]);
		vertices.push_back(crossSection2[i]);
		vertices.push_back(crossSection2[nextIndex]);

		// Calculate the normal for the first triangle
		// we do not need to do the calculation here.
		// we can just load it from the subdivide, pass it as a parameter to operate here.
		// Also, we need to load 8 triangles inside a single section of tube.
		// For each triangle, each normal is assign to 3 vertices as their color
		glm::vec3 edge1 = crossSection2[i] - crossSection1[i];
		glm::vec3 edge2 = crossSection2[nextIndex] - crossSection1[i];
		glm::vec3 normal1 = glm::normalize(glm::cross(edge1, edge2));

		glm::vec4 vertexColor1 = glm::vec4(normal1, 1.0f);
		vertexColor1 = glm::normalize(vertexColor1);
		colors.push_back(glm::abs(vertexColor1));
		colors.push_back(glm::abs(vertexColor1));
		colors.push_back(glm::abs(vertexColor1));

		vertices.push_back(crossSection1[i]);
		vertices.push_back(crossSection2[nextIndex]);
		vertices.push_back(crossSection1[nextIndex]);

		edge1 = crossSection2[nextIndex] - crossSection1[i];
		edge2 = crossSection1[nextIndex] - crossSection1[i];
		glm::vec3 normal2 = glm::normalize(glm::cross(edge1, edge2));

		glm::vec4 vertexColor2 = glm::vec4(normal2, 1.0f);
		vertexColor2 = glm::normalize(vertexColor2);
		colors.push_back(glm::abs(vertexColor2));
		colors.push_back(glm::abs(vertexColor2));
		colors.push_back(glm::abs(vertexColor2));
	}
}

void subdivide(float u0, float u1, float maxLineLength,
	const Point& p0, const Point& p1, const Point& p2, const Point& p3,
	std::vector<Point>& points,
	std::vector<glm::vec3>& tangents,
	std::vector<glm::vec3>& normals,
	std::vector<glm::vec3>& binormals,
	std::vector<glm::vec3>& vertices,
	std::vector<glm::vec4>& colors)   // Pass for cross-sections
{
	float umid = (u0 + u1) / 2.0f;

	// Generate CR spline points at u0, umid, and u1
	Point x0 = generateCatmullRomSpline(p0, p1, p2, p3, u0);
	Point x1 = generateCatmullRomSpline(p0, p1, p2, p3, u1);
	Point xmid = generateCatmullRomSpline(p0, p1, p2, p3, umid);

	// tan at u0 and u1
	glm::vec3 tangent0 = splineDerivative(p0, p1, p2, p3, u0);
	glm::vec3 tangent1 = splineDerivative(p0, p1, p2, p3, u1);
	glm::vec3 tangentMid = splineDerivative(p0, p1, p2, p3, umid);

	// dist
	float distance = glm::distance(glm::vec3(x1.x, x1.y, x1.z), glm::vec3(x0.x, x0.y, x0.z));

	if (distance > maxLineLength) {
		//if too long subdivide again
		subdivide(u0, umid, maxLineLength, p0, p1, p2, p3,
			points, tangents, normals, binormals, vertices, colors);
		subdivide(umid, u1, maxLineLength, p0, p1, p2, p3,
			points, tangents, normals, binormals, vertices, colors);
	}
	else {
		// start end to segment
		points.push_back(x0);
		points.push_back(x1);

		// push normalized tangents
		tangents.push_back(glm::normalize(tangent0));
		tangents.push_back(glm::normalize(tangent1));

		if (tangents.size() > 1) {
			// points beyond first, use previous binormal

			glm::vec3 B0 = binormals.empty() ? glm::vec3(0.0f, 0.0f, 1.0f) : binormals.back();
			FrenetFrame frame0 = sloanGenerate(tangents[tangents.size() - 2], B0, tangentMid);
			normals.push_back(frame0.N);
			binormals.push_back(frame0.B);

			// N and B for tangent1, ensuring continuity
			FrenetFrame frame1 = sloanGenerate(tangents.back(), frame0.B, glm::normalize(tangentMid));
			normals.push_back(frame1.N);
			binormals.push_back(frame1.B);
		}
		else {
			// init normal and binormal for the very first tangent
			glm::vec3 initialNormal = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), tangent0));
			glm::vec3 initialBinormal = glm::cross(tangent0, initialNormal);
			normals.push_back(initialNormal);
			binormals.push_back(initialBinormal);
		}

		// get all the position vertices here
		// maybe load in the normals and binormals inside:
		// or not? 
		// The normal and binormal here may have problem, might be the same?
		std::vector<glm::vec3> crossSection1, crossSection2;
		generateSquareCrossSection(glm::vec3(x0.x, x0.y, x0.z),
			tangent0, normals.back(), binormals.back(),
			0.05f, crossSection1);

		generateSquareCrossSection(glm::vec3(x1.x, x1.y, x1.z),
			tangent0, normals.back(), binormals.back(),
			0.05f, crossSection2);
		// gen & store tube idxs
		// color compute also here:
		// 
		createRailTube(crossSection1, crossSection2, vertices, colors);
	}
}


void AddQuadToVectors(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	std::vector<float>& triVec, std::vector<float>& uvsVec) {
	// (p0, p1, p3)
	triVec.push_back(p0.x); triVec.push_back(p0.y); triVec.push_back(p0.z);
	uvsVec.push_back(0.0f); uvsVec.push_back(0.0f);  // bottom-left
	++vertCount;

	triVec.push_back(p1.x); triVec.push_back(p1.y); triVec.push_back(p1.z);
	uvsVec.push_back(1.0f); uvsVec.push_back(0.0f);  // bottom-right
	++vertCount;

	triVec.push_back(p3.x); triVec.push_back(p3.y); triVec.push_back(p3.z);
	uvsVec.push_back(1.0f); uvsVec.push_back(1.0f);  // top-right
	++vertCount;

	// (p0, p3, p2)
	triVec.push_back(p0.x); triVec.push_back(p0.y); triVec.push_back(p0.z);
	uvsVec.push_back(0.0f); uvsVec.push_back(0.0f);  // bottom-left
	++vertCount;

	triVec.push_back(p3.x); triVec.push_back(p3.y); triVec.push_back(p3.z);
	uvsVec.push_back(1.0f); uvsVec.push_back(1.0f);  // top-right
	++vertCount;

	triVec.push_back(p2.x); triVec.push_back(p2.y); triVec.push_back(p2.z);
	uvsVec.push_back(0.0f); uvsVec.push_back(1.0f);  // top-left
	++vertCount;
}


void initScene(int argc, char* argv[])
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	if (pipelineProgram.BuildShadersFromFiles(shaderBasePath, "vertexShader.glsl", "fragmentShader.glsl") != 0)
	{
		cout << "Failed to build the pipeline program." << endl;
		throw 1;
	}
	cout << "Successfully built the pipeline program." << endl;

	pipelineProgram.Bind();

	// create new pipeline Program and bind it:
// level4:
	if (pipelineProgramTexture.BuildShadersFromFiles(shaderBasePath, "vertexShaderTexture.glsl", "fragmentShaderTexture.glsl") != 0)
	{
		cout << "Failed to build the pipeline Texture program." << endl;
		throw 1;
	}
	cout << "Successfully built the pipeline Texture program." << endl;

	pipelineProgramTexture.Bind();

	// level4:
	// generate Texture mapping:
	glGenTextures(1, &texHandle);
	int code = initTexture("grass.jpg", texHandle);
	if (code != 0)
	{
		printf("Error loading the texture image.\n");
		exit(EXIT_FAILURE);
	}

	//

	float maxLineLength = 0.2f;
	for (size_t i = 0; i < spline.numControlPoints - 3; i++) {
		const Point& p0 = spline.points[i];
		const Point& p1 = spline.points[i + 1];
		const Point& p2 = spline.points[i + 2];
		const Point& p3 = spline.points[i + 3];
		subdivide(0.0f, 1.0f, maxLineLength, p0, p1, p2, p3, splinePoints, tangents, normals, binormals, globalVertices, globalTubeColors);
	}

	// below are for inserting cross section
	std::vector<float> vertexData;
	std::vector<float> vertexColorData;

	for (const glm::vec3& vt : globalVertices) {
		vertexData.insert(vertexData.end(), { vt.x, vt.y, vt.z });
		//std::cout << "position   " << vt.x << " " << vt.y << " " << vt.z << " " << std::endl;
	}
	for (const glm::vec4& color : globalTubeColors) {
		vertexColorData.insert(vertexColorData.end(), { color.r, color.g , color.b, color.a });
	}

	// right now they are the same:
	std::cout << "Size of globalVertices: " << globalVertices.size() << std::endl;
	std::cout << "Size of globalTubeColors: " << globalTubeColors.size() << std::endl;
	std::cout << "Size of vertexData, All the point there?  " << vertexData.size() << std::endl;
	std::cout << "Normal size  " << normals.size() << std::endl;
	std::cout << "Binormal size  " << binormals.size() << std::endl;
	std::cout << "Spline total Points  " << splinePoints.size() << std::endl;
	numVerticesCrossSec = vertexData.size() / 3;
	//numVerticesColor = vertexColorData.size() / 4;
	//VBO
	// Total vertices : A. # of triangles : A/3 to draw is triangles.
	vboTube.Gen(numVerticesCrossSec, 3, vertexData.data(), GL_STATIC_DRAW);
	vboColorTube.Gen(numVerticesCrossSec, 4, vertexColorData.data(), GL_STATIC_DRAW);
	vaoTube.Gen();
	vaoTube.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTube, "position");
	vaoTube.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboColorTube, "color");

	// below is for line:
	// Transfer points to splineDraw and splineColor vectors
		// Vectors for drawing lines and colors
	std::vector<float> splineDraw;
	std::vector<float> splineColor;
	for (const Point& pt : splinePoints) {
		splineDraw.insert(splineDraw.end(), { pt.x, pt.y, pt.z });
		splineColor.insert(splineColor.end(), { 1.0f, 1.0f, 1.0f, 1.0f }); // white color
	}

	numVerticesLine = splineDraw.size() / 3;
	// Generate VBOs and VAO
	vboLine.Gen(numVerticesLine, 3, splineDraw.data(), GL_STATIC_DRAW);
	vboColorLine.Gen(numVerticesLine, 4, splineColor.data(), GL_STATIC_DRAW);

	vaoLine.Gen();
	vaoLine.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboLine, "position");
	vaoLine.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboColorLine, "color");

	// level4 texture mapping:
	//
	AddQuadToVectors(p0_tex, p1_tex, p2_tex, p3_tex, triVec, uvsVec);

	vboTexturePos.Gen(triVec.size(), 3, triVec.data(), GL_STATIC_DRAW);  // Position VBO
	vboTextureUV.Gen(uvsVec.size(), 2, uvsVec.data(), GL_STATIC_DRAW);      // UV VBO

	vaoTextureMap.Gen();
	vaoTextureMap.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgramTexture, &vboTexturePos, "position");
	vaoTextureMap.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgramTexture, &vboTextureUV, "texCoord");
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <spline file>\n", argv[0]);
		exit(0);
	}
	// load spline before render:
	loadSpline(argv[1]);

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);

	cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

#ifdef __APPLE__
	// This is needed on recent Mac OS X versions to correctly display the window.
	glutReshapeWindow(windowWidth - 1, windowHeight - 1);
#endif

	// Tells GLUT to use a particular display function to redraw.
	glutDisplayFunc(displayFunc);
	// Perform animation inside idleFunc.
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
#ifdef __APPLE__
  // nothing is needed on Apple
#else
  // Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
#endif

	// Perform the initialization.
	initScene(argc, argv);

	// Sink forever into the GLUT loop.
	glutMainLoop();
}

