#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif


float top = 2.0, bottom = -2.0, right = 2.0, left = -2.0, near = 1.5, far = 10;
float camera_x = 4.0, camera_y = 3.0, camera_z = 3.0;
float target_x = 0.0, target_y = 0.0, target_z = 0.0;

int width = 800, height = 800;
float lastX = width / 2, lastY = height / 2; // Положение мыши по Y

bool flag = true;
bool isOrtho = true;

int typeOfLight = 1; // 0 - point, 1 - direction, 2 - projector

float radius = 0.5f;
int sectorCount = 36;
int stackCount = 18;

std::string configFileName = "config.txt";

struct Vector3
{
    float x;
    float y;
    float z;
};

void substruct(Vector3& v1, Vector3& v2, Vector3* res)
{
    res->x = v1.x - v2.x;
    res->y = v1.y - v2.y;
    res->z = v1.z - v2.z;
};

float dot(Vector3& v1, Vector3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void cross(Vector3& v1, Vector3& v2, Vector3* res)
{
    res->x = v1.y * v2.z - v1.z * v2.y;
    res->y = v1.z * v2.x - v1.x * v2.z;
    res->z = v1.x * v2.y - v1.y * v2.x;
};

float length(Vector3& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
};

void normalize(Vector3* v) {
    float len = length(*v);
    if (len == 0) return;
    v->x /= len;
    v->y /= len;
    v->z /= len;
}

glm::mat4 setProjection() {
    if (isOrtho) {
        return glm::mat4(
            2.0 / (right - left), 0.0f, 0.0f, 0.0f, // Столбец 1
            0.0f, 2.0 / (top - bottom), 0.0f, 0.0f, // Столбец 2
            0.0f, 0.0f, -2.0 / (far - near), 0.0f, // Столбец 3
            -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f  // Столбец 4
        );
    } else {
        return glm::mat4(
            2.0f * near / (right - left), 0.0f, 0.0f, 0.0f, // Столбец 1
            0.0f, 2.0f * near / (top - bottom), 0.0f, 0.0f, // Столбец 2
            (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1.0f,  // Столбец 3
            0.0f, 0.0f, -2.0f * far * near / (far - near), 0.0f // Столбец 4
        );
    }
}

GLfloat vertices[] = {
    // Позиции           // Нормали
    // Задняя грань
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    
    // Передняя грань
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     
     // Левая грань
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    
    // Правая грань
     0.5f,  0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    
    // Нижняя грань
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     
     // Верхняя грань
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
};


GLuint indices[] = {
    // Задняя грань
    0, 1, 2,
    1, 2, 3,

    // Передняя грань
    4, 5, 6,
    5, 6, 7,

    // Левая грань
    8, 9, 10,
    9, 10, 11,

    // Правая грань
    12, 13, 14,
    13, 14, 15,

    // Нижняя грань
    16, 17, 18,
    17, 18, 19,

    // Верхняя грань
    20, 21, 22,
    21, 22, 23
};

GLfloat plane_vertices[] = {
    // Позиции           // Нормали
    0.0f, -0.5f, +0.5f,  1.0f, 0.0f, 0.0f,
    0.0f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
    0.0f, +0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
    0.0f, +0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
    0.0f, +0.5f, +0.5f,  1.0f, 0.0f, 0.0f,
    0.0f, -0.5f, +0.5f,  1.0f, 0.0f, 0.0f,
    0.0f, -0.5f, +0.5f, -1.0f, 0.0f, 0.0f, 
    0.0f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
    0.0f, +0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
    0.0f, +0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
    0.0f, +0.5f, +0.5f, -1.0f, 0.0f, 0.0f,
    0.0f, -0.5f, +0.5f, -1.0f, 0.0f, 0.0f,
};

GLuint plane_indices[] = {
    0, 1, 2,
    3, 4, 5,
    6, 7, 8,
    9, 10, 11
};

std::vector<GLfloat> createSphere(float radius, int sectorCount, int stackCount) {
    std::vector<GLfloat> vertices;

    float x, y, z, xy; // координаты
    float nx, ny, nz; // нормали

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep; // от π/2 до -π/2
        xy = radius * cosf(stackAngle); // радиус на уровне стека
        z = radius * sinf(stackAngle); // высота

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep; // угол сектора

            // координаты
            x = xy * cosf(sectorAngle); // x
            y = xy * sinf(sectorAngle); // y
            // нормали
            nx = x / radius;
            ny = y / radius;
            nz = z / radius;

            // добавляем вершину
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    return vertices;
}

std::vector<GLuint> createSphereIndices(int sectorCount, int stackCount) {
    std::vector<GLuint> indices;

    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            GLuint first = (i * (sectorCount + 1)) + j;
            GLuint second = first + sectorCount + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    return indices;
}

std::vector<GLfloat> sphere_vertices = createSphere(radius, sectorCount, stackCount);
std::vector<GLuint> sphere_indices = createSphereIndices(sectorCount, stackCount);

struct Object {
    std::string type ;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Object(std::string t, glm::vec3 pos, glm::vec3 rot, glm::vec3 scl)
        : type(t), position(pos), rotation(rot), scale(scl) {}
};

std::vector<Object> objs;
std::vector<bool> isTaken;

void loadObjectsFromFile(const std::string& filename, std::vector<Object> & objects) {
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
    }

    std::string type;
    glm::vec3 position, rotation, scale;
    while (file >> type >> position.x >> position.y >> position.z >>
                rotation.x >> rotation.y >> rotation.z >>
                scale.x >> scale.y >> scale.z) {
            objects.emplace_back(type, position, rotation, scale);
    }
}

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position; // Позиция вершины
layout(location = 1) in vec3 normal;   // Нормаль вершины

out vec3 fragNormal; // Нормаль фрагмента
out vec3 fragPosition; // Позиция фрагмента

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fragPosition = vec3(model * vec4(position, 1.0)); // Преобразуем позицию вершины
    fragNormal = normalize(mat3(transpose(inverse(model))) * normal); // Преобразуем нормаль

    gl_Position = projection * view * model * vec4(position, 1.0); // Устанавливаем позицию
}
)";

const char* fragmentShaderSourceForPointLight = R"(
#version 330 core
in vec3 fragNormal;
in vec3 fragPosition;

uniform vec3 lightPos; // Позиция источника света
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 objectColor;

out vec4 color;

void main() {
    float ambientStrength = 0.1f;
    vec3 ambientColor = ambientStrength * lightColor; // фоновый свет

    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0); // диффузная составляющая
    vec3 diffuseColor = diff * lightColor; // диффузный свет

    // Спекулярная составляющая
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; // отражение (блик)

    vec3 result = (ambientColor + diffuseColor + specular) * objectColor;
    color = vec4(result, 1.0); // Устанавливаем цвет фрагмента
}
)";

const char* fragmentShaderSourceForDirectionLight = R"(
#version 330 core
in vec3 fragNormal;
in vec3 fragPosition;

uniform vec3 lightDir; // Направление источника света
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 objectColor;

out vec4 color;

void main() {

    float ambientStrength = 0.1f;
    vec3 ambientColor = ambientStrength * lightColor; // фоновый свет

    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, normalize(lightDir)), 0.0); // диффузная составляющая
    vec3 diffuseColor = diff * lightColor; // диффузный свет

    // Спекулярная составляющая
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-normalize(lightDir), norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; // отражение (блик)

    vec3 result = (ambientColor + diffuseColor + specular) * objectColor;
    color = vec4(result, 1.0); // Устанавливаем цвет фрагмента

    // float ambientStrength = 0.1f;
    // vec3 ambientColor = ambientStrength * lightColor; // фоновый свет

    // vec3 norm = normalize(fragNormal);
    // vec3 lightDirection = normalize(lightDir);
    // float diff = max(dot(norm, lightDirection), 0.0); // диффузная составляющая
    // vec3 diffuseColor = diff * lightColor; // диффузный свет

    // // Спекулярная составляющая
    // float specularStrength = 0.5f;
    // vec3 viewDir = normalize(viewPos - fragPosition);
    // vec3 reflectDir = reflect(-lightDirection, norm);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // vec3 specular = specularStrength * spec * lightColor; // отражение (блик)

    // vec3 result = (ambientColor + diffuseColor + specular) * objectColor;
    // color = vec4(result, 1.0); // Устанавливаем цвет фрагмента
}
)";

const char* fragmentShaderSourceForSpotlight = R"(
#version 330 core
in vec3 fragNormal;
in vec3 fragPosition;

uniform vec3 projDir; // Направление источника света
uniform vec3 lightPos;
uniform float cutOff;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

out vec4 color;

void main() {
    float ambientStrength = 0.1f;
    vec3 ambientColor = ambientStrength * lightColor; // фоновый свет

    // Нормализуем нормали и направление света
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);

    // Диффузная составляющая
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseColor = diff * lightColor; // диффузный свет

    // Спекулярная составляющая
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; // отражение (блик)

    vec3 projDirection = normalize(projDir);
    float theta = dot(lightDir, projDirection);
    vec3 result;
    if (theta > cutOff) result = (ambientColor + diffuseColor + specular) * objectColor;
    else result = ambientColor * objectColor;

    color = vec4(result, 1.0); // Устанавливаем цвет фрагмента
}
)";

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_W:
            for (int i = 0; i < objs.size(); ++i) {
                if (isTaken[i]) {
                    objs[i].position[1] += 0.5;
                }
            }
            camera_y += 0.5;
            target_y += 0.5;
            break;
        case GLFW_KEY_S:
            for (int i = 0; i < objs.size(); ++i) {
                if (isTaken[i]) {
                    objs[i].position[1] -= 0.5;
                }
            }
            camera_y -= 0.5;
            target_y -= 0.5;
            break;
        case GLFW_KEY_A:
            for (int i = 0; i < objs.size(); ++i) {
                if (isTaken[i]) {
                    objs[i].position[2] += 0.5;
                }
            }
            camera_z += 0.5;
            target_z += 0.5;
            break;
        case GLFW_KEY_D:
            for (int i = 0; i < objs.size(); ++i) {
                if (isTaken[i]) {
                    objs[i].position[2] -= 0.5;
                }
            }
            camera_z -= 0.5;
            target_z -= 0.5;
            break;
        case GLFW_KEY_Q:
            isOrtho = !isOrtho;
            break;
        case GLFW_KEY_Z:
            for (int i = 0; i < objs.size(); ++i) {
                float obj_x = objs[i].position[0], obj_y = objs[i].position[1], obj_z = objs[i].position[1];
                if (sqrt((camera_x - obj_x) * (camera_x - obj_x) + 
                         (camera_y - obj_y) * (camera_y - obj_y) +
                         (camera_z - obj_z) * (camera_z - obj_z)) < 1.9) {
                    isTaken[i] = !isTaken[i];
                }
            }
            break;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float sensitivity = 0.01f; // Чувствительность мыши
    float xoffset = sensitivity * (xpos - lastX);
    float yoffset = sensitivity * (lastY - ypos); // Изменение Y инвертировано, так как Y вниз
    target_x = xoffset;
    target_y = yoffset;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Изменяем масштаб в зависимости от прокрутки колеса мыши
    camera_x -= yoffset * 0.1f; // Увеличиваем или уменьшаем масштаб
    for (int i = 0; i < objs.size(); ++i) {
        if (isTaken[i]) {
            objs[i].position[0] -= yoffset * 0.1f;
        }
    }
}

int main() {
    loadObjectsFromFile(configFileName, objs);
    isTaken.resize(objs.size(), false);

    // Инициализация GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(width, height, "Flat Shading Cube", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetWindowPos(window, 100, 100);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glewInit();

     // Настройка ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Компиляция шейдеров
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceForPointLight, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Создание шейдерной программы
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLuint vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader2, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader2);

    glGetShaderiv(vertexShader2, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader2, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader2, 1, &fragmentShaderSourceForDirectionLight, NULL);
    glCompileShader(fragmentShader2);

    glGetShaderiv(fragmentShader2, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader2, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Создание шейдерной программы
    GLuint shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram2, vertexShader2);
    glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);

    GLuint vertexShader3 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader3, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader3);

    glGetShaderiv(vertexShader3, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader3, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader3 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader3, 1, &fragmentShaderSourceForSpotlight, NULL);
    glCompileShader(fragmentShader3);

    glGetShaderiv(fragmentShader3, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader3, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Создание шейдерной программы
    GLuint shaderProgram3 = glCreateProgram();
    glAttachShader(shaderProgram3, vertexShader3);
    glAttachShader(shaderProgram3, fragmentShader3);
    glLinkProgram(shaderProgram3);

    // Удаление шейдеров, так как они больше не нужны
    glDeleteShader(vertexShader);
    glDeleteShader(vertexShader2);
    glDeleteShader(vertexShader3);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShader2);
    glDeleteShader(fragmentShader3);

    GLuint VBO, VAO, EBO;

    // Настройка вершинного буфера и массива
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Обработка ввода
        glfwPollEvents();

        
        // Начало нового кадра ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Создание окна ImGui
        ImGui::Begin("Square");
        ImGui::Text("Push ESC to exit.\n");
        ImGui::Text("Amount of objects: %d.\n", objs.size());
        ImGui::Text("Distance between camera and object:");
        for (int i = 0; i < objs.size(); ++i) {
            float obj_x = objs[i].position[0], obj_y = objs[i].position[1], obj_z = objs[i].position[1];
            float distance = sqrt((camera_x - obj_x) * (camera_x - obj_x) + 
                         (camera_y - obj_y) * (camera_y - obj_y) +
                         (camera_z - obj_z) * (camera_z - obj_z));
            ImGui::Text("%d - %f", i, distance);
        }
        ImGui::Text("\nType of light");
        ImGui::RadioButton("Point", &typeOfLight, 0);
        ImGui::RadioButton("Direction", &typeOfLight, 1);
        ImGui::RadioButton("Projector", &typeOfLight, 2);
        ImGui::End();

        // Рендеринг
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Включение глубинного теста
        glEnable(GL_DEPTH_TEST);
        
        for (int i = 0; i < objs.size(); ++i) {
            int amountOfVertex;
            if (objs[i].type == "cube") {
                amountOfVertex = sizeof(indices) / sizeof(indices[0]);

                glBindVertexArray(VAO);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

                // Установка указателей для вершинного массива
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
                glEnableVertexAttribArray(1);

                glBindBuffer(GL_ARRAY_BUFFER, 0); // Отвязываем VBO
                glBindVertexArray(0); // Отвязываем VAO
            } else if (objs[i].type == "plane") {
                amountOfVertex = sizeof(plane_indices) / sizeof(plane_indices[0]);

                glBindVertexArray(VAO);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);

                // Установка указателей для вершинного массива
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
                glEnableVertexAttribArray(1);

                glBindBuffer(GL_ARRAY_BUFFER, 0); // Отвязываем VBO
                glBindVertexArray(0); // Отвязываем VAO
            } else {
                amountOfVertex = sphere_indices.size();

                glBindVertexArray(VAO);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sphere_vertices.size() * sizeof(GLfloat), sphere_vertices.data(), GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_indices.size() * sizeof(GLuint), sphere_indices.data(), GL_STATIC_DRAW);

                // Установка указателей для вершинного массива
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
                glEnableVertexAttribArray(1);

                glBindBuffer(GL_ARRAY_BUFFER, 0); // Отвязываем VBO
                glBindVertexArray(0); // Отвязываем VAO
            }

            // Использование шейдерной программы
            if (typeOfLight == 0) {
                glUseProgram(shaderProgram);
            } else if (typeOfLight == 1) {
                glUseProgram(shaderProgram2);
            } else if (typeOfLight == 2) {
                glUseProgram(shaderProgram3);
            }

            // Установка матриц
            Vector3 eye = { camera_x, camera_y, camera_z };
            Vector3 target = { target_x, target_y, target_z };
            Vector3 up = { 0.0, 1.0, 0.0 };

            Vector3 vz;
            Vector3 vx;
            Vector3 vy;
            substruct(eye, target, &vz);
            normalize(&vz);
            cross(up, vz, &vx);
            normalize(&vx);
            cross(vz, vx, &vy);
            normalize(&vy);

            // Установка матриц
            glm::mat4 model(
                objs[i].scale[0]*(cos(objs[i].rotation[1]) + cos(objs[i].rotation[2])) / 2.0, sin(objs[i].rotation[2]), -sin(objs[i].rotation[1]), 0,
                -sin(objs[i].rotation[2]), objs[i].scale[1]*(cos(objs[i].rotation[0]) + cos(objs[i].rotation[2])) / 2.0, sin(objs[i].rotation[0]), 0,
                sin(objs[i].rotation[1]), -sin(objs[i].rotation[0]), objs[i].scale[2]*(cos(objs[i].rotation[0]) + cos(objs[i].rotation[1])) / 2.0, 0,
                objs[i].position[0], objs[i].position[1], objs[i].position[2], 1
            );

            glm::mat4 view(
                vx.x, vy.x, vz.x, 0,
                vx.y, vy.y, vz.y, 0,
                vx.z, vy.z, vz.z, 0,
                -dot(vx, eye), -dot(vy, eye), -dot(vz, eye), 1
            );

            glm::mat4 projection = setProjection();

            // Передача матриц в шейдер
            GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
            GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
            GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

            if (typeOfLight == 0) {
                // Установка позиции источника света
                GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
                glUniform3f(lightPosLoc, 10.0f, 10.0f, 2.0f); // Позиция источника света

                GLuint colorLightLoc = glGetUniformLocation(shaderProgram, "lightColor");
                glUniform3f(colorLightLoc, 1.0f, 1.0f, 0.0f); // Цвет света

                GLuint cameraLoc = glGetUniformLocation(shaderProgram, "viewPos");
                glUniform3f(cameraLoc, camera_x, camera_y, camera_z); // Цвет света

                GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // Цвет объекта
            } else if (typeOfLight == 1) {
                // Установка позиции источника света
                GLuint lightDirLoc = glGetUniformLocation(shaderProgram2, "lightDir");
                glUniform3f(lightDirLoc, 2.0f, 1.0f, 1.0f); // Направление источника света

                GLuint colorLightLoc = glGetUniformLocation(shaderProgram2, "lightColor");
                glUniform3f(colorLightLoc, 1.0f, 1.0f, 0.0f); // Цвет света

                GLuint cameraLoc = glGetUniformLocation(shaderProgram2, "viewPos");
                glUniform3f(cameraLoc, camera_x, camera_y, camera_z); // Цвет света

                GLuint colorLoc = glGetUniformLocation(shaderProgram2, "objectColor");
                glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // Цвет объекта
            } else if (typeOfLight == 2) {
                GLuint lightDirLoc = glGetUniformLocation(shaderProgram3, "projDir");
                glUniform3f(lightDirLoc, 6.0f, 7.0f, -1.0f); // Напралвение источника света

                GLuint lightPosLoc = glGetUniformLocation(shaderProgram3, "lightPos");
                glUniform3f(lightPosLoc, 20.0f, 20.0f, 3.0f); // Позиция источника света

                float cutOffValue = cos(glm::radians(12.5f));
                GLuint cutAngleValue = glGetUniformLocation(shaderProgram3, "cutOff");
                glUniform1f(cutAngleValue, cutOffValue); // Позиция источника света

                GLuint colorLightLoc = glGetUniformLocation(shaderProgram3, "lightColor");
                glUniform3f(colorLightLoc, 1.0f, 1.0f, 0.0f); // Цвет света

                GLuint cameraLoc = glGetUniformLocation(shaderProgram3, "viewPos");
                glUniform3f(cameraLoc, camera_x, camera_y, camera_z); // Цвет света

                GLuint colorLoc = glGetUniformLocation(shaderProgram3, "objectColor");
                glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // Цвет объекта
            }

            // Рендеринг куба
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, amountOfVertex, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            // glfwSwapBuffers(window);

        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Обмен буферов и опрос событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgram2);
    glDeleteProgram(shaderProgram3);

    // Завершение работы ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Завершение работы GLFW
    glfwTerminate();
    return 0;
}
