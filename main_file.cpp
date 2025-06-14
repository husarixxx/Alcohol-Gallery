#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define TINYOBJLOADER_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "lodepng.h"
#include "shaderprogram.h"
#include "models.h"
#include <iostream>

const float PI = 3.141592653589793f;
float aspectRatio = 1;
float yaw = 0.0f;
float moveSpeed = 1.5f;
int drinkCounter = 0;

bool Drinking = false;
int curBottleIndex = -1;
float drinkingStartTime = 0.0f;
bool blackScreen = false;

std::vector<Model> models;
std::vector<ModelInstance> instances;

glm::vec3 eye = glm::vec3(-4.0f, 1.0f, 0.0f);
glm::vec3 allLights[6];
float moveForward = 0.0f;  
float moveRight = 0.0f;    
float rotationDir = 0.0f;

ShaderProgram* sp;

GLuint tex1, tex2, tex3, tex4, tex5, tex6, tex7, tex8, tex9, tex10, tex11, tex12, tex13, tex14, tex15, tex16, tex17, tex18;

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

bool isInside(glm::vec3 pos) {
	bool inside = (pos.x <= 0 && pos.x >= -8 && pos.z >= -4 && pos.z <= 4);
	return inside;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && !Drinking || action == GLFW_REPEAT && !Drinking) {
		if (key == GLFW_KEY_W) moveForward = 1.0f;
		if (key == GLFW_KEY_S) moveForward = -1.0f;
		if (key == GLFW_KEY_A) moveRight = -1.0f;
		if (key == GLFW_KEY_D) moveRight = 1.0f;

		if (key == GLFW_KEY_LEFT) rotationDir = 1.0f;
		if (key == GLFW_KEY_RIGHT) rotationDir = -1.0f;
	}
	if (action == GLFW_PRESS && !Drinking && drinkCounter >= 3 || action == GLFW_REPEAT && !Drinking && drinkCounter >= 3) {
		if (key == GLFW_KEY_W) moveForward = -1.0f;
		if (key == GLFW_KEY_S) moveForward = 1.0f;
		if (key == GLFW_KEY_A) moveRight = 1.0f;
		if (key == GLFW_KEY_D) moveRight = -1.0f;

		if (key == GLFW_KEY_LEFT) rotationDir = -1.0f;
		if (key == GLFW_KEY_RIGHT) rotationDir = 1.0f;
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W || key == GLFW_KEY_S) moveForward = 0.0f;
		if (key == GLFW_KEY_A || key == GLFW_KEY_D) moveRight = 0.0f;

		if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) rotationDir = 0.0f;
	}

	if (key == GLFW_KEY_G && action == GLFW_PRESS && !Drinking) {
		float minDistance = 2.25f;
		int closetIndex = -1;

		for (int i = 0; i < instances.size(); i++) {
			if (instances[i].isBottle) {
				float distance = glm::distance(eye, instances[i].position);
				if (distance < minDistance) {
					closetIndex = i;
				}
			}
		}
		if (closetIndex != -1) {
			Drinking = true;
			curBottleIndex = closetIndex;
			drinkingStartTime = glfwGetTime();
			instances[closetIndex].originalPosition = instances[closetIndex].position;
			instances[closetIndex].drinkProgress = 0.0f;
		}
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

	glGenTextures(1, &tex); 
	glBindTexture(GL_TEXTURE_2D, tex); 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,	GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

void initOpenGLProgram(GLFWwindow* window) {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	glm::vec3 staticLights[5] = {
		glm::vec3(-10.0f, 1.5f, 0.0f),
		glm::vec3(2.0f, 1.5f, 0.0f),
		glm::vec3(-4.0f, 1.5f, -6.0f),
		glm::vec3(-4.0f, 1.5f, 6.0f),
		glm::vec3(-4.0f, 2.5f, 0.0f)
	};

	for (int i = 0; i < 5; ++i) {
		allLights[i + 1] = staticLights[i];
	}

	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

	tex1 = readTexture("obj//textures//stone_floor.png");
	tex2 = readTexture("obj//textures//walter.png");
	tex3 = readTexture("obj//textures//lamp1.png");
	tex4 = readTexture("obj//textures//carpet.png");
	tex5 = readTexture("obj//textures//ceiling1.png");
	tex6 = readTexture("obj//textures//ceiling2.png");
	tex7 = readTexture("obj//textures/wall_base.png");
	tex8 = readTexture("obj//textures//wall_roughness.png");
	tex9 = readTexture("obj//textures//wall_normals.png");
	tex10 = readTexture("obj//textures//stone.png");
	tex11 = readTexture("obj//textures//test1.png");
	tex12 = readTexture("obj//textures//bottle2.png");
	tex13 = readTexture("obj//textures//bottle3.png");
	tex14 = readTexture("obj//textures//bottle1.png");
	tex15 = readTexture("obj//textures//bottle4.png");
	tex16 = readTexture("obj//textures//text.png");
	tex17 = readTexture("obj//textures//lamp2.png");
	tex18 = readTexture("obj//textures//gold.png");
;	Model floorModel = loadModel("obj//models//floorfinalfinal.obj");
	Model pedestal = loadModel("obj//models//pedestal.obj");
	Model bottle1 = loadModel("obj//models//bottle1.obj");
	Model lamp = loadModel("obj//models//lamp.obj");
	Model wall = loadModel("obj//models//finalwalls.obj");
	Model carpet = loadModel("obj//models//carpet.obj");
	Model ceilingLamp = loadModel("obj//models//ceilinglamp.obj");
	Model bottle2 = loadModel("obj//models//bottle2.obj");
	Model bottle3 = loadModel("obj//models//bottle3.obj");
	Model bottle4 = loadModel("obj//models//bottle4.obj");
	Model text = loadModel("obj//models//wypij1.obj");
	Model vip = loadModel("obj//models//vip.obj");
	models.push_back(floorModel);
	models.push_back(pedestal); 
	models.push_back(bottle1);
	models.push_back(lamp);
	models.push_back(wall);
	models.push_back(carpet);
	models.push_back(ceilingLamp);
	models.push_back(bottle2);
	models.push_back(bottle3);
	models.push_back(bottle4);
	models.push_back(text);
	models.push_back(vip);

	instances = {
		{&models[0], glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.01f), {tex1}, 0.0f},

		{&models[0], glm::vec3(0.0f, 2.6f, 0.0f), glm::vec3(0.01f), {tex1}, 0.0f },

		{&models[1], glm::vec3(-4.0f, 0.0f, 6.0f), glm::vec3(0.4f), {tex10}, 0.0f},
		{&models[1], glm::vec3(-4.0f, 0.0f, -6.0f), glm::vec3(0.4f), {tex10}, 0.0f },
		{&models[1], glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.4f), {tex10}, 0.0f },
		{&models[1], glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(0.4f), {tex10}, 0.0f},

		{&models[2], glm::vec3(-10.0f, 0.75f, 0.0f), glm::vec3(0.015f), {tex15}, 0.0f,0.1f, 2.0f, 0.0f, glm::vec3(-10.0f, 0.75f, 0.0f),true, glm::vec3(-10.0f, 0.75f, 0.0f), 0.0f, 17 },
		{&models[9], glm::vec3(2.0f, 0.75f, 0.0f), glm::vec3(0.008f), {tex12}, 0.0f, 0.1f, 2.0f, 1.5f, glm::vec3(2.0f, 0.75f, 0.0f) ,true, glm::vec3(2.0f, 0.75f, 0.0f), 0.0f, 18 }, //test butelek
		{&models[7], glm::vec3(-4.0f, 0.75f, -6.0f), glm::vec3(0.015f), {tex13}, 0.0f,  0.1f, 2.0f, 3.0f, glm::vec3(-4.0f, 0.75f, -6.0f), true, glm::vec3(-4.0f, 0.75f, -6.0f), 0.0f, 19 },
		{&models[8], glm::vec3(-4.0f, 0.75f, 6.0f), glm::vec3(0.015f), {tex14}, 0.0f,   0.1f, 2.0f, 4.5f, glm::vec3(-4.0f, 0.75f, 6.0f), true, glm::vec3(-4.0f, 0.75f, 6.0f), 0.0f, 20 },

		{&models[3], glm::vec3(-4.0f, 0.0f, 6.5f), glm::vec3(0.2f), {tex3, tex17}, 0.0f },
		{&models[3], glm::vec3(2.5f, 0.0f, 0.0f), glm::vec3(0.2f), {tex3, tex17}, 90.0f },
		{&models[3], glm::vec3(-4.0f, 0.0f, -6.5f), glm::vec3(0.2f), {tex3, tex17}, 180.0f},
		{&models[3], glm::vec3(-10.5f, 0.0f, 0.0f), glm::vec3(0.2f), {tex3, tex17}, 270.0f },

		{&models[4], glm::vec3(0.0f, -0.1f, 0.0f), glm::vec3(0.01f), {tex11}, 0.0f},

		{&models[5], glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.025f), {tex4}, 0.0f, 0.1f, 1.0f, 0.0f, glm::vec3(-4.0f, 0.0f, 0.0f), false, glm::vec3(0), 0.0f, -1, true, true},

		{&models[6], glm::vec3(-4.0f, 2.5f, 0.0f), glm::vec3(0.8f), {tex3, tex17}, 90.0f },

		{&models[10], glm::vec3(-9.5f, 0.65f, 0.0f), glm::vec3(1.0f), {tex16}, 90.0f },
		{&models[10], glm::vec3(1.5f, 0.65f, 0.0f), glm::vec3(1.0f), {tex16}, 270.0f },
		{&models[10], glm::vec3(-4.0f, 0.65f, -5.5f), glm::vec3(1.0f), {tex16}, 0.0f },
		{&models[10], glm::vec3(-4.0f, 0.65f, 5.5f), glm::vec3(1.0f), {tex16}, 180.0f },

		{&models[11], glm::vec3(-9.5f, 0.0f, 0.0f), glm::vec3(0.0125f), {tex18}, 0.0f },
		{&models[11], glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0125f), {tex18}, 0.0f },
		{&models[11], glm::vec3(-4.0f, 0.0f, -4.0f), glm::vec3(0.0125f), {tex18}, 90.0f },
		{&models[11], glm::vec3(-4.0f, 0.0f, 5.5f), glm::vec3(0.0125f), {tex18}, 90.0f }
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
	if (blackScreen) {	
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;		
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glm::vec3 direction = glm::normalize(glm::vec3(sin(yaw), 0.0f, cos(yaw)));
	glm::vec3 target = eye + glm::normalize(direction);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 V = glm::lookAt(eye, target, up);
	glm::mat4 P = glm::perspective(60.0f * PI / 180.0f, aspectRatio, 0.01f, 60.0f);

	allLights[0] = eye;

	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float lightCutoff = glm::cos(glm::radians(25.0f));  
	float lightOuterCutoff = glm::cos(glm::radians(30.0f)); 
	sp->use();
	glUniform1f(sp->u("time"), glfwGetTime());
	glUniform1i(sp->u("drinkCounter"), drinkCounter);

	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

	glUniform3fv(sp->u("viewPos"), 1, glm::value_ptr(eye));
	glUniform3fv(sp->u("lightPos"), 6, glm::value_ptr(allLights[0]));
	glUniform4fv(sp->u("lightColor"), 1, glm::value_ptr(lightColor));

	glUniform3fv(sp->u("lightDir"), 1, glm::value_ptr(direction));
	glUniform1f(sp->u("lightCutoff"), lightCutoff);
	glUniform1f(sp->u("lightOuterCutoff"), lightOuterCutoff);

	
	for (const auto& instance : instances) {
		if (!instance.isActive) {
			continue;
		}

		Model& model = *instance.model;

		glBindVertexArray(model.vao);
		for (int i = 0; i < instance.textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, instance.textures[i]);
			std::string uniformName = "texUnits[" + std::to_string(i) + "]";
			glUniform1i(sp->u(uniformName.c_str()), i);
		}

		glUniform1i(sp->u("numTextures"), instance.textures.size());
		glEnableVertexAttribArray(sp->a("vertex"));
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo[0]);
		glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, 0);

		glEnableVertexAttribArray(sp->a("normal"));
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo[1]);
		glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, 0);

		glEnableVertexAttribArray(sp->a("texCoord0"));
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo[2]);
		glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, 0);
		
		glm::mat4 M = glm::translate(glm::mat4(1.0f), instance.position);

		if (instance.isBottle && Drinking && &instance == &instances[curBottleIndex]) {
			glm::vec3 toCamera = glm::normalize(eye - instance.position);

			glm::vec3 right = glm::normalize(glm::cross(toCamera, up));
			up = glm::normalize(glm::cross(right, toCamera));

			glm::mat4 rotation = glm::mat4(1.0f);
			rotation[0] = glm::vec4(right, 0.0f);
			rotation[1] = glm::vec4(up, 0.0f);
			rotation[2] = glm::vec4(-toCamera, 0.0f);

			M = M * rotation;						
			M = glm::rotate(M, glm::radians(instance.turn), glm::vec3(-2.0f, 0.0f, -1.0f));
			M = glm::rotate(M, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 			
		}
		else {
			M = glm::rotate(M, glm::radians(instance.turn), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		
		M = glm::scale(M, instance.scale);

		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

		glDrawArrays(GL_TRIANGLES, 0, model.vertexCount);

		glDisableVertexAttribArray(sp->a("vertex"));
		glDisableVertexAttribArray(sp->a("normal"));
		glDisableVertexAttribArray(sp->a("texCoord0"));
		glBindVertexArray(0);
	}
}

int main(void)
{	GLFWwindow* window; 

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) { 
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(750, 750, "OpenGL", NULL, NULL);

	if (!window) {
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); 

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); 

	glfwSetTime(0); 
	double prevTime = glfwGetTime();

	const float rotationSpeed = 90.0f;
	while (!glfwWindowShouldClose(window)) {
		
		double currTime = glfwGetTime();
		float deltaTime = currTime - prevTime;
		prevTime = currTime;

		yaw += rotationDir * glm::radians(rotationSpeed) * deltaTime;
		glm::vec3 front = glm::normalize(glm::vec3(sin(yaw), 0.0f, cos(yaw)));
		glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

		glm::vec3 newEye = eye;
		newEye += front * moveForward * moveSpeed * deltaTime;
		newEye += right * moveRight * moveSpeed * deltaTime;
		newEye.y = 1.0f; 
				
		if (isInside(newEye)) {
			eye = newEye;
		}

		for (auto& instance : instances) {			
			if (&instance == &instances[15]) {
				if (drinkCounter >= 3 && instance.floatAmplitude > 0.0f) {
					float time = glfwGetTime();
					instance.position.y = instance.basePosition.y + instance.floatAmplitude * sin(instance.floatSpeed * time + instance.floatPhase) + 0.1f;
				}
			}
			else if (instance.floatAmplitude > 0.0f) {
				float time = glfwGetTime();
				instance.position.y = instance.basePosition.y + instance.floatAmplitude * sin(instance.floatSpeed * time + instance.floatPhase) + 0.1f;
			}		
		}

		if (Drinking) {
			float elapsed = glfwGetTime() - drinkingStartTime;
			auto& bottle = instances[curBottleIndex];
			if (elapsed < 2.0f) {
				auto& bottle = instances[curBottleIndex];
				bottle.drinkProgress = elapsed / 2.0f;
				glm::vec3 toCamera = glm::normalize(eye - bottle.originalPosition);
				bottle.position = bottle.originalPosition + toCamera * bottle.drinkProgress * 1.5f;
				bottle.turn = bottle.drinkProgress * 90.0f;

			}
			else if (elapsed < 3.0f) { 
				bottle.position = bottle.originalPosition + glm::normalize(eye - bottle.originalPosition) * 1.5f;
				bottle.turn = 90.0f;
			}
			else if (elapsed < 4.5f) {
				if (!blackScreen) {
					blackScreen = true;
				}
			}  
			else {
				bottle.isActive = false;
				if (bottle.textIndex != -1) {
					instances[bottle.textIndex].isActive = false;
				}
				drinkCounter++;
				curBottleIndex = -1;
				Drinking = false;

				if (drinkCounter != 4) {
					blackScreen = false;
				}
			}
		}
		if (drinkCounter == 4) {
			exit(EXIT_SUCCESS);
		}
				
		drawScene(window, eye);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); 
	glfwTerminate(); 
	exit(EXIT_SUCCESS);
}