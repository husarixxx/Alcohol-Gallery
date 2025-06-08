#include "tiny_obj_loader.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>      
#include <string>      
#include <glm/glm.hpp> 
#include <GL/glew.h>

struct Model {
	GLuint vao;
	GLuint vbo[4];
	size_t vertexCount;
};

struct ModelInstance {
	Model* model;
	glm::vec3 position;
	glm::vec3 scale;
	std::vector<GLuint>textures;
	float turn = 0.0f;
	float floatAmplitude = 0.0f;
	float floatSpeed = 0.0f;
	float floatPhase = 0.0f;

	glm::vec3 basePosition;
	bool isBottle = false;
	glm::vec3 originalPosition;
	float drinkProgress = 0.0f;

	int textIndex = -1;
	bool isActive = true;
	bool shouldFloat = false;
};

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
		fprintf(stderr, "Nie mo¿na wczytaæ modelu: %s\n", filename);
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
