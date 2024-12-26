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

#include <iostream>
#include <cstring>
#include <memory>

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
char windowTitle[512] = "CSCI 420 Homework 1";

// Number of vertices in the single triangle (starter code).
int numVerticesPoint;
int numVerticesLine;
int numVerticesTriangle;
// Setting My using global variable
int renderingMode = 0; // '0' for mode 1, '1' for smooth mode2
int picMode = 0; // '0' for point, '1' for line, '2' for triangle
// '3' for mode 2 
float scale = 1.0f; // constants are shader uniform variables
float exponent = 1.0f;
const int PPOINTT = 0;
const int LINE = 1;
const int TRIANGLE = 2;
const int MIX_TEXTURE = 3;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
PipelineProgram pipelineProgram;
VBO vboVertices;
VBO vboColors;
// for line and triangle mode
VBO vboLines;
VBO vboColorLines;
VBO vboTriangles;
VBO vboColorTriangles;
//
//
VBO vboTriangleTop;
VBO vboTriangleTopColor;
VBO vboTriangleLine;
VBO vboTriangleLineColor;
//
// for smooth mode:
VBO vboPointUp;
VBO vboPointDown;
VBO vboPointLeft;
VBO vboPointRight;
VBO vboTriangleSmooth;
VBO vboTriangleSmoothColor;
VAO vaoTriangleSmooth;
//
//
VAO vaoPoint;
VAO vaoLine;
VAO vaoTriangle;
VAO vaoTriangleWire;
VAO vaoTriangleBot;
//
//

//
//
// for saving pics and animation:
int save = 0; // 0 for not save; 1 for saved as ss
int frameCount = 0; // To count the frames saved
const int maxFrame = 300; // Maximum number of frames for a 250/15 -second video

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

// projection matrix can be computed here.
void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);

	// When the window has been resized, we need to re-set our projection matrix.
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.LoadIdentity();
	// You need to be careful about setting the zNear and zFar. 
	// Anything closer than zNear, or further than zFar, will be culled.
	const float zNear = 0.01f;
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
	case GLUT_ACTIVE_CTRL:
		controlState = TRANSLATE;
		break;

	case GLUT_ACTIVE_SHIFT:
		controlState = SCALE;
		break;

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

	case 'x':
		// Take a screenshot.
		// change this using above setting
		startCapturing();
		break;

	case '1':
		renderingMode = 0; // Points mode
		picMode = PPOINTT;
		break;
	case '2':
		renderingMode = 0; // Lines mode
		picMode = LINE;
		break;
	case '3':
		renderingMode = 0; // Triangles mode
		picMode = TRIANGLE;
		break;
	case '4':
		renderingMode = 1; // Smooth mode
		break;
	case '5':
		renderingMode = 0;
		picMode = MIX_TEXTURE;
		break;
	case '9':
		// expo 
		exponent *= 1.1f;
		pipelineProgram.SetUniformVariablef("exponent", exponent);
		break;
	case '0':
		exponent /= 1.1f;
		pipelineProgram.SetUniformVariablef("exponent", exponent);
		break;

	case '+':
		// keyboard for scale
		scale *= 2.0f;
		pipelineProgram.SetUniformVariablef("scale", scale);
		break;
	case '-':
		scale /= 2.0f;
		pipelineProgram.SetUniformVariablef("scale", scale);
		break;
	}
	glutPostRedisplay();
}

// model view matrix is in displayFunc
// each frame display once
// can call save screenshot at the end.
// self create method to get corresponding screenshot.
void displayFunc()
{
	// This function performs the actual rendering.

	// First, clear the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Second, we set all the matrices
	// Set up the camera position, focus point, and the up vector.
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.LoadIdentity();
	matrix.LookAt(0.0, 0.0, 5.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);
	// In here, you can do additional modeling on the object, such as performing translations, rotations and scales.
	// ...
	// adjustments below: translations, rotations and scales

	// Already done the thing. By adjustment, the view of mode 4 is a bit fart away.
	//Use input from the mouse to rotate the heightfield using OpenGLMatrix::Rotate.
	//Use input from the mouse to move the heightfield using OpenGLMatrix::Translate.
	//Use input from the mouse to change the dimensions of the heightfield using OpenGLMatrix::Scale.
	matrix.Translate(terrainTranslate[0], terrainTranslate[1], terrainTranslate[2]);
	matrix.Rotate(terrainRotate[0], 1.0, 0.0, 0.0); //x-axis
	matrix.Rotate(terrainRotate[1], 0.0, 1.0, 0.0); //y-axis
	matrix.Rotate(terrainRotate[2], 0.0, 0.0, 1.0); //z-axis
	matrix.Scale(terrainScale[0], terrainScale[1], terrainScale[2]);
	//


	// Read the current modelview and projection matrices from our helper class.
	// The matrices are only read here; nothing is actually communicated to OpenGL yet.
	float modelViewMatrix[16];
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.GetMatrix(modelViewMatrix);

	float projectionMatrix[16];
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.GetMatrix(projectionMatrix);

	// Upload the modelview and projection matrices to the GPU. Note that these are "uniform" variables.
	// Important: these matrices must be uploaded to *all* pipeline programs used.
	// In hw1, there is only one pipeline program, but in hw2 there will be several of them.
	// In such a case, you must separately upload to *each* pipeline program.
	// Important: do not make a typo in the variable name below; otherwise, the program will malfunction.
	pipelineProgram.SetUniformVariableMatrix4fv("modelViewMatrix", GL_FALSE, modelViewMatrix);
	pipelineProgram.SetUniformVariableMatrix4fv("projectionMatrix", GL_FALSE, projectionMatrix);

	// rendering mode to shader:
	pipelineProgram.SetUniformVariablei("renderingMode", renderingMode);
	pipelineProgram.SetUniformVariablef("scale", scale);
	pipelineProgram.SetUniformVariablef("exponent", exponent);

	// Execute the rendering. start below:
	// Bind the VAO that we want to render. Remember, one object = one VAO. 
	//vao.Bind();

	// base on different rendering mode we change the draw way:
	// should have two ways to draw the pic.
	if (picMode == PPOINTT && renderingMode == 0)
	{
		vaoPoint.Bind();
		glDrawArrays(GL_POINTS, 0, numVerticesPoint);

	}
	// what to do next, after loading all the pixels
	// connect all lines or triangles to run.
	else if (picMode == LINE && renderingMode == 0)
	{
		vaoLine.Bind();
		glDrawArrays(GL_LINES, 0, numVerticesLine);

	}
	else if (picMode == TRIANGLE && renderingMode == 0)
	{
		vaoTriangle.Bind();
		glDrawArrays(GL_TRIANGLES, 0, numVerticesTriangle);
	}
	else if (picMode == MIX_TEXTURE && renderingMode == 0)
	{
		vaoTriangleWire.Bind();
		glDrawArrays(GL_LINES, 0, numVerticesLine);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(765.0f, 765.0f);
		vaoTriangleBot.Bind();
		glDrawArrays(GL_TRIANGLES, 0, numVerticesTriangle);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	else if (renderingMode == 1) {
		vaoTriangleSmooth.Bind();
		glDrawArrays(GL_TRIANGLES, 0, numVerticesTriangle);
	}
	//glDrawArrays(GL_TRIANGLES, 0, numVertices); // Render the VAO, by rendering "numVertices", starting from vertex 0.

	// Swap the double-buffers.
	glutSwapBuffers();
}

void initScene(int argc, char* argv[])
{
	// Load the image from a jpeg disk file into main memory.
	std::unique_ptr<ImageIO> heightmapImage = std::make_unique<ImageIO>();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}



	// Set the background color.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black color.

	// Enable z-buffering (i.e., hidden surface removal using the z-buffer algorithm).
	glEnable(GL_DEPTH_TEST);

	// Create a pipeline program. This operation must be performed BEFORE we initialize any VAOs.
	// A pipeline program contains our shaders. Different pipeline programs may contain different shaders.
	// In this homework, we only have one set of shaders, and therefore, there is only one pipeline program.
	// In hw2, we will need to shade different objects with different shaders, and therefore, we will have
	// several pipeline programs (e.g., one for the rails, one for the ground/sky, etc.).
	// Load and set up the pipeline program, including its shaders.
	if (pipelineProgram.BuildShadersFromFiles(shaderBasePath, "vertexShader.glsl", "fragmentShader.glsl") != 0)
	{
		cout << "Failed to build the pipeline program." << endl;
		throw 1;
	}
	cout << "Successfully built the pipeline program." << endl;

	// Bind the pipeline program that we just created. 
	// The purpose of binding a pipeline program is to activate the shaders that it contains, i.e.,
	// any object rendered from that point on, will use those shaders.
	// When the application starts, no pipeline program is bound, which means that rendering is not set up.
	// So, at some point (such as below), we need to bind a pipeline program.
	// From that point on, exactly one pipeline program is bound at any moment of time.
	pipelineProgram.Bind();


	int resolution = heightmapImage->getWidth();
	numVerticesPoint = resolution * resolution;
	numVerticesLine = (resolution - 1) * (resolution - 1) * 4 + 2 * (resolution - 1) + 2 * (resolution - 1);
	numVerticesTriangle = (resolution - 1) * (resolution - 1) * 6;

	// Vertex colors.
	// For point mode we only need to iterate through all the points and record it in a unique pointer
	// And for colors is strongly related to the positions of points.
	std::unique_ptr<float[]> positions = std::make_unique<float[]>(numVerticesPoint * 3);
	std::unique_ptr<float[]> colors = std::make_unique<float[]>(numVerticesPoint * 4);

	// Support color (ImageIO::getBytesPerPixel == 3) in input images.
	// for extra credit: Render wireframe on top of solid triangles (use glPolygonOffset to avoid z-buffer fighting).
	//

	for (int i = 0; i < resolution; ++i) {
		for (int j = 0; j < resolution; ++j) {
			// normalize it first.
			float height = heightmapImage->getPixel(i, j, 0) / 255.0f;

			// refer to the helper instruc
			int index = (i * resolution + j) * 3;
			positions[index + 0] = 2.5 * i / float(resolution - 1);   // x
			positions[index + 1] = height;                      // height (y)
			positions[index + 2] = -2.5 * j / float(resolution - 1);  // z

			// use height to set color
			// for white just to set every coordinate the same
			int colorIndex = (i * resolution + j) * 4;
			float normalizedHeight = glm::clamp(1.5f * height, 0.0f, 1.0f);
			colors[colorIndex + 0] = normalizedHeight; // R
			colors[colorIndex + 1] = normalizedHeight; // G
			colors[colorIndex + 2] = normalizedHeight; // B
			colors[colorIndex + 3] = 1.0f;   // alpha
		}
	}

	vboVertices.Gen(numVerticesPoint, 3, positions.get(), GL_STATIC_DRAW); // 3 values per position
	vboColors.Gen(numVerticesPoint, 4, colors.get(), GL_STATIC_DRAW); // 4 values per color
	vaoPoint.Gen();
	vaoPoint.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboVertices, "position");
	vaoPoint.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboColors, "color");

	//
	// things remain to do: add vao for corresponding mode: line and triangle.
	// just like what we did in previous section
	// for line and triangle, we need to 'connect' the points using the positions we recorded in previous section
	// for unique_ptr of line position, just like matrix, in fact it is a flat pointer. Hence we just record the
	// drawing sequence in the linePos or trianglePos, i.e. record 01/12/23 ... by iteration of two for loop.
	// 
	// extra credit: use gldrawelement-> store corresponding index.
	std::unique_ptr<float[]> linePositions = std::make_unique<float[]>(numVerticesLine * 3);
	std::unique_ptr<float[]> lineColors = std::make_unique<float[]>(numVerticesLine * 4);

	int lineIdx = 0;
	for (int i = 0; i < resolution - 1; i++) {
		for (int j = 0; j < resolution - 1; j++) {

			int currentIndex = i * resolution + j;
			float height = heightmapImage->getPixel(i, j, 0) / 255.0f;
			float normalizedHeight = glm::clamp(1.5f * height, 0.0f, 1.0f);
			// remember row unchanged, col change is horizontal
			// col unchanged, row change is vertical
			//horizontal line from (i, j) to (i, j + 1)
			linePositions[lineIdx * 3 + 0] = positions[currentIndex * 3 + 0];
			linePositions[lineIdx * 3 + 1] = positions[currentIndex * 3 + 1];
			linePositions[lineIdx * 3 + 2] = positions[currentIndex * 3 + 2];

			linePositions[(lineIdx + 1) * 3 + 0] = positions[(currentIndex + 1) * 3 + 0];
			linePositions[(lineIdx + 1) * 3 + 1] = positions[(currentIndex + 1) * 3 + 1];
			linePositions[(lineIdx + 1) * 3 + 2] = positions[(currentIndex + 1) * 3 + 2];

			lineIdx += 2;

			//vertical line from (i, j) to (i + 1, j)
			linePositions[lineIdx * 3 + 0] = positions[currentIndex * 3 + 0];
			linePositions[lineIdx * 3 + 1] = positions[currentIndex * 3 + 1];
			linePositions[lineIdx * 3 + 2] = positions[currentIndex * 3 + 2];

			linePositions[(lineIdx + 1) * 3 + 0] = positions[(currentIndex + resolution) * 3 + 0];
			linePositions[(lineIdx + 1) * 3 + 1] = positions[(currentIndex + resolution) * 3 + 1];
			linePositions[(lineIdx + 1) * 3 + 2] = positions[(currentIndex + resolution) * 3 + 2];

			lineIdx += 2;

			// colors -> lines created for the current position
			for (int k = 0; k < 4; ++k)
			{
				int colorIndex = (lineIdx - 4 + k) * 4;
				lineColors[colorIndex + 0] = normalizedHeight; // R
				lineColors[colorIndex + 1] = normalizedHeight; // G
				lineColors[colorIndex + 2] = normalizedHeight; // B
				lineColors[colorIndex + 3] = 1.0f; // alpha
			}
		}
	}

	// below is for the complementary element for above for loop generating wireframe
	// To generate extra two lines, we get the final place of horizontal pos and vertical pos.
	// Then to draw the line we iterate through each pixels in the final col and row
	for (int j = 0; j < resolution - 1; ++j) {
		int currentIndex = (resolution - 1) * resolution + j;

		// start point and end point connect together
		// below is for horizontal line
		linePositions[lineIdx * 3 + 0] = positions[currentIndex * 3 + 0];
		linePositions[lineIdx * 3 + 1] = positions[currentIndex * 3 + 1];
		linePositions[lineIdx * 3 + 2] = positions[currentIndex * 3 + 2];

		linePositions[(lineIdx + 1) * 3 + 0] = positions[(currentIndex + 1) * 3 + 0];
		linePositions[(lineIdx + 1) * 3 + 1] = positions[(currentIndex + 1) * 3 + 1];
		linePositions[(lineIdx + 1) * 3 + 2] = positions[(currentIndex + 1) * 3 + 2];

		lineIdx += 2;

		// set line colors for both points
		for (int k = 0; k < 2; ++k) {
			int colorIndex = (lineIdx - 2 + k) * 4;
			lineColors[colorIndex + 0] = 1.5f * heightmapImage->getPixel(resolution - 1, j, 0) / 255.0f; // R
			lineColors[colorIndex + 1] = 1.5f * heightmapImage->getPixel(resolution - 1, j, 0) / 255.0f; // G
			lineColors[colorIndex + 2] = 1.5f * heightmapImage->getPixel(resolution - 1, j, 0) / 255.0f; // B
			lineColors[colorIndex + 3] = 1.0f; // alpha
		}
	}

	for (int i = 0; i < resolution - 1; ++i) {
		int currentIndex = i * resolution + (resolution - 1);

		// start point and end point connect together
		// below is for vertical line.
		linePositions[lineIdx * 3 + 0] = positions[currentIndex * 3 + 0];
		linePositions[lineIdx * 3 + 1] = positions[currentIndex * 3 + 1];
		linePositions[lineIdx * 3 + 2] = positions[currentIndex * 3 + 2];

		linePositions[(lineIdx + 1) * 3 + 0] = positions[(currentIndex + resolution) * 3 + 0];
		linePositions[(lineIdx + 1) * 3 + 1] = positions[(currentIndex + resolution) * 3 + 1];
		linePositions[(lineIdx + 1) * 3 + 2] = positions[(currentIndex + resolution) * 3 + 2];

		lineIdx += 2;

		// set line colors for both points
		for (int k = 0; k < 2; ++k) {
			int colorIndex = (lineIdx - 2 + k) * 4;
			lineColors[colorIndex + 0] = 1.5f * heightmapImage->getPixel(i, resolution - 1, 0) / 255.0f; // R
			lineColors[colorIndex + 1] = 1.5f * heightmapImage->getPixel(i, resolution - 1, 0) / 255.0f; // G
			lineColors[colorIndex + 2] = 1.5f * heightmapImage->getPixel(i, resolution - 1, 0) / 255.0f; // B
			lineColors[colorIndex + 3] = 1.0f; // alpha
		}
	}


	vboLines.Gen(numVerticesLine, 3, linePositions.get(), GL_STATIC_DRAW);
	vboColorLines.Gen(numVerticesLine, 4, lineColors.get(), GL_STATIC_DRAW);
	vaoLine.Gen();
	vaoLine.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboLines, "position");
	vaoLine.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboColorLines, "color");

	// to store triangle positions and colors
	// triangle is similar to line mode. To record the draw sequence. just record the points in the original position
	// then we let it draw in displayFunc.
	std::unique_ptr<float[]> trianglePositions = std::make_unique<float[]>(numVerticesTriangle * 3);
	std::unique_ptr<float[]> triangleColors = std::make_unique<float[]>(numVerticesTriangle * 4);

	int triIdx = 0;
	for (int i = 0; i < resolution - 1; ++i) {
		for (int j = 0; j < resolution - 1; ++j) {

			int currentIndex = i * resolution + j;
			float height = heightmapImage->getPixel(i, j, 0) / 255.0f;
			float normalizedHeight = glm::clamp(1.5f * height, 0.0f, 1.0f);
			// define two triangles to form a corresponding vbo
			// v1-v0 x v2-v0 , orientation of triangle is wrong.
			// use cross product to check the orientation
			// Triangle 1: (i, j), (i, j+1), (i+1, j)
			trianglePositions[triIdx * 3 + 0] = positions[currentIndex * 3 + 0];
			trianglePositions[triIdx * 3 + 1] = positions[currentIndex * 3 + 1];
			trianglePositions[triIdx * 3 + 2] = positions[currentIndex * 3 + 2];

			trianglePositions[(triIdx + 1) * 3 + 0] = positions[(currentIndex + 1) * 3 + 0];
			trianglePositions[(triIdx + 1) * 3 + 1] = positions[(currentIndex + 1) * 3 + 1];
			trianglePositions[(triIdx + 1) * 3 + 2] = positions[(currentIndex + 1) * 3 + 2];

			trianglePositions[(triIdx + 2) * 3 + 0] = positions[(currentIndex + resolution) * 3 + 0];
			trianglePositions[(triIdx + 2) * 3 + 1] = positions[(currentIndex + resolution) * 3 + 1];
			trianglePositions[(triIdx + 2) * 3 + 2] = positions[(currentIndex + resolution) * 3 + 2];

			triIdx += 3;

			// triangle 2 connect -> (i, j+1), (i+1, j+1), (i+1, j)
			trianglePositions[triIdx * 3 + 0] = positions[(currentIndex + 1) * 3 + 0];
			trianglePositions[triIdx * 3 + 1] = positions[(currentIndex + 1) * 3 + 1];
			trianglePositions[triIdx * 3 + 2] = positions[(currentIndex + 1) * 3 + 2];

			trianglePositions[(triIdx + 1) * 3 + 0] = positions[(currentIndex + resolution + 1) * 3 + 0];
			trianglePositions[(triIdx + 1) * 3 + 1] = positions[(currentIndex + resolution + 1) * 3 + 1];
			trianglePositions[(triIdx + 1) * 3 + 2] = positions[(currentIndex + resolution + 1) * 3 + 2];

			trianglePositions[(triIdx + 2) * 3 + 0] = positions[(currentIndex + resolution) * 3 + 0];
			trianglePositions[(triIdx + 2) * 3 + 1] = positions[(currentIndex + resolution) * 3 + 1];
			trianglePositions[(triIdx + 2) * 3 + 2] = positions[(currentIndex + resolution) * 3 + 2];

			triIdx += 3;

			for (int k = 0; k < 6; ++k) {
				triangleColors[(triIdx - 6 + k) * 4 + 0] = normalizedHeight; // R
				triangleColors[(triIdx - 6 + k) * 4 + 1] = normalizedHeight; // G
				triangleColors[(triIdx - 6 + k) * 4 + 2] = normalizedHeight; // B
				triangleColors[(triIdx - 6 + k) * 4 + 3] = 1.0f; // alpha
			}
		}
	}
	// To be checked. Those out of bound points should be handled in CPU.

	vboTriangles.Gen(numVerticesTriangle, 3, trianglePositions.get(), GL_STATIC_DRAW);
	vboColorTriangles.Gen(numVerticesTriangle, 4, triangleColors.get(), GL_STATIC_DRAW);
	vaoTriangle.Gen();
	vaoTriangle.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTriangles, "position");
	vaoTriangle.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboColorTriangles, "color");


	// For the smooth mode, I choose to not using 4 vbos to do the smooth job. Because I can not really
	// figure out what is going out there. Hence I choose to use only 1 vbo and update all the position information
	// inside the cpu.
	std::unique_ptr<float[]> positionSmoothUp = std::make_unique<float[]>(numVerticesPoint * 3);
	std::unique_ptr<float[]> positionSmoothDown = std::make_unique<float[]>(numVerticesPoint * 3);
	std::unique_ptr<float[]> positionSmoothLeft = std::make_unique<float[]>(numVerticesPoint * 3);
	std::unique_ptr<float[]> positionSmoothRight = std::make_unique<float[]>(numVerticesPoint * 3);
	std::unique_ptr<float[]> positionSmoothTriangle = std::make_unique<float[]>(numVerticesPoint * 3);

	std::unique_ptr<float[]> smoothTriangle = std::make_unique<float[]>(numVerticesTriangle * 3);
	std::unique_ptr<float[]> smoothTriangleColor = std::make_unique<float[]>(numVerticesTriangle * 4);

	for (int i = 0; i < resolution; ++i) {
		for (int j = 0; j < resolution; ++j) {

			int index = (i * resolution + j) * 3;
			// neighboring indices with boundary checking
			int upIndex = (i - 1 >= 0) ? ((i - 1) * resolution + j) : (i * resolution + j); // Up
			int downIndex = (i + 1 < resolution) ? ((i + 1) * resolution + j) : (i * resolution + j); // Down
			int leftIndex = (j - 1 >= 0) ? (i * resolution + (j - 1)) : (i * resolution + j); // Left
			int rightIndex = (j + 1 < resolution) ? (i * resolution + (j + 1)) : (i * resolution + j); // Right


			float p_center_x = positions[index];
			float p_center_y = positions[index + 1];
			float p_center_z = positions[index + 2];

			float p_up_x = positions[upIndex * 3];          // x above
			float p_up_y = positions[upIndex * 3 + 1];      // y above
			float p_up_z = positions[upIndex * 3 + 2];      // z above

			float p_down_x = positions[downIndex * 3];      // x below
			float p_down_y = positions[downIndex * 3 + 1];  // y below
			float p_down_z = positions[downIndex * 3 + 2];  // z below

			float p_left_x = positions[leftIndex * 3];        // x left
			float p_left_y = positions[leftIndex * 3 + 1];    // y left
			float p_left_z = positions[leftIndex * 3 + 2];    // z left

			float p_right_x = positions[rightIndex * 3];      // x right
			float p_right_y = positions[rightIndex * 3 + 1];  // y right
			float p_right_z = positions[rightIndex * 3 + 2];  // z right

			positionSmoothTriangle[index] = (p_center_x + p_left_x + p_right_x + p_down_x + p_up_x) / 5.0f; // x
			positionSmoothTriangle[index + 1] = (p_center_y + p_left_y + p_right_y + p_down_y + p_up_y) / 5.0f; // y
			positionSmoothTriangle[index + 2] = (p_center_z + p_left_z + p_right_z + p_down_z + p_up_z) / 5.0f; // z
		}
	}

	int triIdxSmooth = 0;
	for (int i = 0; i < resolution - 1; ++i) {
		for (int j = 0; j < resolution - 1; ++j) {

			int currentIndexSmooth = i * resolution + j;
			float height = heightmapImage->getPixel(i, j, 0) / 255.0f;
			float normalizedHeight = glm::clamp(1.5f * height, 0.0f, 1.0f);
			//
			// define two triangles to form a corresponding vbo
			// v1-v0 x v2-v0 , orientation of triangle is wrong.
			// use cross product to check the orientation
			// Triangle 1: (i, j), (i, j+1), (i+1, j)
			smoothTriangle[triIdxSmooth * 3 + 0] = positionSmoothTriangle[currentIndexSmooth * 3 + 0];
			smoothTriangle[triIdxSmooth * 3 + 1] = positionSmoothTriangle[currentIndexSmooth * 3 + 1];
			smoothTriangle[triIdxSmooth * 3 + 2] = positionSmoothTriangle[currentIndexSmooth * 3 + 2];

			smoothTriangle[(triIdxSmooth + 1) * 3 + 0] = positionSmoothTriangle[(currentIndexSmooth + 1) * 3 + 0];
			smoothTriangle[(triIdxSmooth + 1) * 3 + 1] = positionSmoothTriangle[(currentIndexSmooth + 1) * 3 + 1];
			smoothTriangle[(triIdxSmooth + 1) * 3 + 2] = positionSmoothTriangle[(currentIndexSmooth + 1) * 3 + 2];

			smoothTriangle[(triIdxSmooth + 2) * 3 + 0] = positionSmoothTriangle[(currentIndexSmooth + resolution) * 3 + 0];
			smoothTriangle[(triIdxSmooth + 2) * 3 + 1] = positionSmoothTriangle[(currentIndexSmooth + resolution) * 3 + 1];
			smoothTriangle[(triIdxSmooth + 2) * 3 + 2] = positionSmoothTriangle[(currentIndexSmooth + resolution) * 3 + 2];

			triIdxSmooth += 3;

			// triangle 2 connect -> (i, j+1), (i+1, j+1), (i+1, j)
			smoothTriangle[triIdxSmooth * 3 + 0] = positionSmoothTriangle[(currentIndexSmooth + 1) * 3 + 0];
			smoothTriangle[triIdxSmooth * 3 + 1] = positionSmoothTriangle[(currentIndexSmooth + 1) * 3 + 1];
			smoothTriangle[triIdxSmooth * 3 + 2] = positionSmoothTriangle[(currentIndexSmooth + 1) * 3 + 2];

			smoothTriangle[(triIdxSmooth + 1) * 3 + 0] = positionSmoothTriangle[(currentIndexSmooth + resolution + 1) * 3 + 0];
			smoothTriangle[(triIdxSmooth + 1) * 3 + 1] = positionSmoothTriangle[(currentIndexSmooth + resolution + 1) * 3 + 1];
			smoothTriangle[(triIdxSmooth + 1) * 3 + 2] = positionSmoothTriangle[(currentIndexSmooth + resolution + 1) * 3 + 2];

			smoothTriangle[(triIdxSmooth + 2) * 3 + 0] = positionSmoothTriangle[(currentIndexSmooth + resolution) * 3 + 0];
			smoothTriangle[(triIdxSmooth + 2) * 3 + 1] = positionSmoothTriangle[(currentIndexSmooth + resolution) * 3 + 1];
			smoothTriangle[(triIdxSmooth + 2) * 3 + 2] = positionSmoothTriangle[(currentIndexSmooth + resolution) * 3 + 2];

			triIdxSmooth += 3;

			for (int k = 0; k < 6; ++k) {
				smoothTriangleColor[(triIdxSmooth - 6 + k) * 4 + 0] = normalizedHeight; // R
				smoothTriangleColor[(triIdxSmooth - 6 + k) * 4 + 1] = normalizedHeight; // G
				smoothTriangleColor[(triIdxSmooth - 6 + k) * 4 + 2] = normalizedHeight; // B
				smoothTriangleColor[(triIdxSmooth - 6 + k) * 4 + 3] = 1.0f; // alpha
			}
		}
	}

	//VBO and VAO setting for smooth mode:
	vboTriangleSmooth.Gen(numVerticesTriangle, 3, smoothTriangle.get(), GL_STATIC_DRAW);
	vboTriangleSmoothColor.Gen(numVerticesTriangle, 4, smoothTriangleColor.get(), GL_STATIC_DRAW);

	vaoTriangleSmooth.Gen();
	vaoTriangleSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTriangleSmooth, "positionSmooth");
	vaoTriangleSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTriangleSmoothColor, "color");

	//-------------------------------------------------------------------------------------------------------------------
	//
	//
	//
	//
	//
	//
	//
	// for wireframe on top;
	vboTriangleLine.Gen(numVerticesLine, 3, linePositions.get(), GL_STATIC_DRAW);
	vboTriangleLineColor.Gen(numVerticesLine, 4, lineColors.get(), GL_STATIC_DRAW);
	vaoTriangleWire.Gen();
	vaoTriangleWire.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTriangleLine, "position");
	vaoTriangleWire.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTriangleLineColor, "color");

	vboTriangleTop.Gen(numVerticesTriangle, 3, trianglePositions.get(), GL_STATIC_DRAW);
	vboTriangleTopColor.Gen(numVerticesTriangle, 4, triangleColors.get(), GL_STATIC_DRAW);
	vaoTriangleBot.Gen();
	vaoTriangleBot.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTriangleTop, "position");
	vaoTriangleBot.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTriangleTopColor, "color");
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}

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

