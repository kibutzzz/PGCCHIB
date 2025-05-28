#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "../../build/_deps/stb_image-src/stb_easy_font.h"

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;
const int COLUMNS = 8;
const int ROWS = 6;
const GLuint RECTANGLE_WIDTH = 100;
const GLuint RECTANGLE_HEIGHT = 100;
const float TOLERANCE = 0.2f;
const float MAXIMUM_DISTANCE = sqrt(3.0f);
const char *WINDOW_TITLE = "Jogo das Cores - Módulo 3";
bool finished = false;
float attempts = 0;

struct Rectangle
{
    glm::vec3 position;
    glm::vec3 dimensions;
    glm::vec3 color;
    bool eliminated;
};

Rectangle grid[ROWS][COLUMNS];

// initial setup (GLAD, GL hints and window configuration)
void setupGlConfiguration()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    std::cout << "Configuração do OpenGL definida com sucesso!" << std::endl;
}

void setViewportDimensions(GLFWwindow *window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    std::cout << "Dimensões da viewport obtidas: " << width << "x" << height << std::endl;
    glViewport(0, 0, width, height);
    std::cout << "Viewport configurada com sucesso!" << std::endl;
}
void setupGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "GLAD inicializado com sucesso!" << std::endl;
}

GLFWwindow *makeWindow(GLuint width, GLuint height, const char *title)
{
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar a janela" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    std::cout << "Janela criada com sucesso!" << std::endl;
    glfwMakeContextCurrent(window);
    if (!glfwGetCurrentContext())
    {
        std::cerr << "Erro: Contexto OpenGL não foi configurado corretamente!" << std::endl;
        exit(EXIT_FAILURE);
    }
    setupGlad();
    std::cout << "Contexto OpenGL atual definido!" << std::endl;
    setViewportDimensions(window);
    std::cout << "Dimensões da viewport definidas!" << std::endl;

    return window;
}

void initializeGlfw()
{
    if (!glfwInit())
    {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "GLFW inicializado com sucesso!" << std::endl;
}

// Shader configuration

void checkOpenGLError(const std::string &context)
{
    GLenum error = glGetError();
    while (error != GL_NO_ERROR)
    {
        std::string errorMessage;
        switch (error)
        {
        case GL_INVALID_ENUM:
            errorMessage = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
            break;
        case GL_INVALID_VALUE:
            errorMessage = "GL_INVALID_VALUE: A numeric argument is out of range.";
            break;
        case GL_INVALID_OPERATION:
            errorMessage = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorMessage = "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
            break;
        case GL_OUT_OF_MEMORY:
            errorMessage = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
            break;
        default:
            errorMessage = "Unknown error.";
            break;
        }
        std::cerr << "OpenGL Error (" << context << "): " << errorMessage << std::endl;
        error = glGetError(); // Check for additional errors
    }
}
void assertShaderCompilationStatus(GLuint shader)
{
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
}

GLuint createShader(GLchar *shaderSource, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    if (!shader)
    {
        std::cerr << "Erro ao criar shader" << std::endl;
        exit(EXIT_FAILURE);
    }

    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);
    checkOpenGLError("Shader Complilation");

    assertShaderCompilationStatus(shader);
    return shader;
}

void assertProgramLinkingStatus(GLuint shaderProgram)
{
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
}

GLuint createShaderProgram()
{
    const GLuint vertexShader = createShader(R"(
        #version 400 
        layout(location = 0) in vec3 position;
        uniform mat4 projection;
        uniform mat4 model;
        void main() {
	        gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
        }
    )",
                                             GL_VERTEX_SHADER);

    const GLuint fragmentShader = createShader(R"(
        #version 400
        uniform vec4 inputColor;
        out vec4 color;
        void main() {
            color = inputColor * vec4(1.0); // Ensure inputColor is used
        }
    )",
                                               GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkOpenGLError("Shader Program Linking");
    assertProgramLinkingStatus(shaderProgram);

    std::cout << "Shader program criado e vinculado com sucesso!" << std::endl;
    return shaderProgram;
}

bool areAllSquaresEliminated()
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            if (!grid[i][j].eliminated)
            {
                return false; // Found a square that is not eliminated
            }
        }
    }
    return true; // All squares are eliminated
}

void initializeGrid()
{

    finished = false;
    attempts = 0;
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            Rectangle rectangle;
            glm::vec2 initialPosition = glm::vec2(RECTANGLE_WIDTH / 2, RECTANGLE_HEIGHT / 2);
            rectangle.position = glm::vec3(initialPosition.x + j * RECTANGLE_WIDTH, initialPosition.y + i * RECTANGLE_HEIGHT, 0.0f);
            rectangle.dimensions = glm::vec3(RECTANGLE_WIDTH, RECTANGLE_HEIGHT, 1.0f);

            float r = rand() % 256 / 255.0f;
            float g = rand() % 256 / 255.0f;
            float b = rand() % 256 / 255.0f;

            rectangle.color = glm::vec3(r, g, b);
            rectangle.eliminated = false;
            grid[i][j] = rectangle;
        }
    }
}

// callbacks
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        std::cout << "Reiniciando o jogo" << std::endl;
        initializeGrid();
    }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << "Clique do mouse: " << xpos << ", " << ypos << std::endl;
        int x = xpos / RECTANGLE_WIDTH;
        int y = ypos / RECTANGLE_HEIGHT;

        Rectangle &selectedRectangle = grid[y][x];
        selectedRectangle.eliminated = true;

        std::cout << "Quadrado selecionado: " << x << ", " << y << std::endl;

        glm::vec3 currentColor = selectedRectangle.color;

        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                float distance = sqrt(pow(currentColor.r - grid[i][j].color.r, 2) +
                                      pow(currentColor.g - grid[i][j].color.g, 2) +
                                      pow(currentColor.b - grid[i][j].color.b, 2));

                float normalizedDistance = distance / MAXIMUM_DISTANCE;
                if (normalizedDistance < TOLERANCE)
                {
                    grid[i][j].eliminated = true;
                }
            }
        }

        attempts++;

        std::cout << "Pontuanção Atual: " << 100.0f / sqrt(attempts) << std::endl;
        if (areAllSquaresEliminated())
        {
            std::cout << "Todos os quadrados foram eliminados!" << std::endl;
            std::cout << "Reinicie o jogo pressionando R." << std::endl;
        }
    }
}

GLuint createRectangle()
{
    GLfloat vertices[] = {
        -0.5, 0.5, 0.0,  // Top-left
        -0.5, -0.5, 0.0, // Bottom-left
        0.5, 0.5, 0.0,   // Top-right
        0.5, -0.5, 0.0   // Bottom-right
    };

    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint VAO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    const GLuint POSITION_ATTRIBUTE_LOCATION = 0;
    const GLuint POSITION_ATTRIBUTE_SIZE = 3;
    glVertexAttribPointer(POSITION_ATTRIBUTE_LOCATION, POSITION_ATTRIBUTE_SIZE, GL_FLOAT, GL_FALSE, POSITION_ATTRIBUTE_SIZE * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

void renderText(const char *text, float x, float y)
{
    static char buffer[99999]; // pode armazenar até 99999 vértices
    int num_quads;

    num_quads = stb_easy_font_print(x, y, (char *)text, nullptr, buffer, sizeof(buffer));

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

int main()
{
    std::cout << "Jogo das Cores - Módulo 3 - Leonardo Meinerz Ramos" << std::endl;

    initializeGlfw();
    setupGlConfiguration();

    GLFWwindow *window = makeWindow(WIDTH, HEIGHT, WINDOW_TITLE);

    GLuint shaderId = createShaderProgram();
    glUseProgram(shaderId);

    GLint numUniforms;
    glGetProgramiv(shaderId, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::cout << "Número de uniforms ativos: " << numUniforms << std::endl;

    for (GLint i = 0; i < numUniforms; ++i)
    {
        char name[256];
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(shaderId, i, sizeof(name), &length, &size, &type, name);
        std::cout << "Uniform " << i << ": " << name << std::endl;
    }
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    GLuint rectangleVAO = createRectangle();
    std::cout << "VAO do retângulo criado com sucesso!" << std::endl;

    initializeGrid();

    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    std::cout << "Matriz de projeção definida!" << std::endl;

    GLint colorLocation = glGetUniformLocation(shaderId, "inputColor");
    checkOpenGLError("Uniform Location Retrieval");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        glBindVertexArray(rectangleVAO);

        renderText("Pressione R para reiniciar o jogo", 10.0f, 20.0f);
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                Rectangle &currentRectangle = grid[i][j];
                if (currentRectangle.eliminated)
                {
                    continue; // Skip rendering for eliminated rectangles
                }
                glm::mat4 model = glm::mat4(1);
                model = translate(model, currentRectangle.position);
                model = scale(model, currentRectangle.dimensions);

                GLint modelLocation = glGetUniformLocation(shaderId, "model");
                if (modelLocation == -1)
                {
                    std::cerr << "Erro: Uniform 'model' não encontrado no shader!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

                if (colorLocation == -1)
                {
                    std::cerr << "Erro: Uniform 'inputColor' não encontrado no shader!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                glUniform4f(colorLocation, currentRectangle.color.r, currentRectangle.color.g, currentRectangle.color.b, 1.0f);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}