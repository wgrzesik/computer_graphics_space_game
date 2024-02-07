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
Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext cubeContext;
Core::RenderContext starContext;
Core::RenderContext saberContext;
Core::RenderContext heartContext;

glm::vec3 cameraPos = glm::vec3(-3.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(-3.f, 0, 0);
glm::vec3 spaceshipDir = glm::vec3(1.f, 0.f, 0.f);

bool fire = false;
float a = 3 ;

std::random_device rd;
std::mt19937 gen(rd());
//std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
std::uniform_real_distribution<float> radiusDistribution(-5.f, 5.f);
std::uniform_real_distribution<float> planetoidsYDistribution(-3.f, 3.f);
std::uniform_real_distribution<float> planetoidsXDistribution(-2.f, 10.f);
std::uniform_real_distribution<float> planetoidsScaleDistribution(0.1f, 0.2f);
float planetoidsArray[400][5];
glm::vec3 asteroid_Calc = spaceshipDir * glm::vec3(a, a, a);
glm::vec3 asteroid_Pos = spaceshipPos + glm::vec3(0, a, 0) + asteroid_Calc;
glm::vec3 distance = asteroid_Pos - spaceshipPos;

double step = 0.0000001;

GLuint VAO,VBO;
float lastAsteroidTime = 0;
float aspectRatio = 1.f;
glm::vec3 ammoPos;
unsigned int textureID;

float tiltAngleSide;
float tiltAngleUpDown;

int colission = 3;
int star = 0;


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
	//Core::SetActiveTexture(textureID, "colorTexture", programSkyBox, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	Core::DrawContext(context);
	glEnable(GL_DEPTH_TEST);

}

void generateAsteroids(glm::vec3 asteroid_Pos, glm::vec3 distance, double step) {
	glm::vec3 normalizedDir = glm::normalize(distance);
	asteroid_Pos = asteroid_Pos - normalizedDir *step;
	drawObjectTexture(sphereContext, glm::translate(asteroid_Pos) * glm::scale(glm::vec3(0.1f)), texture::moon, texture::moonNormal,  texture::metalnessSphere, texture::roughnessSphere);
			
}
float speed = 0.03;
float starind = 50;



bool checkCollision(glm::vec3 objectPos, float objectRadius, glm::vec3 spaceshipPos, float spaceshipRadius, bool isStar) {
	float distance = glm::length(objectPos - spaceshipPos);
	float starExtraRadius = isStar ? 0.03f : 0.0f;
	return distance < objectRadius + spaceshipRadius + starExtraRadius;
}

void generatePlanetoidBelt() {
	
	

	for (int i = 0; i < 400; ++i) {
		float z = planetoidsArray[i][0];
		float y = planetoidsArray[i][1];
		float pScale = planetoidsArray[i][2];
		bool collision = false;
		planetoidsArray[i][3] -= speed;

		if (planetoidsArray[i][4] == 1) {
			// Planeta ju¿ uczestniczy³a w kolizji, przejdŸ do kolejnej iteracji
			continue;
		}

		if (planetoidsArray[i][3] < -3.f) {
			planetoidsArray[i][3] = 10.f;
		}
		float x = planetoidsArray[i][3];
		
		for (int j = 0; j < i; ++j) {
			float prevZ = planetoidsArray[j][0];
			float prevY = planetoidsArray[j][1];
			float prevScale = planetoidsArray[j][2];
			float prevX = planetoidsArray[j][3];

			float distance = std::sqrt((z - prevZ) * (z - prevZ) + (y - prevY) * (y - prevY) + (x - prevX) * (x - prevX));

			float sumRadii = pScale + prevScale;
		


			if (distance < sumRadii) {
				collision = true;
				break;
			}
		}
		if (!collision) {
			if( fmod(i, starind) == 0) {
				if (checkCollision(glm::vec3(x, y, z), 0.1f, spaceshipPos, 0.025f,true)) {
					// Kolizja z gwiazd¹
					std::cout << "Collision with star " << i << std::endl;
					planetoidsArray[i][4] = 1;
					
				}
		}
			else if (checkCollision(glm::vec3(x, y, z), 0.1f, spaceshipPos, 0.025f, true))
			{
				//kolizja z asteroida
				std::cout << "Collision with asteroid " << i << std::endl;
				colission--;
				planetoidsArray[i][4] = 1;
				if (colission == 0)
				{
					exit(0);
				}
			}
		}
		
		if (!collision) {
			if (fmod(i, starind) == 0) {
				float time = glfwGetTime();
				glm::mat4 modelMatrix = glm::translate(glm::vec3(x, y, z)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 1.0f));
				drawObjectColor(starContext, modelMatrix * glm::eulerAngleX(time) * glm::scale(glm::vec3(0.03f)), glm::vec3(0.2,0.3,0.2));
				if (star == 0)
				{
					star++;
					std::cout << "Star " << star << std::endl;

				}
				
			}
			else {
				drawObjectTexture(sphereContext, glm::translate(glm::vec3(x, y, z)) * glm::scale(glm::vec3(pScale)), texture::moon, texture::moonNormal, texture::metalnessSphere, texture::roughnessSphere);
				

			}
		}
		
	}
	if(star==1)
	{
		star = 0;
	}
	else
	{
		exit(0);
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



void renderScene(GLFWwindow* window)
{
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();



	drawObjectSkyBox(cubeContext, glm::translate(cameraPos));


	drawObjectTexture(sphereContext, glm::translate(glm::vec3(20.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(), glm::vec3(2.0f, 2.0f, 2.0f)), texture::sun, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);

	drawObjectTexture(sphereContext, glm::translate(glm::vec3(18.0f, 0.0f, 3.0f)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.3f)), texture::earth, texture::earthNormal, texture::metalnessSphere, texture::roughnessSphere);
	//drawObjectTexture(sphereContext,
	//	glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(8.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)), texture::moon, texture::moonNormal, texture::metalnessSphere, texture::roughnessSphere);

	drawObjectTexture(sphereContext,  glm::translate(glm::vec3(18.f, 0, -3.0f)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.15f)), texture::mercury, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);
	drawObjectTexture(sphereContext,  glm::translate(glm::vec3(17.f, 0, -4.0f)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.2f)), texture::mars, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);
	drawObjectTexture(sphereContext,  glm::translate(glm::vec3(17.f, 0, 4.0f)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.3f)), texture::venus, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);
	drawObjectTexture(sphereContext,  glm::translate(glm::vec3(15.f, 0, -6.0f)) * glm::eulerAngleY(time/500000) * glm::scale(glm::vec3(0.9f)), texture::jupiter, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);
	drawObjectTexture(sphereContext,  glm::translate(glm::vec3(15.f, 0, 6.0f)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.9f)), texture::saturn, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);
	drawObjectTexture(sphereContext,  glm::translate(glm::vec3(13.f, 0, 8.0f)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.6f)), texture::uranus, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);
	drawObjectTexture(sphereContext, glm::translate(glm::vec3(13.f, 0, -8.0f)) * glm::eulerAngleY(time) * glm::scale(glm::vec3(0.6f)), texture::neptune, texture::rustNormal, texture::metalnessSphere, texture::roughnessSphere);


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

	generatePlanetoidBelt();
	lastAsteroidTime = glfwGetTime();
	if (fire == true){
		ammoPos = ammoPos + glm::vec3(0.1f, 0.f, 0.f);
		std::cout << ammoPos.x;
		glm::mat4 modelMatrix = glm::translate(ammoPos) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		drawObjectColor(saberContext, glm::translate(ammoPos) * glm::scale(glm::vec3(0.005f)), glm::vec3(0.2, 0.3, 0.2));
		}

	drawObjectColor(starContext, glm::translate(glm::vec3(10.0f, 2.0f, 12.0f)) * glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.03f)), glm::vec3(0, 1, 0));
	drawObjectColor(starContext, glm::translate(glm::vec3(10.0f, 2.0f, 11.5f)) * glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.03f)), glm::vec3(0, 1, 0));
	drawObjectColor(starContext, glm::translate(glm::vec3(10.0f, 2.0f, 11.0f)) * glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.03f)), glm::vec3(0, 1, 0));

	drawObjectColor(heartContext, glm::translate(glm::vec3(10.0f, 1.0f, 11.0f)) * glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.025f)), glm::vec3(1, 0, 0));
	drawObjectColor(heartContext, glm::translate(glm::vec3(10.0f, 1.0f, 10.5f)) * glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.025f)), glm::vec3(1, 0, 0));
	drawObjectColor(heartContext, glm::translate(glm::vec3(10.0f, 1.0f, 10.0f)) * glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.025f)), glm::vec3(1, 0, 0));
	drawObjectColor(heartContext, glm::translate(glm::vec3(10.0f, 1.0f, 11.5f)) * glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.025f)), glm::vec3(1, 0, 0));
	drawObjectColor(heartContext, glm::translate(glm::vec3(10.0f, 1.0f, 12.0f)) * glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.025f)), glm::vec3(1, 0, 0));

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

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship_new.obj", shipContext);
	loadModelToContext("./models/cube.obj", cubeContext);
	loadModelToContext("./models/estrellica.obj", starContext);
	loadModelToContext("./models/CraneoOBJ.obj", saberContext);
	loadModelToContext("./models/heart.obj", heartContext);

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
	for (int i = 0; i < 400; ++i) {
		float z = radiusDistribution(gen);
		float x = planetoidsXDistribution(gen);
		float y = planetoidsYDistribution(gen);
		float pScale = planetoidsScaleDistribution(gen);
		planetoidsArray[i][0] = z;
		planetoidsArray[i][1] = y;
		planetoidsArray[i][2] = pScale;
		planetoidsArray[i][3] = x;
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
		tiltAngleUpDown += easeInExpo(x);
	}
	else {
		if (tiltAngleUpDown > 0) {
			tiltAngleUpDown -= 0.003;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		spaceshipPos -= glm::vec3(0.f, moveSpeed, 0.f);
		tiltAngleUpDown -= easeInExpo(x);
	}
	else {
		if (tiltAngleUpDown < 0) {
			tiltAngleUpDown += 0.003;
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
