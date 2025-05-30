#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "lodepng.h"
#include "shaderprogram.h"
#include <iostream>

const float PI = 3.141592653589793f;
float aspectRatio = 1;
float yaw = 0.0f;
float moveSpeed = 1.5f;

struct Model {
	GLuint vao;
	GLuint vbo[4];
	size_t vertexCount;
};

struct ModelInstance {
	Model* model;
	glm::vec3 position;
	glm::vec3 scale;
	GLuint texture1 = 0;
	float turn = 0.0f;
};

std::vector<Model> models;
std::vector<ModelInstance> instances;

glm::vec3 eye = glm::vec3(-4.0f, 1.0f, 0.0f); 
glm::vec3 allLights[5];
float moveForward = 0.0f;  
float moveRight = 0.0f;    

ShaderProgram* sp;

GLuint tex1;
GLuint tex2;
GLuint tex3;
GLuint tex4;
GLuint tex5;
GLuint tex6;


Model loadModel(const char* filename) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	Model model = { 0 };
	glGenVertexArrays(1, &model.vao);
	glBindVertexArray(model.vao);

	glGenBuffers(4, model.vbo);

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {
		fprintf(stderr, "Nie można wczytać modelu: %s\n", filename);
		exit(1);
	}

	std::vector<float> vertices, normals, texCoords;

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
			vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
			vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);
			vertices.push_back(1.0f);

			if (!attrib.normals.empty() && index.normal_index >= 0) {
				normals.push_back(attrib.normals[3 * index.normal_index + 0]);
				normals.push_back(attrib.normals[3 * index.normal_index + 1]);
				normals.push_back(attrib.normals[3 * index.normal_index + 2]);
				normals.push_back(0.0f);
			}

			if (!attrib.texcoords.empty() && index.texcoord_index >= 0) {
				texCoords.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
				texCoords.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, model.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	model.vertexCount = vertices.size() / 4;

	glBindBuffer(GL_ARRAY_BUFFER, model.vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, model.vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	return model;
}

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W) moveForward = 1.0f;  
		if (key == GLFW_KEY_S) moveForward = -1.0f; 
		if (key == GLFW_KEY_A) moveRight = -1.0f;   
		if (key == GLFW_KEY_D) moveRight = 1.0f;    
		if (key == GLFW_KEY_LEFT) yaw += glm::radians(5.0f);
		if (key == GLFW_KEY_RIGHT) yaw -= glm::radians(5.0f);
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W || key == GLFW_KEY_S) moveForward = 0.0f;
		if (key == GLFW_KEY_A || key == GLFW_KEY_D) moveRight = 0.0f;
	}
}


void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}


GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	std::vector<unsigned char> image;  
	unsigned width, height; 
	unsigned error = lodepng::decode(image, width, height, filename);

	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

void initOpenGLProgram(GLFWwindow* window) {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	glm::vec3 staticLights[4] = {
		glm::vec3(-10.0f, 0.8f, 0.0f),
		glm::vec3(2.0f, 0.8f, 0.0f),
		glm::vec3(-4.0f, 0.8f, -6.0f),
		glm::vec3(-4.0f, 0.8f, 6.0f),
	};

	for (int i = 0; i < 4; ++i)
		allLights[i + 1] = staticLights[i];

	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

	tex1 = readTexture("obj//textures//test.png");
	tex2 = readTexture("obj//textures//walter.png");
	tex3 = readTexture("obj//textures//lamp.png");
	tex4 = readTexture("obj//textures//carpet.png");
	tex5 = readTexture("obj//textures//ceiling1.png");
	tex6 = readTexture("obj//textures//ceiling2.png");
	Model floorModel = loadModel("obj//models//floorfinalfinal.obj");
	Model pedestal = loadModel("obj//models//pedestal.obj");
	Model bottle1 = loadModel("obj//models//bottle1.obj");
	Model lamp = loadModel("obj//models//lamp.obj");
	Model wall = loadModel("obj//models//finalwalls.obj");
	Model carpet = loadModel("obj//models//carpet.obj");
	Model ceilingLamp = loadModel("obj//models//ceilinglamp.obj");
	models.push_back(floorModel);
	models.push_back(pedestal); 
	models.push_back(bottle1);
	models.push_back(lamp);
	models.push_back(wall);
	models.push_back(carpet);
	models.push_back(ceilingLamp);

	instances = {
		{&models[0], glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.01f), tex1, 0.0f}, 

		{&models[0], glm::vec3(0.0f, 2.6f, 0.0f), glm::vec3(0.01f), tex1, 0.0f},

		{&models[1], glm::vec3(-4.0f, 0.0f, 6.0f), glm::vec3(0.4f), tex1, 0.0f},
		{&models[1], glm::vec3(-4.0f, 0.0f, -6.0f), glm::vec3(0.4f), tex1, 0.0f},
		{&models[1], glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.4f), tex1, 0.0f},
		{&models[1], glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(0.4f), tex1, 0.0f},

		{&models[2], glm::vec3(-10.0f, 0.75f, 0.0f), glm::vec3(0.015f), tex2, 0.0f},
		{&models[2], glm::vec3(2.0f, 0.75f, 0.0f), glm::vec3(0.015f), tex2, 0.0f},
		{&models[2], glm::vec3(-4.0f, 0.75f, -6.0f), glm::vec3(0.015f), tex2, 0.0f},
		{&models[2], glm::vec3(-4.0f, 0.75f, 6.0f), glm::vec3(0.015f), tex2, 0.0f},

		{&models[3], glm::vec3(-4.0f, 0.0f, 6.5f), glm::vec3(0.2f), tex3, 0.0f},
		{&models[3], glm::vec3(2.5f, 0.0f, 0.0f), glm::vec3(0.2f), tex3, 90.0f},
		{&models[3], glm::vec3(-4.0f, 0.0f, -6.5f), glm::vec3(0.2f), tex3, 180.0f},
		{&models[3], glm::vec3(-10.5f, 0.0f, 0.0f), glm::vec3(0.2f), tex3, 270.0f},

		{&models[4], glm::vec3(0.0f, -0.1f, 0.0f), glm::vec3(0.01f), tex1, 0.0f},

		{&models[5], glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.025f), tex4, 0.0f},

		{&models[6], glm::vec3(-4.0f, 2.5f, 0.0f), glm::vec3(0.8f), tex3, 90.0f}
	};
}

void freeOpenGLProgram(GLFWwindow* window) {
	for (auto& model : models) {
		glDeleteVertexArrays(1, &model.vao);
		glDeleteBuffers(3, model.vbo);
	}
	delete sp;
	glDeleteTextures(1, &tex1);
}

void drawScene(GLFWwindow* window, glm::vec3 eye) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 direction = glm::normalize(glm::vec3(sin(yaw), 0.0f, cos(yaw)));
	glm::vec3 target = eye + glm::normalize(direction);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 V = glm::lookAt(eye, target, up);
	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.01f, 50.0f);

	allLights[0] = eye;

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

	glUniform3fv(sp->u("viewPos"), 1, glm::value_ptr(eye));
	glUniform3fv(sp->u("lightPos"), 5, glm::value_ptr(allLights[0]));
	
	for (const auto& instance : instances) {
		Model& model = *instance.model;

		glBindVertexArray(model.vao);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, instance.texture1);
		glUniform1i(sp->u("textureMap0"), 0);
		
		glEnableVertexAttribArray(sp->a("vertex"));
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo[0]);
		glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, 0);

		if (sp->a("normal") != -1) {
			glEnableVertexAttribArray(sp->a("normal"));
			glBindBuffer(GL_ARRAY_BUFFER, model.vbo[1]);
			glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, 0);
		}

		if (sp->a("texCoord0") != -1) {
			glEnableVertexAttribArray(sp->a("texCoord0"));
			glBindBuffer(GL_ARRAY_BUFFER, model.vbo[2]);
			glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, 0);
		}

		glm::mat4 M = glm::translate(glm::mat4(1.0f), instance.position);
		M = glm::rotate(M, glm::radians(instance.turn), glm::vec3(0.0f, 1.0f, 0.0f));
		M = glm::scale(M, instance.scale);
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

		glDrawArrays(GL_TRIANGLES, 0, model.vertexCount);

		glDisableVertexAttribArray(sp->a("vertex"));
		glDisableVertexAttribArray(sp->a("normal"));
		glDisableVertexAttribArray(sp->a("texCoord0"));
		glBindVertexArray(0);
	}

	glfwSwapBuffers(window);
}

int main(void)
{	GLFWwindow* window; 

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) { 
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(750, 750, "OpenGL", NULL, NULL);

	if (!window) 
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); 

	glfwSetTime(0); 
	double prevTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		double currTime = glfwGetTime();
		float deltaTime = currTime - prevTime;
		prevTime = currTime;

		glm::vec3 front = glm::normalize(glm::vec3(sin(yaw), 0.0f, cos(yaw)));
		glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

		eye += front * moveForward * moveSpeed * deltaTime;   
		eye += right * moveRight * moveSpeed * deltaTime;

		drawScene(window, eye);
		glfwPollEvents();
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); 
	glfwTerminate(); 
	exit(EXIT_SUCCESS);
}