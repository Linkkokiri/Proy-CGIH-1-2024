/*
* Laboratorio de Computación Gráfica e Interacción Humano-Computadora
* 06 - Modelos de Iluminación
*/

#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>

#include <irrKlang.h>
using namespace irrklang;

// Functions
bool Start();
bool Update();
void sonidoAmb(float xSound, float ySound, float zSound); //void sonidoAmb(float xSound, float ySound, float zSound);
void distSond(ISound* snd);

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Gobals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f)); 

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción
// de la animación
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float elapsedTime = 0.0f;

//VARIABLES DE SONIDO Y PARA SONIDO
float xSound = 0.0f;
float ySound = 0.0f;
float zSound = 0.0f;
float xVol = 0.0f;
float yVol = 0.0f;
float zVol = 0.0f;
float guarSond;
//glm::vec3 musicPosition = glm::vec3(0.0f, 0.0f, 0.0f);

// Shaders
Shader *phongShader;
Shader *basicShader;
Shader *cubemapShader;

// Carga la información del modelo
Model	*basemodel;
Model	*balonp;
Model	*balons;
//Iluminacion

Model   *lightDummy;

float tradius = 10.0f;
float theta = 0.0f;
float alpha = 0.0f;

// Cubemap
CubeMap *mainCubeMap;

// Materiales
Material material;
Light    light;

// Audio
ISoundEngine *SoundEngine = createIrrKlangDevice();

// Entrada a función principal
int main()
{

	if (!Start())
		return -1;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		if (!Update())
			break;
	}

	glfwTerminate();
	return 0;

}

bool Start() {
	// Inicialización de GLFW

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Illumination Models", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Ocultar el cursor mientras se rota la escena
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);

	// Compilación y enlace de shadersstatic
	phongShader   = new Shader("shaders/11_BasicPhongShader.vs", "shaders/11_BasicPhongShader.fs");
	basicShader = new Shader("shaders/10_vertex_simple.vs", "shaders/10_fragment_simple.fs");
	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");

	// Dibujar en malla de alambre
	// glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	//DIBUJAR EL MODELO

	basemodel = new Model("models/IllumModels/House02.fbx");
	balonp = new Model("models/P6_TEXT/balon1.fbx");


	// Cubemap
	vector<std::string> faces
	{
		"textures/cubemap/01/posx.png",
		"textures/cubemap/01/negx.png",
		"textures/cubemap/01/posy.png",
		"textures/cubemap/01/negy.png",
		"textures/cubemap/01/posz.png",
		"textures/cubemap/01/negz.png"
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);

	// Parámetros de la Iluminación
	light.Position = glm::vec3(0.0f, 5.0f, 0.0f);
	light.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Power = glm::vec4(60.0f, 60.0f, 60.0f, 1.0f);
	light.alphaIndex = 10;
	light.distance = 5.0f;

	lightDummy = new Model("models/IllumModels/lightDummy.fbx");

	//SoundEngine->play2D("sound/EternalGarden.mp3", true); //sound/Easton Kingdom - Super Mario Land.mp3
	//SoundEngine->play2D("sound/mixkit-wind-leaves-617.mp3", true);
	vec3df musicposition(0.0f, 0.0f, 0.0f);
	ISound* snd = SoundEngine->play3D("sound/Easton Kingdom - Super Mario Land.mp3", musicposition, true, true);
	distSond(snd);
	if (snd)
	{
		snd->setMinDistance(1.0f); // configura la distancia mínima a la que el sonido puede ser escuchado en su volumen completo. 
		//Si el oyente (o receptor del sonido) está más cerca de esta distancia mínima, escuchará el sonido a su volumen máximo.
		snd->setMaxDistance(5.0f);
		snd->setIsPaused(false); // reanuda la reproducción del sonido si estaba en pausa (el false)
		vec3df position2(xVol, xVol, xVol);        // position of the listener vec3df position(xSound, ySound, zSound);
		vec3df lookDirection(0.0f, 0.0f, 1.0f); // the direction the listener looks into
		vec3df velPerSecond(0, 0, 0);    // only relevant for doppler effects
		vec3df upVector(0, 1, 0);        // where 'up' is in your 3D scene
		SoundEngine->setListenerPosition(position2, lookDirection, velPerSecond, upVector);
	}

	return true;
}

bool Update() {
	// Cálculo del framerate
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Procesa la entrada del teclado o mouse
	processInput(window);

	// Renderizado R - G - B - A
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// Cubemap (fondo)
	{
		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}
	
	// Dibujamos un objeto cualquiera
	{
		// Activamos el shader de Phong
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light.Color);
		phongShader->setVec4("LightPower", light.Power);
		phongShader->setInt("alphaIndex", light.alphaIndex);
		phongShader->setFloat("distance", light.distance);
		phongShader->setVec3("lightPosition", light.Position);
		phongShader->setVec3("lightDirection", light.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material.specular);
		phongShader->setFloat("transparency", material.transparency);

		basemodel->Draw(*phongShader);

	}

	glUseProgram(0);

	// Dibujamos el indicador auxiliar de la fuente de iluminación
	{
		basicShader->use();

		basicShader->setMat4("projection", projection);
		basicShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, light.Position);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		basicShader->setMat4("model", model);

		lightDummy->Draw(*basicShader);

	}

	glUseProgram(0);

	//BALON 1

	{
		// Activamos el shader de Phong
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-6.0f, 2.0f, 8.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light.Color);
		phongShader->setVec4("LightPower", light.Power);
		phongShader->setInt("alphaIndex", light.alphaIndex);
		phongShader->setFloat("distance", light.distance);
		phongShader->setVec3("lightPosition", light.Position);
		phongShader->setVec3("lightDirection", light.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material.specular);
		phongShader->setFloat("transparency", material.transparency);

		balonp->Draw(*phongShader);

	}

	glUseProgram(0);

	// glfw: swap buffers 
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

// Procesamos entradas del teclado
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

		camera.ProcessKeyboard(FORWARD, deltaTime);
		xVol += 5.0f;
		//guarSond = distSond(xVol);
	}
		
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {

		camera.ProcessKeyboard(BACKWARD, deltaTime);
		xVol -= 5.0f;
		//guarSond = distSond(xVol);
		//sonidoAmb(xVol);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	// Character movement
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {


	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		
	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
		
		//sonidoAmb(5.0f,5.0f,5.0f);
		//sonidoAmb();
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		//xSound = 5.0f;
		//ySound = 5.0f;
		//zSound = 5.0f;
		//sonidoAmb(xSound, ySound, zSound);
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		
	}

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		
	}
	
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		light.Position.x -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		light.Position.x += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		light.Position.y += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		light.Position.y -= 0.1f;
	
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
// de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; 

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}

//DISTANCIA DEL SONIDO

void distSond(ISound* snd) {

	

}

//SONIDO
void sonidoAmb(float xSound, float ySound, float zSound) { //void sonidoAmb(float xSound, float ySound, float zSound) {

	//ISoundEngine* SoundEngine = createIrrKlangDevice();
	//vec3df position(xSound , ySound, zSound);
	vec3df musicposition(0.0f , 0.0f, 0.0f);
	ISound* snd = SoundEngine->play3D("sound/Easton Kingdom - Super Mario Land.mp3", musicposition, true, true); 
	//El primer parametro boolean es para reproducirse en un bucle
	//El segundo parametro es para el sonido se inicia en pausa. Esto significa que el sonido se cargará y se preparará, 
	// pero no comenzará a reproducirse hasta que se le indique explícitamente que lo haga.
	if (snd)
	{
		vec3df position2(0.0f, 0.0f,0.0f);        // position of the listener vec3df position(xSound, ySound, zSound);
		vec3df lookDirection(10.0f, 0.0f, 10.0f); // the direction the listener looks into
		vec3df velPerSecond(0, 0, 0);    // only relevant for doppler effects
		vec3df upVector(0, 1, 0);        // where 'up' is in your 3D scene

		SoundEngine->setListenerPosition(position2, lookDirection, velPerSecond, upVector);
		
		snd->setMinDistance(1.0f); // configura la distancia mínima a la que el sonido puede ser escuchado en su volumen completo. 
		//Si el oyente (o receptor del sonido) está más cerca de esta distancia mínima, escuchará el sonido a su volumen máximo.
		snd->setMaxDistance(10.0f);
		snd->setIsPaused(false); // reanuda la reproducción del sonido si estaba en pausa (el false)
	}

	
}
