#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <future>
#include <chrono>
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"
#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "SOIL/SOIL.h"

namespace texture {
	GLuint earth;
	GLuint clouds;
	GLuint moon;
	GLuint ship;
	GLuint rust;

	GLuint sun;

	GLuint mars;
	GLuint mercury;
	GLuint venus;
	GLuint jupiter;
	GLuint saturn;
	GLuint uranus;
	GLuint neptune;

	GLuint grid;

	GLuint earthNormal;
	GLuint moonNormal;
	GLuint shipNormal;
	GLuint rustNormal;
}


GLuint program;
GLuint programSun;
GLuint programTex;
GLuint programEarth;
GLuint programProcTex;
GLuint programSkyBox;
GLuint programBloom;
GLuint programQuad;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext cubeContext;
Core::RenderContext test;

glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(-4.f, 0, 0);
glm::vec3 spaceshipDir = glm::vec3(1.f, 0.f, 0.f);

bool START_AS = false;
float a = 3 ;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
std::uniform_real_distribution<float> planetoidsYDistribution(-1.f, 1.f);
std::uniform_real_distribution<float> planetoidsScaleDistribution(0.01f, 0.05f);
float planetoidsArray[500][4];
glm::vec3 asteroid_Calc = spaceshipDir * glm::vec3(a, a, a);
glm::vec3 asteroid_Pos = spaceshipPos + glm::vec3(0, a, 0) + asteroid_Calc;
glm::vec3 distance = asteroid_Pos - spaceshipPos;

double step = 0.0000001;

GLuint VAO,VBO;
float lastAsteroidTime = 0;
float aspectRatio = 1.f;

unsigned int textureID;


unsigned int hdrFBO;
GLuint testMap;

unsigned int colorBuffers[2];

int WIDTH = 800;
int HEIGHT = 800;

unsigned int rectVAO, rectVBO;




glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir,glm::vec3(0.f,1.f,0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide,cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{
	
	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 20.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f+n) / (n - f),2*f * n / (n - f),
		0.,0.,-1.,0.,
		});

	
	perspectiveMatrix=glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {

	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightPos"), 0, 0, 0);
	Core::DrawContext(context);

}

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapId) {
	//glEnable(GL_DEPTH_TEST);
	glUseProgram(programTex);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programTex, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programTex, "lightPos"), 0, 0, 0);
	Core::SetActiveTexture(normalMapId, "normalSampler", programTex, 1);
	Core::SetActiveTexture(textureID, "colorTexture", programTex, 0);

	glUseProgram(programBloom);
	glUniform1i(glGetUniformLocation(programBloom, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(programBloom, "bloomTexture"), 1);
	glUniform1f(glGetUniformLocation(programBloom, "gamma"), 2.2f);

	glEnable(GL_DEPTH_TEST);

	Core::DrawContext(context);

	//Core::SetActiveTexture(colorBuffers[0], "colorTexture", programQuad, 0);

}
void drawObjectSkyBox(Core::RenderContext& context, glm::mat4 modelMatrix) {

	glDisable(GL_DEPTH_TEST);
	glUseProgram(programSkyBox);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSkyBox, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programSkyBox, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programSkyBox, "lightPos"), 0, 0, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	Core::DrawContext(context);
	glEnable(GL_DEPTH_TEST);

}


//void drawObjectBloom(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapId) {
//
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glUseProgram(programTex);
//	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
//	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
//	glUniformMatrix4fv(glGetUniformLocation(programTex, "transformation"), 1, GL_FALSE, (float*)&transformation);
//	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
//	glUniform3f(glGetUniformLocation(programTex, "lightPos"), 0, 0, 0);
//	Core::SetActiveTexture(normalMapId, "normalSampler", programTex, 2);
//	Core::SetActiveTexture(textureID, "colorTexture", programTex, 1);
//	Core::DrawContext(context);
//
//	glUseProgram(programBloom);
//	glUniform1i(glGetUniformLocation(programBloom, "screenTexture"), 0);
//	glUniform1i(glGetUniformLocation(programBloom, "bloomTexture"), 1);
//	glUniform1f(glGetUniformLocation(programBloom, "gamma"), 2.2f);
//
//	//Core::DrawContext(context);
//
//
//}

void generateAsteroids(glm::vec3 asteroid_Pos, glm::vec3 distance, double step) {
	glm::vec3 normalizedDir = glm::normalize(distance);
	asteroid_Pos = asteroid_Pos - normalizedDir *step;
	drawObjectTexture(sphereContext, glm::translate(asteroid_Pos) * glm::scale(glm::vec3(0.1f)), texture::moon, texture::moonNormal);
			
}

void generatePlanetoidBelt() {

	for (int i = 0; i < 500; ++i) {
		
		float x = planetoidsArray[i][0];
		float z = planetoidsArray[i][1];
		float y = planetoidsArray[i][2];
		float pScale = planetoidsArray[i][3];

		float time = glfwGetTime();

		drawObjectTexture(sphereContext,glm::eulerAngleY(time / 5) * glm::translate(glm::vec3(x, y, z)) * glm::scale(glm::vec3(pScale)), texture::moon, texture::moonNormal);
	}
}

int steps = 0;
glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
	spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
	spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
	-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
	0.,0.,0.,1.,
	});

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


void renderScene(GLFWwindow* window)
{
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClearColor(0.0f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 transformation;
	float time = glfwGetTime();

	drawObjectSkyBox(cubeContext, glm::translate(cameraPos));

	//drawObjectTexture(sphereContext, glm::scale(glm::mat4(), glm::vec3(2.0f, 2.0f, 2.0f)), texture::rust, texture::rustNormal);
	drawObjectTexture(sphereContext, glm::scale(glm::mat4(), glm::vec3(2.0f, 2.0f, 2.0f)), texture::sun, texture::rustNormal);


	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(8.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.3f)), texture::earth, texture::earthNormal);
	drawObjectTexture(sphereContext,
		glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(8.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)), texture::moon, texture::moonNormal);

	drawObjectTexture(sphereContext, glm::eulerAngleY(time) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.15f)), texture::mercury, texture::rustNormal);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 4) * glm::translate(glm::vec3(10.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.2f)), texture::mars, texture::rustNormal);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 2) * glm::translate(glm::vec3(6.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.3f)), texture::venus, texture::rustNormal);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 5) * glm::translate(glm::vec3(14.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.9f)), texture::jupiter, texture::rustNormal);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 6) * glm::translate(glm::vec3(17.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.9f)), texture::saturn, texture::rustNormal);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 7) * glm::translate(glm::vec3(20.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.6f)), texture::uranus, texture::rustNormal);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 8) * glm::translate(glm::vec3(23.f, 0, 0)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.6f)), texture::neptune, texture::rustNormal);

	
	spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	drawObjectTexture(shipContext,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>())*glm::scale(glm::vec3(0.1f)),
		texture::ship, texture::shipNormal
	);

	generatePlanetoidBelt();

	if (time - lastAsteroidTime < 5 && START_AS) {	//funkcja generujaca pierwsza asteroide 
		step = step + 0.03;
		generateAsteroids(asteroid_Pos, distance, step);
		//steps++;
		//std::cout << steps << std::endl;
	}
	else {
		float random_x = distribution(gen);
		float random_y = distribution(gen);
		float random_z = distribution(gen);
		//std::cout << random_x << std::endl;
		glm::vec3 random_dir = glm::normalize(glm::vec3(spaceshipDir.x + random_x, spaceshipDir.y +  random_y, spaceshipDir.z +  random_z));

		asteroid_Calc = a * random_dir;
		glm::vec3 estimated_Spaceship_Pos = spaceshipPos + glm::vec3(1.f, 1.f, 1.f) * glm::normalize(spaceshipDir);
		asteroid_Pos = spaceshipPos + glm::vec3(0, a / 5, 0) + asteroid_Calc;
		
		distance = asteroid_Pos - estimated_Spaceship_Pos;
		lastAsteroidTime = time;
		step = 0.0001;
		//std::cout << cameraDir.x << cameraDir.y << cameraDir.z << std::endl;
		//steps = 0;
	}
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programQuad);
	//Core::SetActiveTexture(colorBuffers[0], "colorTexture", programQuad, 0);
	//Core::SetActiveTexture(colorBuffers[1], "highlightTexture", programQuad, 1);
	glUniform1i(glGetUniformLocation(programQuad, "originalTexture"), 0);
	glUniform1i(glGetUniformLocation(programQuad, "bloomTexture"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);

	renderQuad();
	//Core::DrawContext(test);

	//glBindTexture(GL_TEXTURE_2D, 0);
	//glUseProgram(0);

	glfwSwapBuffers(window);


	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//// Draw the framebuffer rectangle
	//glUseProgram(programQuad);
	////glBindVertexArray(rectVAO);
	////Core::DrawContext(test);
	//renderQuad();
	//glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
	////glActiveTexture(GL_TEXTURE0);
	////glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
	////glActiveTexture(GL_TEXTURE1);
	////glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
	////glDrawArrays(GL_TRIANGLES, 0, 6);
	//Core::SetActiveTexture(colorBuffers[0], "colorTexture", programQuad, 0);
	//Core::SetActiveTexture(colorBuffers[1], "highlightTexture", programQuad, 1);

	//glUseProgram(0);

	//glfwSwapBuffers(window);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	//glViewport(0, 0, width, height);
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	WIDTH = 800;
	HEIGHT = 800;

}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");

	programTex = shaderLoader.CreateProgram("shaders/shader_5_tex.vert", "shaders/shader_5_tex.frag");
	programEarth = shaderLoader.CreateProgram("shaders/shader_5_tex.vert", "shaders/shader_5_tex.frag");
	programProcTex = shaderLoader.CreateProgram("shaders/shader_5_tex.vert", "shaders/shader_5_tex.frag");

	programSkyBox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");

	programQuad = shaderLoader.CreateProgram("shaders/shader_quad.vert", "shaders/shader_quad.frag");

	programBloom = shaderLoader.CreateProgram("shaders/shader_bloom.vert", "shaders/shader_bloom.frag");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.obj", shipContext);
	loadModelToContext("./models/cube.obj", cubeContext);

	texture::earth = Core::LoadTexture("textures/earth.png");
	texture::ship = Core::LoadTexture("textures/spaceship.jpg");
	texture::moon = Core::LoadTexture("textures/moon.jpg");
	texture::rust = Core::LoadTexture("textures/rust.jpg");

	texture::sun = Core::LoadTexture("textures/2k_sun.jpg");

	texture::mars = Core::LoadTexture("textures/2k_mars.jpg");
	texture::mercury = Core::LoadTexture("textures/2k_mercury.jpg");
	texture::venus = Core::LoadTexture("textures/2k_venus_surface.jpg");
	texture::jupiter = Core::LoadTexture("textures/2k_jupiter.jpg");
	texture::saturn = Core::LoadTexture("textures/8k_saturn.jpg");
	texture::uranus = Core::LoadTexture("textures/2k_uranus.jpg");
	texture::neptune = Core::LoadTexture("textures/2k_neptune.jpg");

	texture::earthNormal = Core::LoadTexture("textures/earth_normalmap.png");
	texture::shipNormal = Core::LoadTexture("textures/spaceship_normal.jpg");
	texture::rustNormal = Core::LoadTexture("textures/rust_normal.jpg");
	texture::moonNormal = Core::LoadTexture("textures/moon_normal.jpg");

	loadModelToContext("./models/test.obj", test);

	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);



	//const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F,WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
		);
	}

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	float rectangleVertices[] =
	{
		//  Coords   // texCoords
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,

		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};
	
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));




	std::uniform_real_distribution<float> angleDistribution(0.0f, 2.0f * glm::pi<float>());
	std::uniform_real_distribution<float> radiusDistribution(10.f, 17.f);

	for (int i = 0; i < 500; ++i) {
		float angle = angleDistribution(gen);
		float radius = radiusDistribution(gen);
		float y = planetoidsYDistribution(gen);
		float pScale = planetoidsScaleDistribution(gen);
		planetoidsArray[i][0] = radius * std::cos(angle);
		planetoidsArray[i][1] = radius * std::sin(angle);
		planetoidsArray[i][2] = y;
		planetoidsArray[i][3] = pScale;
	}

	std::vector<std::string> filepaths = {
		"textures/skybox/space_rt.png",
		"textures/skybox/space_lf.png",
		"textures/skybox/space_up.png",
		"textures/skybox/space_dn.png",
		"textures/skybox/space_bk.png",
		"textures/skybox/space_ft.png",
	};

	int w, h;
	unsigned char* image;
	for (unsigned int i = 0; i < 6; i++)
	{
		image = SOIL_load_image(filepaths[i].c_str(), &w, &h, 0, SOIL_LOAD_RGBA);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

double lastX = 0.0;
double lastY = 0.0;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	
	double deltaX = xpos - lastX;
	double deltaY = ypos - lastY;
	lastX = xpos;
	lastY = ypos;
	//std::cout << lastY << "   " << ypos << std::endl;
	const float sensitivity = 0.001f;
	deltaX *= sensitivity;
	deltaY *= sensitivity;
	glm::mat4 rotationMatrix = glm::eulerAngleY(-deltaX);
	spaceshipDir = glm::normalize(rotationMatrix * glm::vec4(spaceshipDir,0));
	spaceshipDir = spaceshipDir + glm::vec3(0, - deltaY / 4, 0);
	/*specshipCameraRotrationMatrix = glm::rotate(specshipCameraRotrationMatrix, static_cast<float>(deltaX), glm::vec3(0.f, 1.f, 0.f));
	specshipCameraRotrationMatrix = glm::rotate(specshipCameraRotrationMatrix, static_cast<float>(deltaY), spaceshipSide);
	spaceshipDir = glm::normalize(glm::vec3(-specshipCameraRotrationMatrix[2]));
	spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));*/
}
//obsluga wejscia
void processInput(GLFWwindow* window)
{
	spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	spaceshipUp = glm::vec3(0.f, 1.f, 0.f);

	float angleSpeed = 0.01f;
	float moveSpeed = 0.01f;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		spaceshipPos += spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		spaceshipPos -= spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		START_AS = true;

	//glfwSetCursorPosCallback(window, mouseCallback);

	//cameraPos = spaceshipPos - 0.5 * spaceshipDir + glm::vec3(0, 2, 0) * 0.1f;
	//cameraDir = spaceshipDir;
	cameraPos = spaceshipPos - 0.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.2f;
	cameraDir = spaceshipDir;

	//cameraDir = glm::normalize(-cameraPos);

}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
