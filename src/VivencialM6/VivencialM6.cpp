#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "../../build/_deps/stb_image-src/stb_easy_font.h"
#include <string>
#include <vector>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;
const char *WINDOW_TITLE = "Vivencia M6";

int playerX = 3;
int playerY = 3;

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
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 color;
        layout (location = 2) in vec2 texc;
        out vec3 vColor;
        out vec2 tex_coord;

        uniform mat4 projection;
        uniform mat4 model;
        uniform int frameIndex;
        void main()
        {
            vColor = color;
            tex_coord = vec2(texc.x + float(frameIndex) * 0.142857, texc.y); // Assuming 7 frames in the texture atlas
            gl_Position = projection * model * vec4(position, 1.0);
        }
        )",
                                             GL_VERTEX_SHADER);

    const GLuint fragmentShader = createShader(R"(
        #version 400
        in vec3 vColor;
        in vec2 tex_coord;
        out vec4 color;
        uniform sampler2D tex_buff;
        void main()
        {
            color = texture(tex_buff,tex_coord);//vec4(vColor,1.0);
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

// callbacks
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    // use qweadzxc to move player
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_W: // up
            if (playerX > 0 && playerY < 4)
            {
                playerX--;
                playerY++;
            }
            break;
        case GLFW_KEY_X: // down
            if (playerX < 4 && playerY > 0)
            {
                playerX++;
                playerY--;
            }
            break;
        case GLFW_KEY_A: // left
            if (playerX > 0 && playerY > 0)
            {
                playerX--;
                playerY--;
            }
            break;
        case GLFW_KEY_D: // right
            if (playerX < 4 && playerY < 4)
            {
                playerX++;
                playerY++;
            }
            break;
        case GLFW_KEY_Q: // up-left

            if (playerX > 0)
                playerX--;
            break;
        case GLFW_KEY_E: // up-right

            if (playerY < 4)
                playerY++;
            break;
        case GLFW_KEY_Z: // down-left
            if (playerY > 0)
                playerY--;
            break;
        case GLFW_KEY_C: // down-right

            if (playerX < 4)
                playerX++;
            break;
        default:
            break;
        }
    }
}

int loadTexture(std::string filePath)
{
    GLuint texID;

    // Gera o identificador da textura na memória
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;

    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        if (nrChannels == 3) // jpg, bmp
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else // png
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

GLfloat calcIsoX(float x, float y)
{
    return (x - y);
}
GLfloat calcIsoY(float x, float y)
{
    return (x + y) / 2;
}
int setupGeometry()
{

    // Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
    // sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
    // Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
    // Pode ser arazenado em um VBO único ou em VBOs separados
    GLfloat vertices[] = {
        // x      y      z      r    g    b      s           t
        // T0
        0.0,    0.5,    0.0,    0.0, 0.0, 0.0,  0.0,        0.5,        //
        0.5,    1.0,    0.0,    0.0, 0.0, 0.0,  1.0 / 14.0, 1.0,        //
        1.0,    0.5,    0.0,    0.0, 0.0, 0.0,  1.0 / 7.0,  0.5,        //
        0.5,    0.0,    0.0,    0.0, 0.0, 0.0,  1.0 / 14.0, 0.0,        //
    };

    GLuint VBO, VAO;
    // Geração do identificador do VBO
    glGenBuffers(1, &VBO);
    // Faz a conexão (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Geração do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);
    // Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
    // e os ponteiros para os atributos
    glBindVertexArray(VAO);
    // Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
    //  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
    //  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
    //  Tipo do dado
    //  Se está normalizado (entre zero e um)
    //  Tamanho em bytes
    //  Deslocamento a partir do byte zero

    // Ponteiro pro atributo 0 - Posição - coordenadas x, y, z
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // Ponteiro pro atributo 1 - Cor - componentes r,g e b
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Ponteiro pro atributo 2 - Coordenada de textura - coordenadas s,t
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
    // atualmente vinculado - para que depois possamos desvincular com segurança
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)

    unsigned int indices[] = {
        0, 1, 2, // Primeiro triângulo
        0, 2, 3  // Segundo triângulo
    };

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    return VAO;
}

struct Sprite
{
    GLuint VAO;
    GLuint textureId;
    GLuint shaderId;
    glm::vec3 translate;
    glm::vec3 scale;
    int frameIndex;
};

bool isPlayerPosition(int x, int y)
{
    return (x == playerX && y == playerY);
}

void draw(const Sprite &sprite, int x, int y)
{
    glBindTexture(GL_TEXTURE_2D, sprite.textureId);
    glBindVertexArray(sprite.VAO);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sprite.translate);
    model = glm::scale(model, sprite.scale);

    glUniformMatrix4fv(glGetUniformLocation(sprite.shaderId, "model"), 1, GL_FALSE, glm::value_ptr(model));
    int frameIndex = isPlayerPosition(x, y) ? 6 : sprite.frameIndex;

    glUniform1i(glGetUniformLocation(sprite.shaderId, "frameIndex"), frameIndex);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main()
{

    std::cout << "Jogo das Cores - Módulo 3 - Leonardo Meinerz Ramos" << std::endl;

    initializeGlfw();
    setupGlConfiguration();

    GLFWwindow *window = makeWindow(WIDTH, HEIGHT, WINDOW_TITLE);

    GLuint shaderId = createShaderProgram();
    glUseProgram(shaderId);

    glfwSetKeyCallback(window, keyCallback);

    GLuint VAO = setupGeometry();

    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    std::cout << "Matriz de projeção definida!" << std::endl;

    GLuint texID = loadTexture("../assets/sprites/tilesetIso.png");

    Sprite jorge = Sprite();
    jorge.VAO = VAO;
    jorge.textureId = texID;
    jorge.shaderId = shaderId;
    jorge.scale = glm::vec3(100.0f, 100.0f, 1.0f);
    jorge.translate = glm::vec3(0.0f, 0.0f, 0.0f);

    int mapWidth = 5;
    int mapHeight = 5;
    std::vector<std::vector<Sprite>> map;

    std::vector<std::vector<int>> mapData = {
        {0, 1, 2, 3, 4},
        {5, 4, 4, 3, 4},
        {0, 0, 0, 0, 0},
        {3, 4, 1, 5, 4},
        {2, 3, 4, 1, 1}};

    for (int i = 0; i < mapHeight; ++i)
    {
        std::vector<Sprite> row;
        for (int j = 0; j < mapWidth; ++j)
        {
            Sprite tile = jorge;

            float x = j * tile.scale.x / 2.0f + i * tile.scale.y / 2.0f;
            float y = i * tile.scale.x / 2.0f - j * tile.scale.y / 2.0f;

            tile.translate = glm::vec3(x + WIDTH / 4.0f, y + HEIGHT / 4.0f, 0.0f);
            tile.frameIndex = mapData[i][j];
            row.push_back(tile);
        }
        map.push_back(row);
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        for (size_t i = 0; i < map.size(); ++i)
        {
            for (size_t j = 0; j < map[i].size(); ++j)
            {
                draw(map[i][j], i, j);
            }
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}