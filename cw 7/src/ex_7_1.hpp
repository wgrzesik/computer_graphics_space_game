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
#include <cmath>
#include "SOIL/SOIL.h"
#include <chrono>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
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

	GLuint metalnessSphere;
	GLuint roughnessSphere;
	GLuint metalnessShip;
	GLuint roughnessShip;
}


GLuint program;
GLuint programSun;
GLuint programTex;
GLuint programEarth;
GLuint programProcTex;
GLuint programSkyBox;
GLuint programSign;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext cubeContext;
Core::RenderContext starContext;
Core::RenderContext saberContext;
Core::RenderContext heartContext;
Core::RenderContext rocketContext;

glm::vec3 cameraPos = glm::vec3(-3.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(-3.f, 0, 0);
glm::vec3 spaceshipDir = glm::vec3(1.f, 0.f, 0.f);

bool fire = false;
float a = 3;
std::random_device rd;
std::mt19937 gen(rd());
//std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
std::uniform_real_distribution<float> radiusDistribution(-6.f, 6.f);
std::uniform_real_distribution<float> planetoidsYDistribution(-5.f, 5.f);
std::uniform_real_distribution<float> planetoidsXDistribution(-2.f, 11.f);
std::uniform_real_distribution<float> planetoidsScaleDistribution(0.2f, 0.3f);
std::vector<glm::vec4> shearVectors;
std::vector<glm::vec3> scaleVectors;
std::vector<glm::vec3> ammunitionPositions;
std::vector<glm::mat4> ammunitionModelMatrices;

const int arraySize = 200;
const int planetoidParams = 5;
std::vector<std::vector<float>> planetoidsVector(arraySize, std::vector<float>(planetoidParams));

float tempPlanArray[200][2];

glm::vec3 asteroid_Calc = spaceshipDir * glm::vec3(a, a, a);
glm::vec3 asteroid_Pos = spaceshipPos + glm::vec3(0, a, 0) + asteroid_Calc;
glm::vec3 distance = asteroid_Pos - spaceshipPos;

double step = 0.0000001;

GLuint VAO,VBO;
float lastFiretime = 0;
float ammoReloadTime = 3.0f;  // Czas ładowania amunicji w sekundach
float ammoReloadProgress = 0.0f;

float aspectRatio = 1.f;
glm::vec3 ammoPos;
unsigned int textureID;

float tiltAngleSide;
float tiltAngleUpDown;

int colission = 3;
int star = 0;
int star_counter = 0;
float starMetalness = 0.8;
float starRoughness = 0.1;
glm::vec3 lightPos = glm::vec3(-8, 4, 2);
glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8) * 100;

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.5, 0.9, 0.8) * 10;
float exposition = 1.f;
float spotlightPhi = 3.14 / 3;


double easeInExpo(double x) {
	return pow(2, 10 * x - 10);
}


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
	float f = 60.;
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

void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float metalness, float roughness, glm::vec3 cameraPos) {
	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(program, "lightPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	glUniform1f(glGetUniformLocation(program, "metalness"), metalness);
	glUniform1f(glGetUniformLocation(program, "roughness"), roughness);
	glUniform3f(glGetUniformLocation(program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	Core::DrawContext(context);

}
void drawObjectSigns(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float metalness, float roughness, glm::vec3 lightstarPos) {
	glUseProgram(programSign);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programSign, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programSign, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programSign, "lightPos"), lightstarPos.x, lightstarPos.y, lightstarPos.z);

	glUniform3f(glGetUniformLocation(programSign, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(programSign, "lightColor"), lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(glGetUniformLocation(programSign, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glGetUniformLocation(programSign, "metalness"), metalness);
	glUniform1f(glGetUniformLocation(programSign, "roughness"), roughness);


	// Przesyłanie informacji o widoku (view) do shadera
	Core::DrawContext(context);
}

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapId, GLuint metalnessTexture, GLuint roughnessTexture) {
	glUseProgram(programTex);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programTex, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programTex, "lightPos"), 0, 0, 0);

	Core::SetActiveTexture(normalMapId, "normalSampler", programTex, 1);
	Core::SetActiveTexture(textureID, "colorTexture", programTex, 0);

	glUniform1i(glGetUniformLocation(programTex, "metalnessTexture"), 2);
	glUniform1i(glGetUniformLocation(programTex, "roughnessTexture"), 3);

	// Bind metalness and roughness textures to texture units
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, metalnessTexture);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, roughnessTexture);

	glUniform3f(glGetUniformLocation(programTex, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	Core::DrawContext(context);

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


void drawObjectSun(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID)
{
	glUseProgram(programSun);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), 1.f);
	Core::SetActiveTexture(textureID, "sunTexture", programSun, 0);
	glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, textureID);
	Core::DrawContext(sphereContext);
}


//float speed = 0.03;
float speed = 0.02;
float starind = 50;



bool checkCollision(glm::vec3 objectPos, float objectRadius, glm::vec3 spaceshipPos, float spaceshipRadius, bool isStar) {
	float distance = glm::length(objectPos - spaceshipPos);
	float starExtraRadius = isStar ? 0.03f : 0.0f;
	return distance < objectRadius + spaceshipRadius + starExtraRadius;
}

void generatePlanetoidBelt() {



	for (int i = 0; i < 200; ++i) {
		float z = tempPlanArray[i][0];
		float y = tempPlanArray[i][1];
		glm::vec3 Scale = scaleVectors[i];
		bool collision = false;
		planetoidsVector[i][3] -= speed;
		float x = planetoidsVector[i][3];

		if (planetoidsVector[i][3] < -3.5f) {
			//planetoidsArray[i][0] += spaceshipPos.z - planetoidsArray[i][0];
			//planetoidsArray[i][1] += spaceshipPos.y - planetoidsArray[i][1];
		
			tempPlanArray[i][0] = spaceshipPos.z + planetoidsVector[i][0];
			tempPlanArray[i][1] = spaceshipPos.y + planetoidsVector[i][1];
			planetoidsVector[i][3] = 11.f;
			 z = tempPlanArray[i][0];
			 y = tempPlanArray[i][1];
			 x = planetoidsVector[i][3];
			 planetoidsVector[i][4] = 0;
		}
		

		if (planetoidsVector[i][4] == 1) {
			// Planeta ju� uczestniczy�a w kolizji, przejd� do kolejnej iteracji
			continue;
		}



		for (int j = 0; j < i; ++j) {
			float prevZ = tempPlanArray[j][0];
			float prevY = tempPlanArray[j][1];
			glm::vec3 prevScale = scaleVectors[j];
			float prevX = planetoidsVector[j][3];
			float dx = x - prevX;
			float dy = y - prevY;
			float dz = z - prevZ;
			float distanceSquared = dx * dx + dy * dy + dz * dz;

			float sumRadiiSquared = glm::length2(Scale + prevScale);


			if (distanceSquared < sumRadiiSquared) {
				collision = true;
				break;
			}
		}
		if (!collision) {
			if (fmod(i, starind) == 0) {
				if (checkCollision(glm::vec3(x, y, z), 0.1f, spaceshipPos, 0.08f, true)) {
					// Kolizja z gwiazd�
					//std::cout << "Collision with star " << i << std::endl;
					planetoidsVector[i][4] = 1;
					star_counter++;
					if (star_counter == 3) {
						exit(0);
					}

				}
			}
			else if (checkCollision(glm::vec3(x, y, z), 0.1f, spaceshipPos, 0.08f, true))
			{
				//kolizja z asteroida
				std::cout << "Collision with asteroid " << i << std::endl;
				colission--;
				planetoidsVector[i][4] = 1;
				if (colission == 0)
				{
					exit(0);
				}
			}
			else
			{
				for (int n = 0; n < ammunitionPositions.size(); ++n)
				{
					if (checkCollision(glm::vec3(x, y, z), 0.1f, ammunitionPositions[n], 0.08f, true))
					{
						// Asteroida zestrzelona
						std::cout << "Collision with ammo " << i << std::endl;
						planetoidsVector[i][4] = 1;
					}
				}
			}
		}
				if (!collision) {
					if (fmod(i, starind) == 0) {
						float time = glfwGetTime();
						glm::mat4 modelMatrix = glm::translate(glm::vec3(x, y, z)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 1.0f));
						drawObjectColor(starContext, modelMatrix * glm::eulerAngleX(time) * glm::scale(glm::vec3(0.03f)), glm::vec3(0.1, 1.f, 1.0), starMetalness, starRoughness, glm::vec3(x-1, y, z));
						if (star == 0)
						{
							star++;
							std::cout << "Star " << star << std::endl;

						}

					}
					else {
						float rotationAngle = glm::radians(70*y);
						glm::mat4 transformationMatrix = glm::translate(glm::vec3(x, y, z)) *
							glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(1.0f, 0.0f, 1.0f)) *
							glm::scale(glm::vec3(Scale));
						drawObjectTexture(sphereContext, transformationMatrix,
							texture::moon, texture::moonNormal, texture::metalnessSphere, texture::roughnessSphere);
					}
				}

			}
}
	


	
glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
	spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
	spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
	-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
	0.,0.,0.,1.,
	});

void drawStars(int star_number) {
	float yOffset = 0.55f + spaceshipPos.y;
	float zOffset =5.10f + spaceshipPos.z;
	float scaleFactor = 0.03f;

	for (int i = 0; i < star_number; ++i) {
		drawObjectSigns(starContext, glm::translate(glm::vec3(2.3f, yOffset, (zOffset) - i * 0.5f ))
			* glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::scale(glm::vec3(scaleFactor)), glm::vec3(0.1, 1.f, 1.0), starMetalness, starRoughness,glm::vec3(1.0f, yOffset, zOffset));
	}
}

void drawHearts(int collision_number) {
	float yOffset = -0.5f + spaceshipPos.y;
	float zOffset = 6.15 + spaceshipPos.z;


	for (int i = 0; i < 5; ++i) {
		if (collision_number > i) {
			drawObjectSigns(heartContext, glm::translate(glm::vec3(3.5f, yOffset, (zOffset) - i * 0.5f))
				* glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::scale(glm::vec3(0.025f)), glm::vec3(1, 0, 0), starMetalness, starRoughness, glm::vec3(1.0f, yOffset, zOffset));
		}
	}
}
void updateAmmunition() {
	if (fire) {
		glm::vec3 newAmmoPos = spaceshipPos + glm::vec3(0.025f, 0.f, 0.f);
		ammunitionPositions.push_back(newAmmoPos);
		fire = false;
	}

}

void renderAmmunition() {

	for (int i = ammunitionPositions.size() - 1; i >= 0; --i) {
		ammunitionPositions[i] = ammunitionPositions[i] + glm::vec3(0.025f, 0.f, 0.f);
		if (ammunitionPositions[i].x > 3.f) {
			ammunitionPositions.erase(ammunitionPositions.begin() + i);
		}
		else {
			glm::mat4 modelMatrix = glm::translate(ammunitionPositions[i]) * glm::rotate(glm::mat4(1.0f), glm::radians(90.f), glm::vec3(1.0f, 0.0f, 0.f));
			drawObjectColor(rocketContext, glm::translate(ammunitionPositions[i]) * glm::rotate(glm::mat4(1.0f), glm::radians(270.f), glm::vec3(0.0f, 1.0f, 0.f)) * glm::scale(glm::vec3(0.003f)), glm::vec3(0.1, 0.1, 0.7), starMetalness, starRoughness, spaceshipPos);
		}
	}
}

void renderScene(GLFWwindow* window)
{
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();

	drawObjectSkyBox(cubeContext, glm::translate(cameraPos));
	glDepthMask(GL_FALSE);
	drawObjectSun(sphereContext, glm::translate(spaceshipPos + glm::vec3(16.0f, -4.0f, -13.0f)) * glm::scale(glm::mat4(), glm::vec3(2)), texture::sun);

	drawObjectSun(sphereContext, glm::translate(spaceshipPos + glm::vec3(12.0f, -4.2f, -8.0f)) * glm::scale(glm::vec3(0.3f)), texture::earth);
	//drawObjectTexture(sphereContext,
	//	glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(8.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)), texture::moon, texture::moonNormal, texture::metalnessSphere, texture::roughnessSphere);

	drawObjectSun(sphereContext,  glm::translate(spaceshipPos + glm::vec3(14.f, -4.5, -9.f)) * glm::scale(glm::vec3(0.05f)), texture::mercury);
	//drawObjectSun(sphereContext,  glm::translate(spaceshipPos + glm::vec3(9.f, -5.f, -7.0f)) * glm::scale(glm::vec3(0.1f)), texture::mars);
	drawObjectSun(sphereContext,  glm::translate(spaceshipPos + glm::vec3(12.f, -2.7f, -5.5f)) * glm::scale(glm::vec3(0.2f)), texture::venus);
	drawObjectSun(sphereContext,  glm::translate(spaceshipPos + glm::vec3(4.f, -2.f, 3.0f)) * glm::scale(glm::vec3(0.57f)), texture::jupiter);
	drawObjectSun(sphereContext,  glm::translate(spaceshipPos + glm::vec3(15.f, -3.f, 6.0f)) * glm::scale(glm::vec3(0.8f)), texture::saturn);
	//drawObjectSun(sphereContext,  glm::translate(spaceshipPos + glm::vec3(13.f, -8.0f, 8.0f)) * glm::scale(glm::vec3(0.6f)), texture::uranus);
	drawObjectSun(sphereContext, glm::translate(spaceshipPos + glm::vec3(13.f, 0.f, 7.0f))  * glm::scale(glm::vec3(0.35f)), texture::neptune);
	glDepthMask(GL_TRUE);

	spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));

	specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});


	drawObjectTexture(shipContext,
		glm::translate(spaceshipPos) *
		specshipCameraRotrationMatrix *
		glm::eulerAngleY(glm::pi<float>()) *
		glm::rotate(glm::mat4(), tiltAngleSide * glm::radians(30.0f), glm::vec3(0, 0, 1)) *
		glm::rotate(glm::mat4(), tiltAngleUpDown * glm::radians(20.0f), glm::vec3(1, 0, 0)) *
		glm::scale(glm::vec3(0.025f)),
		texture::ship, texture::shipNormal, texture::metalnessShip, texture::roughnessShip);
	//drawObjectTexture(shipContext,
	//	glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>())*glm::scale(glm::vec3(0.1f)),
	//	texture::ship, texture::shipNormal
	//);
	updateAmmunition();
	renderAmmunition();
	generatePlanetoidBelt();
	//if (fire == true){
	//	ammoPos = ammoPos + glm::vec3(0.025f, 0.f, 0.f);
	//	glm::mat4 modelMatrix = glm::translate(ammoPos) * glm::rotate(glm::mat4(1.0f), glm::radians(90.f), glm::vec3(1.0f, 0.0f, 0.f));
	//	drawObjectColor(rocketContext, modelMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(270.f), glm::vec3(0.0f, 1.0f, 0.f)) * glm::scale(glm::vec3(0.003f)), glm::vec3(0.1, 0.1, 0.7), starMetalness, starRoughness, spaceshipPos);
	//	}

	glDisable(GL_DEPTH_TEST);
	drawStars(star_counter);
	drawHearts(colission);	
	glEnable(GL_DEPTH_TEST);

	glUseProgram(0);
	glfwSwapBuffers(window);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	glViewport(0, 0, width, height);
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
	programSun = shaderLoader.CreateProgram("shaders/shader_5_sun.vert", "shaders/shader_5_sun.frag");
	programSign = shaderLoader.CreateProgram("shaders/signs.vert", "shaders/signs.frag");
	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship_new.obj", shipContext);
	loadModelToContext("./models/cube.obj", cubeContext);
	loadModelToContext("./models/estrellica.obj", starContext);
	loadModelToContext("./models/CraneoOBJ.obj", saberContext);
	loadModelToContext("./models/heart.obj", heartContext);
	loadModelToContext("./models/rocket.obj", rocketContext);

	texture::earth = Core::LoadTexture("textures/earth.png");
	texture::ship = Core::LoadTexture("textures/ship/spaceship_color.jpg");
	texture::moon = Core::LoadTexture("textures/moon.jpg");
	texture::rust = Core::LoadTexture("textures/rust.jpg");

	texture::mars = Core::LoadTexture("textures/2k_mars.jpg");
	texture::mercury = Core::LoadTexture("textures/2k_mercury.jpg");
	texture::venus = Core::LoadTexture("textures/2k_venus_surface.jpg");
	texture::jupiter = Core::LoadTexture("textures/2k_jupiter.jpg");
	texture::saturn = Core::LoadTexture("textures/8k_saturn.jpg");
	texture::uranus = Core::LoadTexture("textures/2k_uranus.jpg");
	texture::neptune = Core::LoadTexture("textures/2k_neptune.jpg");
	texture::sun = Core::LoadTexture("textures/2k_sun.jpg");

	texture::earthNormal = Core::LoadTexture("textures/earth_normalmap.png");
	texture::shipNormal = Core::LoadTexture("textures/ship/spaceship_nmap.jpg");
	texture::rustNormal = Core::LoadTexture("textures/rust_normal.jpg");
	texture::moonNormal = Core::LoadTexture("textures/moon_normal.jpg");

	texture::metalnessSphere = Core::LoadTexture("textures/rusty_metal_sheet_diff_2k.jpg");
	texture::roughnessSphere = Core::LoadTexture("textures/rough_concrete_diff_1k.jpg");

	texture::metalnessShip = Core::LoadTexture("textures/ship/spaceship_metalness.jpg");
	texture::roughnessShip = Core::LoadTexture("textures/ship/spaceship_rough.jpg");


	for (int i = 0; i < 200; ++i) {
		float z = radiusDistribution(gen);
		float x = planetoidsXDistribution(gen);
		float y = planetoidsYDistribution(gen);
		planetoidsVector[i][0] = z;
		planetoidsVector[i][1] = y;
		planetoidsVector[i][3] = x;
		glm::vec3 scaleVector = glm::vec3(planetoidsScaleDistribution(gen), planetoidsScaleDistribution(gen), planetoidsScaleDistribution(gen));
		scaleVectors.push_back(scaleVector);

	}
	const int arraySize = 200;
	const int planetoidParams = 5;
	std::sort(planetoidsVector.begin(), planetoidsVector.end(),
		[](const auto& a, const auto& b) {
			return a[3] < b[3];
		});

	for (int i = 0; i < arraySize; ++i) {
		tempPlanArray[i][0] = planetoidsVector[i][0];
		tempPlanArray[i][1] = planetoidsVector[i][1];
	}
	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
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



void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		fire = true;
		ammoPos = spaceshipPos;
		lastFiretime = glfwGetTime();

	}
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.005f;
	float moveSpeed = 0.01f;

	double x = 0.01;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		spaceshipPos += glm::vec3(0.f,moveSpeed,0.f);
		tiltAngleUpDown -= easeInExpo(x);
	}
	else {
		if (tiltAngleUpDown < 0) {
			tiltAngleUpDown += 0.003;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		spaceshipPos -= glm::vec3(0.f, moveSpeed, 0.f);
		tiltAngleUpDown += easeInExpo(x);
	}
	else {
		if (tiltAngleUpDown > 0) {
			tiltAngleUpDown -= 0.003;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		spaceshipPos -= glm::vec3(0.f, 0.f, moveSpeed);
		tiltAngleSide -= easeInExpo(x);
	} else {
		if (tiltAngleSide < 0) {
			tiltAngleSide += 0.003;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		spaceshipPos += glm::vec3(0.f, 0.f, moveSpeed);
		tiltAngleSide += easeInExpo(-x);
	} else {
		if (tiltAngleSide > 0) {
			tiltAngleSide -= 0.003;
		}
	}
	

	glfwSetMouseButtonCallback(window, mouse_button_callback);

	cameraPos = spaceshipPos - 0.5 * spaceshipDir+ glm::vec3(0.f,0.3,0.f);
	cameraDir = spaceshipDir;

	tiltAngleSide = fmaxf(-1, fminf(1, tiltAngleSide));
	tiltAngleUpDown = fmaxf(-1, fminf(1, tiltAngleUpDown));


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
