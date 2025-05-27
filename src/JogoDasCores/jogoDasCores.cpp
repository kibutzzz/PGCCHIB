#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;
const char *WINDOW_TITLE = "Jogo das Cores - Módulo 3";

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

void assertShaderCompilationStatus(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
}

GLuint createShader(GLchar *shaderSource, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        std::cerr << "Erro ao criar shader" << std::endl;
        exit(EXIT_FAILURE);
    }

    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    assertShaderCompilationStatus(shader);

}

void assertProgramLinkingStatus(GLuint shaderProgram) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if ( !success) 
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }    
}

GLuint createShaderProgram() {
    const GLuint vertexShader = createShader(R"(
        #version 400 
        layout(location = 0) in vec3 position;
        void main() {
            gl_Position = vec4(position, 1.0);
        }
    )", GL_VERTEX_SHADER);

    const GLuint fragmentShader = createShader(R"(
        #version 400
        uniform vec4 inputColor;
        out vec4 color;
        void main() {
            color = inputColor;
        }
    )", GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    assertProgramLinkingStatus(shaderProgram);
    
    std::cout << "Shader program criado e vinculado com sucesso!" << std::endl;
    return shaderProgram;
}


void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << "Mouse clicked at: " << xpos << ", " << ypos << std::endl;
    }
}

int main()
{
    std::cout << "Jogo das Cores - Módulo 3 - Leonardo Meinerz Ramos" << std::endl;

    initializeGlfw();
    setupGlConfiguration();

    GLFWwindow *window = makeWindow(WIDTH, HEIGHT, WINDOW_TITLE);

    GLuint shaderId = createShaderProgram();
    
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);



        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;    
}