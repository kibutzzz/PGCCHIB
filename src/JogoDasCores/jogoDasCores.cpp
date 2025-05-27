#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;
const int COLUMNS = 8;
const int ROWS = 6;
const GLuint RECTANGLE_WIDTH = 100;
const GLuint RECTANGLE_HEIGHT = 100;
const char *WINDOW_TITLE = "Jogo das Cores - Módulo 3";

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
void assertShaderCompilationStatus(GLuint shader)
{
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
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

    assertShaderCompilationStatus(shader);
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
            color = inputColor;
        }
    )",
                                               GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    assertProgramLinkingStatus(shaderProgram);

    std::cout << "Shader program criado e vinculado com sucesso!" << std::endl;
    return shaderProgram;
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
        std::cout << "Tecla R pressionada" << std::endl;
    }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << "Clique do mouse: " << xpos << ", " << ypos << std::endl;
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

struct Rectangle
{
    glm::vec3 position;
    glm::vec3 dimensions;
    glm::vec3 color;
    bool eliminated;
};

Rectangle grid[ROWS][COLUMNS];

void initializeGrid()
{

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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        glBindVertexArray(rectangleVAO);
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                Rectangle &currentRectangle = grid[i][j];

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

                GLint colorLocation = glGetUniformLocation(shaderId, "inputColor");
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