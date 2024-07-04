// Tarefa M6 - Gabriel M. Carossi

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Bezier.h"

// Protótipos das funções
int setupGeometry();
int loadSimpleOBJ(std::string filepath, int& nVerts, glm::vec3 color = glm::vec3(1.0, 0.0, 1.0));
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
vector <glm::vec3> generateControlPointsSet(int nPoints);
vector <glm::vec3> generateControlPointsSet();
vector <glm::vec3> generateControlPointsSet1();
GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

//Instanciando o objeto camera
Camera camera;
bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Projeto Bezier - Gabriel", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Registro das funções de callback
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD: carrega todos os ponteiros de funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Obtendo as informações de versão
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported " << version << std::endl;

    // Definindo as dimensões da viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilando e buildando o programa de shader
    Shader shader("Phong.vs", "Phong.fs");
    glUseProgram(shader.ID);

    // Matriz de projeção perspectiva
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("projection", glm::value_ptr(projection));

    glEnable(GL_DEPTH_TEST);

    int nVerts;
    GLuint VAO = loadSimpleOBJ("../../3D_models/Suzanne/suzanneTriLowPoly.obj", nVerts, glm::vec3(0.0, 1.0, 1.0));
    GLuint VAO2 = loadSimpleOBJ("../../3D_models/Suzanne/suzanneTriLowPoly.obj", nVerts);
    GLuint VAO3 = loadSimpleOBJ("../../3D_models/Suzanne/suzanneTriLowPoly.obj", nVerts, glm::vec3(1.0, 1.0, 1.0));

    Mesh suzanne1, suzanne2, suzanne3;
    suzanne1.initialize(VAO, nVerts, &shader, glm::vec3(-2.75, 0.0, 0.0));
    suzanne2.initialize(VAO2, nVerts, &shader);
    suzanne3.initialize(VAO3, nVerts, &shader, glm::vec3(2.75, 0.0, 0.0));

    // Propriedades do material da superfície
    shader.setFloat("ka", 0.2);
    shader.setFloat("kd", 0.5);
    shader.setFloat("ks", 0.5);
    shader.setFloat("q", 10.0);

    // Fonte de luz pontual
    shader.setVec3("lightPos", -2.0, 10.0, 2.0);
    shader.setVec3("lightColor", 1.0, 1.0, 0.0);

	//Conjunto de pontos de controle
	std::vector<glm::vec3> controlPoints = generateControlPointsSet();
	std::vector<glm::vec3> controlPoints1 = generateControlPointsSet1();

	GLuint VAOControl = generateControlPointsBuffer(controlPoints);

	Bezier bezier;
	bezier.setControlPoints(controlPoints);
	bezier.setShader(&shader);
	bezier.generateCurve(10);

	Bezier bezier1;
	bezier1.setControlPoints(controlPoints1);
	bezier1.setShader(&shader);
	bezier1.generateCurve(15);

	int nbCurvePoints = bezier.getNbCurvePoints();
	int i = 0;

	const double fpsLimit = 1.0 / 10.0;
	double lastUpdateTime = 0;  // number of seconds since the last loop
	double lastFrameTime = 0;   // number of seconds since the last frame

    // Loop da aplicação
    while (!glfwWindowShouldClose(window)) {
        // Checa eventos de input
        glfwPollEvents();

        // Limpa o buffer de cor
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        // Atualizando a posição e orientação da câmera
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", glm::value_ptr(view));

        // Atualizando o shader com a posição da câmera
        shader.setVec3("cameraPos", camera.Position.x, camera.Position.y, camera.Position.z);

		double now = glfwGetTime();
		double deltaTime = now - lastUpdateTime;

		if ((now - lastFrameTime) >= fpsLimit)
		{
			glm::vec3 pointOnCurve = bezier.getPointOnCurve(i);
			std::vector <glm::vec3> aux;
			aux.push_back(pointOnCurve);

			glm::vec3 pointOnCurve1 = bezier1.getPointOnCurve(i);
			std::vector <glm::vec3> aux1;
			aux.push_back(pointOnCurve1);

			//glBindVertexArray(VAOPoint);

			//shader.setVec4("finalColor", 0, 0, 0, 1);
			// Chamada de desenho - drawcall
			// CONTORNO e PONTOS - GL_LINE_LOOP e GL_POINTS
			//glDrawArrays(GL_POINTS, 0, aux.size());
			//glDrawArrays(GL_LINE_STRIP, 0, controlPoints.size());
			//glBindVertexArray(0);

			// Chamada de desenho
			shader.setFloat("q", 10.0);
			suzanne1.update(pointOnCurve);
			suzanne1.draw();

			shader.setFloat("q", 1.0);
			suzanne2.update(pointOnCurve1);
			suzanne2.draw();

			//shader.setFloat("q", 250.0);
			//suzanne3.update(bezier.getPointOnCurve(i));
			//suzanne3.draw();

			i = (i + 1) % nbCurvePoints;

			// Troca os buffers da tela
			glfwSwapBuffers(window);

			// only set lastFrameTime when you actually draw something
			lastFrameTime = now;
		}

		// set lastUpdateTime every iteration
		lastUpdateTime = now;
    }

    // Desaloca os buffers da OpenGL
	glDeleteVertexArrays(1, &VAOControl);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VAO2);
    glDeleteBuffers(1, &VAO3);

    // Finaliza a GLFW
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    float cameraSpeed = 2.5f * 0.01f; // Ajustar a velocidade conforme necessário
    if (key == GLFW_KEY_W)
        camera.ProcessKeyboard('W', cameraSpeed);
    if (key == GLFW_KEY_S)
        camera.ProcessKeyboard('S', cameraSpeed);
    if (key == GLFW_KEY_A)
        camera.ProcessKeyboard('A', cameraSpeed);
    if (key == GLFW_KEY_D)
        camera.ProcessKeyboard('D', cameraSpeed);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints)
{
	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat) * 3, controlPoints.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color)
{
	vector <glm::vec3> vertices;
	vector <GLuint> indices;
	vector <glm::vec2> texCoords;
	vector <glm::vec3> normals;
	vector <GLfloat> vbuffer;

	ifstream inputFile;
	inputFile.open(filepath.c_str());
	if (inputFile.is_open())
	{
		char line[100];
		string sline;



		while (!inputFile.eof())
		{
			inputFile.getline(line, 100);
			sline = line;

			string word;

			istringstream ssline(line);
			ssline >> word;

			//cout << word << " ";
			if (word == "v")
			{
				glm::vec3 v;

				ssline >> v.x >> v.y >> v.z;

				vertices.push_back(v);
			}
			if (word == "vt")
			{
				glm::vec2 vt;

				ssline >> vt.s >> vt.t;

				texCoords.push_back(vt);
			}
			if (word == "vn")
			{
				glm::vec3 vn;

				ssline >> vn.x >> vn.y >> vn.z;

				normals.push_back(vn);
			}
			if (word == "f")
			{
				string tokens[3];

				ssline >> tokens[0] >> tokens[1] >> tokens[2];

				for (int i = 0; i < 3; i++)
				{
					//Recuperando os indices de v
					int pos = tokens[i].find("/");
					string token = tokens[i].substr(0, pos);
					int index = atoi(token.c_str()) - 1;
					indices.push_back(index);

					vbuffer.push_back(vertices[index].x);
					vbuffer.push_back(vertices[index].y);
					vbuffer.push_back(vertices[index].z);
					vbuffer.push_back(color.r);
					vbuffer.push_back(color.g);
					vbuffer.push_back(color.b);

					//Recuperando os indices de vts
					tokens[i] = tokens[i].substr(pos + 1);
					pos = tokens[i].find("/");
					token = tokens[i].substr(0, pos);
					index = atoi(token.c_str()) - 1;

					vbuffer.push_back(texCoords[index].s);
					vbuffer.push_back(texCoords[index].t);

					//Recuperando os indices de vns
					tokens[i] = tokens[i].substr(pos + 1);
					index = atoi(tokens[i].c_str()) - 1;

					vbuffer.push_back(normals[index].x);
					vbuffer.push_back(normals[index].y);
					vbuffer.push_back(normals[index].z);
				}
			}

		}

	}
	else
	{
		cout << "Problema ao encontrar o arquivo " << filepath << endl;
	}
	inputFile.close();

	GLuint VBO, VAO;

	nVerts = vbuffer.size() / 11; //Provisório

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(GLfloat), vbuffer.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Atributo coordenada de textura (s, t)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	//Atributo normal do vértice (x, y, z)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;

}

vector<glm::vec3> generateControlPointsSet()
{
	vector <glm::vec3> controlPoints;

	controlPoints.push_back(glm::vec3(-0.6, -0.4, 0.0));
	controlPoints.push_back(glm::vec3(-0.4, -0.6, 0.0));
	controlPoints.push_back(glm::vec3(-0.2, -0.2, 0.0));
	controlPoints.push_back(glm::vec3(0.0, 0.0, 0.0));
	controlPoints.push_back(glm::vec3(0.2, 0.2, 0.0));
	controlPoints.push_back(glm::vec3(0.4, 0.6, 0.0));
	controlPoints.push_back(glm::vec3(0.6, 0.4, 0.0));

	return controlPoints;
}

vector<glm::vec3> generateControlPointsSet1()
{
	vector <glm::vec3> controlPoints;

	controlPoints.push_back(glm::vec3(0.6, -0.4, 2.0));
	controlPoints.push_back(glm::vec3(0.4, -0.6, 2.0));
	controlPoints.push_back(glm::vec3(0.2, -0.2, 2.0));
	controlPoints.push_back(glm::vec3(0.0, 0.0, 2.0));
	controlPoints.push_back(glm::vec3(-0.2, 0.2, 2.0));
	controlPoints.push_back(glm::vec3(-0.4, 0.6, 2.0));
	controlPoints.push_back(glm::vec3(-0.6, 0.4, 2.0));

	return controlPoints;
}