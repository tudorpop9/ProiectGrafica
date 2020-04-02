//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"
void sceneObjs(gps::Shader shader);
void skyBoxStuff();


int glWindowWidth = 1280;
int glWindowHeight = 720;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
float mouseSesitivity = 0.05f;
float yaw = -90.0f, pitch; // for camera
float waterAngle = 0.0f;

int diffuseUniLoc;
int ambientalLoc;
int specularLoc;
int isTreeLoc;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::vec3 pointLight;
GLuint pointLightLoc;
glm::vec3 pointLightColor = glm::vec3(1.0f, 1.0f, 1.0f) ;
GLuint pointLightColorLoc;

gps::Camera myCamera(glm::vec3(6.0f, 2.36f, 31.26f), glm::vec3(0.0f, 0.0f, 0.0));
GLfloat cameraSpeed = 0.08f;

bool pressedKeys[1024];

// last frame's mouse position
double lastMouseX = glWindowWidth / 2; // init in the center of the scene
double lastMouseY = glWindowHeight / 2;
bool initMouse = true; // just once
GLfloat angle;
GLfloat angleInc = 2.0f;
GLfloat lightAngle;

gps::Model3D myModel;
gps::Model3D ground;
gps::Model3D river;
gps::Model3D lightCube;
gps::Model3D spruce;

gps::Model3D bonfire;
gps::Model3D fireLogs;
gps::Model3D logs;
gps::Model3D tower;
gps::Model3D bridge;
gps::Model3D wc;
gps::Model3D cabin;

gps::Model3D cate;
gps::Model3D dog;
gps::Model3D dogTail;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

float skyboxRotationAngle = 0.0f;
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;
bool isDay = true;



GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	lightShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (initMouse) {
		lastMouseX = xpos;
		lastMouseY = ypos;
		initMouse = false;
	}


	float  xCamOffset = xpos - lastMouseX;
	float yCamOffset = lastMouseY - ypos; // revesed since y-coors range from bottom to top
	lastMouseX = xpos;					  // gotta try the other way around tho.
	lastMouseY = ypos;

	xCamOffset *= mouseSesitivity;
	yCamOffset *= mouseSesitivity;

	yaw += xCamOffset;
	pitch += yCamOffset;

	if (pitch > 89.0f) // no wierd camera stuff (reverse)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

}

void processRotation() {
	myCamera.rotate(pitch, yaw);
}

void tailMovement() {
	if (angle == 20.0f) {
		angleInc *= (-1.0f);
	}
	else if (angle == 340.0f) {
		angleInc *= (-1.0f);
	}
	else if (angle < 0.0f) {
		angle = 360.0f;
	}
	else if (angle > 360.0f) {
		angle = 0.0f;
	}
	angle += angleInc;
	glm::mat4 trs = glm::translate(glm::mat4(1.0f), glm::vec3(11.0f, 2.0f, 31.0f));
	trs = trs * glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	trs = trs * glm::translate(glm::mat4(1.0f), glm::vec3(-11.0f, -2.0f, -31.0f));
	model = trs;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}

void processMovement()
{

	if (pressedKeys[GLFW_KEY_UP]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_LEFT]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}


	/*if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f; 
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}*/

	if (pressedKeys[GLFW_KEY_L]) {
		myCustomShader.useShaderProgram();
		lightColor = glm::vec3(1.0f, 1.0f, 0.85f); //white light
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
		glUniform1f(diffuseUniLoc, 0.8f);
		glUniform1f(ambientalLoc, 0.2f);
		isDay = true;
		skyBoxStuff(); //update skybox
	}
	if (pressedKeys[GLFW_KEY_O]) {
		myCustomShader.useShaderProgram();
		lightColor = glm::vec3(0.2f, 0.2f, 0.2f); //white light
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
		glUniform1f(diffuseUniLoc, 0.0f);
		glUniform1f(ambientalLoc, 0.1f);
		isDay = false;
		skyBoxStuff(); //update skybox
	}
}



bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//hides the cursor, and tracks the position
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//
	glfwSetCursorPosCallback(glWindow, mouseCallback);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(myCamera.getCameraTarget() + 1.0f * lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{
	myModel = gps::Model3D("objects/nanosuit/nanosuit.obj", "objects/nanosuit/");
	//ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	river = gps::Model3D("objects/river/river.obj", "objects/river/");
	ground = gps::Model3D("objects/groundPlane/groundPlane.obj", "objects/groundPlane/");
	lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");
	spruce = gps::Model3D("objects/testObj/forest.obj", "objects/testObj/");

	bonfire = gps::Model3D("objects/bonfire/bonfire.obj", "objects/bonfire/");
	fireLogs = gps::Model3D("objects/fireLogs/fireLogs.obj", "objects/fireLogs/");
	logs = gps::Model3D("objects/logs/logs.obj", "objects/logs/");
	tower = gps::Model3D("objects/tower/tower.obj", "objects/tower/");
	bridge = gps::Model3D("objects/bridge/bridge.obj", "objects/bridge/");
	wc = gps::Model3D("objects/wc/wc.obj", "objects/wc/");
    cabin = gps::Model3D("objects/cabin/cabin.obj", "objects/cabin/");

	cate = gps::Model3D("objects/cate/cate.obj", "objects/cate/");
	dog = gps::Model3D("objects/dog/dog.obj", "objects/dog/");
	dogTail = gps::Model3D("objects/dog/dogTail.obj", "objects/dog/");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	isTreeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "isTree");

	diffuseUniLoc = glGetUniformLocation(myCustomShader.shaderProgram, "diffuseStregth");
	ambientalLoc = glGetUniformLocation(myCustomShader.shaderProgram, "ambientStrength");
	specularLoc = glGetUniformLocation(myCustomShader.shaderProgram, "specularStrength");

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	pointLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLight");
	pointLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 2.0f, 3.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 0.7f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//Day/Night
	glUniform1f(diffuseUniLoc, 0.9f);
	glUniform1f(ambientalLoc, 0.2f);
	glUniform1f(specularLoc, 0.5f);
	glUniform1i(isTreeLoc, 0);

	//bonfire
	pointLight =glm::vec3(0.225f, 1.0f, 33.44f);
	glUniform3fv(pointLightLoc, 1, glm::value_ptr(pointLight));
	pointLightColor = glm::vec3(1.0f, 0.88f, 0.0f);
	glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processRotation();
	processMovement();	

	/*lightDir = (glm::vec3(0.0f, 2.0f, 3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0)); //* (glm::vec3(0.0f, 2.0f, 3.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
	*/
	//render the scene to the depth buffer (first pass)
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1,	GL_FALSE,glm::value_ptr(computeLightSpaceTrMatrix()));	
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	//create model matrix for nanosuit
	//model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-32.0f, 5.7f, -23.58f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),1,GL_FALSE,glm::value_ptr(model));
	myModel.Draw(depthMapShader);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	sceneObjs(depthMapShader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (second pass)
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),1,GL_FALSE,glm::value_ptr(computeLightSpaceTrMatrix()));

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),1,GL_FALSE,glm::value_ptr(view));	
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-32.0f, 5.7f, -23.58f));
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	//model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	myModel.Draw(myCustomShader);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	sceneObjs(myCustomShader);

	//Skyboxtuff
	skyboxRotationAngle += 0.00625f;
	if (skyboxRotationAngle < 0) {
		skyboxRotationAngle = 360;
	}
	else if (skyboxRotationAngle > 360) {
		skyboxRotationAngle = 0;
	}
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	view *= glm::rotate(glm::mat4(1.0f), glm::radians(skyboxRotationAngle), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	mySkyBox.Draw(skyboxShader, view, projection);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void sceneObjs(gps::Shader shader) {

	glUniform1f(specularLoc, 0.0f);
	ground.Draw(shader);
	glUniform1f(specularLoc, 0.5f);

	glUniform1f(specularLoc, 1.0f);
	river.Draw(shader);
	glUniform1f(specularLoc, 0.5f);
	
	//turn off specular
	glUniform1f(specularLoc, 0.0f);

	glUniform1i(isTreeLoc, 1); //clip
	spruce.Draw(shader);
	glUniform1i(isTreeLoc, 0);//unclip

	
	bonfire.Draw(shader);
	fireLogs.Draw(shader);
	logs.Draw(shader);
	tower.Draw(shader);
	bridge.Draw(shader);
	wc.Draw(shader);
	cabin.Draw(shader);
	cate.Draw(shader);
	dog.Draw(shader);

	tailMovement();

	
	dogTail.Draw(shader);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//turn specular back on
	glUniform1f(specularLoc, 0.5f);
}

void skyBoxStuff() {

	if (isDay) {
		faces.clear();
		faces.push_back("objects/skybox/day/graycloud_rt.jpg");
		faces.push_back("objects/skybox/day/graycloud_lf.jpg");
		faces.push_back("objects/skybox/day/graycloud_up.jpg");
		faces.push_back("objects/skybox/day/graycloud_dn.jpg");
		faces.push_back("objects/skybox/day/graycloud_bk.jpg");
		faces.push_back("objects/skybox/day/graycloud_ft.jpg");
	}
	else {
		std::cout << "hello? \n";
		faces.clear();
		faces.push_back("objects/skybox/night/right.tga");
		faces.push_back("objects/skybox/night/left.tga");
		faces.push_back("objects/skybox/night/top.tga");
		faces.push_back("objects/skybox/night/bottom.tga");
		faces.push_back("objects/skybox/night/back.tga");
		faces.push_back("objects/skybox/night/front.tga");
	}
	mySkyBox.Load(faces);
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();	
	skyBoxStuff();
	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
