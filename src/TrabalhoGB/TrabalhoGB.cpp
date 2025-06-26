#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "../../build/_deps/stb_image-src/stb_easy_font.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

const int FPS = 12;
const char *WINDOW_TITLE = "Vivencia M6";

int playerX = 3;
int playerY = 3;
int mapWidth;
int mapHeight;
std::vector<std::vector<int>> mapData;

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

std::string lerArquivoParaString(const std::string& caminhoArquivo) {
    try {
        std::ifstream arquivo(caminhoArquivo);
        if (!arquivo.is_open()) {
            throw std::runtime_error("Não foi possível abrir o arquivo: " + caminhoArquivo);
        }

        std::ostringstream conteudoStream;
        conteudoStream << arquivo.rdbuf(); // Lê todo o conteúdo do arquivo
        return conteudoStream.str();
    } catch (const std::exception& e) {
        std::cerr << "Erro ao ler o arquivo: " << e.what() << std::endl;
        throw; 
    }
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

GLuint createPlayerShaderProgram() {
    const GLuint vertexShader = createShader(R"(#version 400
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 colors;
        layout (location = 2) in vec2 texture_mapping;
        
        out vec2 texture_coordinates;
        out vec3 color_values;
        
        uniform mat4 projection;
        uniform mat4 model;
        
        uniform ivec2 sheetSize;   
        uniform int frameIndex;
        
        void main()
        {
            int column = frameIndex % sheetSize.x;
            int row    = frameIndex / sheetSize.x;
            vec2 cellSize = vec2(1.0) / vec2(sheetSize);
            vec2 frameOffset = vec2(column, row) * cellSize;
            texture_coordinates = texture_mapping * cellSize + frameOffset;
            color_values = colors;
            gl_Position = projection * model * vec4(position, 1.0);
        })",
                                             GL_VERTEX_SHADER);

    const GLuint fragmentShader = createShader(R"(#version 400
        in vec2 texture_coordinates;
        in vec3 color_values;
        out vec4 color;
        
        uniform sampler2D spriteTexture;
        
        void main()
        {
            vec4 texColor = texture(spriteTexture, texture_coordinates);
            color = texColor;
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

GLuint createTileShaderProgram()
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
            tex_coord = vec2(texc.x + float(frameIndex) * 0.142857, texc.y);
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

double currentTime;
double lastTime = 0.0;
int currentPlayerFrameIndex = 0;
bool isWalking = false;
int UP = 0;
int DOWN = 1;
int LEFT = 2;
int RIGHT = 3;
int walkinDirection;

void resetWalkingAnimation () {
    isWalking = false;
}
// callbacks
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if( action == GLFW_RELEASE) {
        resetWalkingAnimation();   
    }
    if (action == GLFW_PRESS)
    {
        isWalking = true;
        switch (key)
        {
        case GLFW_KEY_W:
            if(playerX > 0 && playerY > 0)
            {
            playerX--;
            playerY--;
            }
            std::cout << "direção: W (up) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = UP;
            break;
        case GLFW_KEY_X: // down
            if (playerX < mapWidth -1 && playerY < mapHeight -1)
            {
                playerX++;
                playerY++;
            }
            std::cout << "direção: X (down) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = DOWN;
            break;
        case GLFW_KEY_A: // left
            if (playerX < mapWidth -1 && playerY > 0)
            {
                playerX++;
                playerY--;
            }
            std::cout << "direção: A (left) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = LEFT;
            break;
        case GLFW_KEY_D: // right
            if (playerX > 0 && playerY < mapHeight-1)
            {
                playerX--;
                playerY++;
            }
            std::cout << "direção: D (right) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = RIGHT;
            break;
        case GLFW_KEY_Q: // up-left

            if (playerY > 0)
                playerY--;
            std::cout << "direção: Q (up-left) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = UP;
            break;
        case GLFW_KEY_E: // up-right

            if ( playerX > 0) {
                playerX--;
            }

            std::cout << "direção: E (up-right) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = UP;
            break;
        case GLFW_KEY_Z: // down-left
            if (playerX < mapWidth -1)
                playerX++;

            std::cout << "direção: Z (down-left) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = DOWN;
            break;
        case GLFW_KEY_C: // down-right

            if (playerY < mapHeight - 1)
                playerY++;

            std::cout << "direção: C (down-right) - Posição atual: (" << playerX << ", " << playerY << ")" << std::endl;
            walkinDirection = DOWN;
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

GLuint setupPlayerVAO() {

     GLfloat vertices[] = {
        // x      y      z      r    g    b      s           t
        // T0
        0.0,    0.0,    0.0,    0.0, 0.0, 0.0,  0.0,    0.0,                //
        0.0,    1.0,    0.0,    0.0, 0.0, 0.0,  0.0,    1.0 ,       //
        1.0,    0.0,    0.0,    0.0, 0.0, 0.0,  1.0,    0.0,      //
        1.0,    1.0,    0.0,    0.0, 0.0, 0.0,  1.0,    1.0,       //
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int indices[] = {
        0, 1, 2, // Primeiro triângulo
        1, 2, 3  // Segundo triângulo
    };

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    return VAO;
}
int setupTileVAO()
{

    GLfloat vertices[] = {
        // x      y      z      r    g    b      s           t
        // T0
        0.0,    0.5,    0.0,    0.0, 0.0, 0.0,  0.0,        0.5,        //
        0.5,    1.0,    0.0,    0.0, 0.0, 0.0,  1.0 / 14.0, 1.0,        //
        1.0,    0.5,    0.0,    0.0, 0.0, 0.0,  1.0 / 7.0,  0.5,        //
        0.5,    0.0,    0.0,    0.0, 0.0, 0.0,  1.0 / 14.0, 0.0,        //
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);


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

void drawTiles(const Sprite &sprite, int x, int y)
{
    glUseProgram(sprite.shaderId);

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

void drawPlayer(const Sprite &sprite)
{
    glUseProgram(sprite.shaderId);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, sprite.textureId);
    glBindVertexArray(sprite.VAO);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sprite.translate);
    model = glm::scale(model, sprite.scale);

    glUniformMatrix4fv(glGetUniformLocation(sprite.shaderId, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform2i(glGetUniformLocation(sprite.shaderId, "sheetSize"), 6,4); 
    glUniform1i(glGetUniformLocation(sprite.shaderId, "frameIndex"),currentPlayerFrameIndex); // frame index for player

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    currentTime = glfwGetTime();
    if (isWalking && currentTime - lastTime >= 1.0 / FPS) {
        currentPlayerFrameIndex = (currentPlayerFrameIndex + 1) % 7;
        lastTime = currentTime;
    }
}

std::vector<std::string> split(const std::string& texto, char delimitador) {
    std::vector<std::string> partes;
    std::stringstream ss(texto);
    std::string item;

    while (std::getline(ss, item, delimitador)) {
        partes.push_back(item);
    }

    return partes;
}

std::vector<int> extrairValores(const std::string& linha) {
    std::vector<int> valores;
    std::vector<std::string> partes = split(linha, ' ');
    
    for (const std::string& parte : partes) {
        try {
            valores.push_back(std::stoi(parte));
        } catch (const std::invalid_argument&) {
        }
    }

    return valores;
}

void loadMap() {

    std::string arquivo = lerArquivoParaString("../assets/maps/map15x15.txt");
    std::cout << "Conteúdo do arquivo lido: " << arquivo << std::endl;
    std::vector<std::string> linhas = split(arquivo, '\n');
    std::vector<int> tamanhoMapa = extrairValores(linhas[0]);

    std::cout << "Tamanho do mapa: " << tamanhoMapa[0] << "x" << tamanhoMapa[1] << std::endl;
    mapWidth = tamanhoMapa[0];
    mapHeight = tamanhoMapa[1];

    mapData.resize(mapHeight, std::vector<int>(mapWidth, 0));
    for (int i = 1; i < linhas.size(); ++i)
    {
        std::vector<int> valoresLinha = extrairValores(linhas[i]);
        if (valoresLinha.size() == mapWidth)
        {
            for (int j = 0; j < mapWidth; ++j)
            {
                mapData[i - 1][j] = valoresLinha[j];
            }
        }
        else
        {
            std::cerr << "Erro: Linha " << i << " tem tamanho diferente do esperado." << std::endl;
        }
    }

}

int main()
{
    std::cout << "Trabalho GB - Benjamin Vichel, Leonardo Ramos e Lucas Kappes" << std::endl;
    initializeGlfw();
    setupGlConfiguration();

    GLFWwindow *window = makeWindow(WIDTH, HEIGHT, WINDOW_TITLE);
    glfwSetKeyCallback(window, keyCallback);

    GLuint tileShaderId = createTileShaderProgram();

    glm::mat4 orthProjection = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);
    glUseProgram(tileShaderId);
    glUniformMatrix4fv(glGetUniformLocation(tileShaderId, "projection"), 1, GL_FALSE, glm::value_ptr(orthProjection));
    std::cout << "Matriz de projeção definida!" << std::endl;

    GLuint playerShaderId = createPlayerShaderProgram();
    GLuint playerVAO = setupPlayerVAO();
    glUseProgram(playerShaderId);
    glUniformMatrix4fv(glGetUniformLocation(playerShaderId, "projection"), 1, GL_FALSE, glm::value_ptr(orthProjection));

    loadMap();

    Sprite player = Sprite();
    player.VAO = playerVAO;
    player.textureId = loadTexture("../assets/sprites/jorge.png");
    player.shaderId = playerShaderId;
    player.scale = glm::vec3(300.0f, 300.0f, 1.0f);
    player.translate = glm::vec3(200.0f, 200.0f, 0.0f);

    Sprite tileSprite = Sprite();
    tileSprite.VAO = setupTileVAO();
    tileSprite.textureId = loadTexture("../assets/sprites/tilesetIso.png");
    tileSprite.shaderId = tileShaderId;
    float tileW = WIDTH/mapWidth;
    float tileH = tileW / 2.0f; // altura = metade da largura
    tileSprite.scale = glm::vec3(tileW, tileH, 1.0f);

    std::vector<std::vector<Sprite>> map;

    float sobraAltura = WIDTH - (HEIGHT / 2.0f);
    for (int i = 0; i < mapHeight; ++i)
    {
        std::vector<Sprite> row;
        for (int j = 0; j < mapWidth; ++j)
        {
            Sprite tile = tileSprite;

            float x = (j - i) * (tileW / 2.0f);
            float y = (i + j) * (tileH / 2.0f);
            tile.translate = glm::vec3(x + WIDTH/2 - tileW/2, y + sobraAltura/4, 0.0f);
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
                drawTiles(map[i][j], i, j);
            }
        }
        drawPlayer(player);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}