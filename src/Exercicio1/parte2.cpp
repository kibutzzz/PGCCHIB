#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>

#define WIDTH 800
#define HEIGHT 600
#define WINDOW_TITLE "Exercicio Modulo 2 - Triangulos - Leonardo Ramos"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct Triangle
{
    float posX, posY;
    float r, g, b;
    float size;
    GLuint vao;
};

GLuint createShader(GLchar *shaderSource, GLenum shaderType);
Triangle createTriangle(float x, float y, float r, float g, float b, float size);
void initializeTriangleVao(Triangle &triangle);
int createShaderProgram(GLuint vertexShader, GLuint fragmentShader);

void setupGlConfiguration();
void setupGlad();
void setViewportDimensions(GLFWwindow *window);
void assertProgramLinkingStatus(GLuint shaderProgram);
void assertShaderCompilationStatus(GLuint shader);

void cursorClickCallback(GLFWwindow *window, int button, int action, int mods);

int createShaderProgram();
void drawTriangle(Triangle triangle, GLint colorLoc);
GLFWwindow *makeWindow(int width, int height, const char *title);
float randomNumber(float min, float max);

std::vector<Triangle> triangles;

int main()
{
    glfwInit();

    setupGlConfiguration();

    GLFWwindow *window = makeWindow(WIDTH, HEIGHT, WINDOW_TITLE);
    glfwMakeContextCurrent(window);

    setupGlad();
    setViewportDimensions(window);

    GLuint shaderId = createShaderProgram();
    GLint colorLoc = glGetUniformLocation(shaderId, "inputColor");
    glUseProgram(shaderId);

    glfwSetMouseButtonCallback(window, cursorClickCallback);

    

    // v0(-0.1, -0.1),   v1(0.1, -0.1),  v2(0.0, 0.1)
    Triangle triangle = createTriangle(100, 100, 0.1, 0.4, 0.2, 50);
    initializeTriangleVao(triangle);

    mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "projection"), 1, GL_FALSE, value_ptr(projection));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);
        
        drawTriangle(triangle, colorLoc);

        for(Triangle &t : triangles)
        {
            // std::cout << "Triangle: " << t.posX << ", " << t.posY << std::endl;
            drawTriangle(t, colorLoc);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void drawTriangle(Triangle triangle, GLint colorLoc)
{
    // std::cout << "Triangle: " << triangle.posX << ", " << triangle.posY << std::endl;
    // std::cout << "Triangle color: " << triangle.r << ", " << triangle.g << ", " << triangle.b << std::endl;
    glBindVertexArray(triangle.vao);
    glUniform4f(colorLoc, triangle.r, triangle.g, triangle.b, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

Triangle createTriangle(float x, float y, float r, float g, float b, float size)
{
    Triangle triangle;
    triangle.r = r;
    triangle.g = g;
    triangle.b = b;
    triangle.posX = x;
    triangle.posY = y;
    triangle.size = size;

    return triangle;
}

void initializeTriangleVao(Triangle &triangle)
{
    GLfloat vertices[] = {
        triangle.posX - triangle.size, triangle.posY + triangle.size, 0.0f,
        triangle.posX, triangle.posY - triangle.size, 0.0f,
        triangle.posX + triangle.size, triangle.posY + triangle.size, 0.0f};

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    triangle.vao = VAO;
}

GLuint createShader(GLchar *shaderSource, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    assertShaderCompilationStatus(shader);
    return shader;
}

void assertShaderCompilationStatus(GLuint shader)
{
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;

        exit(1);
    }
}

int createShaderProgram()
{
    GLuint vertexShader = createShader(R"(
        #version 400
        layout (location = 0) in vec3 position;
        uniform mat4 projection;
        void main()
        {
            gl_Position = projection * vec4(position.x, position.y, position.z, 1.0);
        }
        )",
                                       GL_VERTEX_SHADER);

    GLuint fragmentShader = createShader(R"(
        #version 400
        uniform vec4 inputColor;
        out vec4 color;
        void main()
        {
            color = inputColor;
        }
        )",
                                         GL_FRAGMENT_SHADER);

    GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int createShaderProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    assertProgramLinkingStatus(shaderProgram);
    return shaderProgram;
}

void assertProgramLinkingStatus(GLuint shaderProgram)
{
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
}

void setViewportDimensions(GLFWwindow *window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
}

GLFWwindow *makeWindow(int width, int height, const char *title)
{
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar a janela GLFW" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    return window;
}
void setupGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }
}
void setupGlConfiguration()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);
}

void cursorClickCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        Triangle triangle = createTriangle(mouseX, mouseY, randomNumber(0, 1), randomNumber(0, 1), randomNumber(0, 1), randomNumber(10, 100));
        initializeTriangleVao(triangle);
        std::cout << "Triangle: " << triangle.posX << ", " << triangle.posY << std::endl;
        std::cout << "Triangle color: " << triangle.r << ", " << triangle.g << ", " << triangle.b << std::endl;
        std::cout << "Triangle VAO: " << triangle.vao << std::endl;
        std::cout << "Triangle size: " << triangle.size << std::endl;
        triangles.push_back(triangle);
    }
}

float randomNumber(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(min, max);
    return dist(gen);
}