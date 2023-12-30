#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadCubemap2(vector<std::string> faces);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
struct Pole {
    GLfloat x, z, y_start, y_end, u;
};

std::vector<Pole> generateCylinder(glm::vec3 center, GLfloat height, GLfloat radius, int num_points) {
    std::vector<Pole> poles;
    Pole pole;

    for (int i = 0; i < num_points; ++i) {
        GLfloat u = i / (GLfloat)num_points;

        pole.x = center.x + radius * cos(2 * 3.14159265358979323846 * u);
        pole.z = center.z + radius * sin(2 * 3.14159265358979323846 * u);
        pole.y_start = 0.0f;
        pole.y_end = height;
        pole.u = u;

        poles.push_back(pole);
    }

    return poles;
}
unsigned int cylinderVAO = 0;
unsigned int indexCountc;

void renderCylinder()
{
    if (cylinderVAO == 0)
    {
        glGenVertexArrays(1, &cylinderVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<unsigned int> indices;

        // Generate cylinder vertices
        glm::vec3 center(0.0f, 0.0f, 0.0f);
        GLfloat height = 0.1f;
        GLfloat radius = 0.35f;
        int num_points = 50;
        std::vector<Pole> cylinderVertices = generateCylinder(center, height, radius, num_points);

        // Convert cylinder vertices to positions array
        for (const auto& pole : cylinderVertices)
        {
            positions.push_back(glm::vec3(pole.x, pole.y_start, pole.z));
            positions.push_back(glm::vec3(pole.x, pole.y_end, pole.z));
        }

        // Generate cylinder indices
        for (unsigned int i = 0; i < cylinderVertices.size() * 2; ++i)
        {
            indices.push_back(i);
        }

        indexCountc = static_cast<unsigned int>(indices.size());
        glBindVertexArray(cylinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = 3 * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1); // Enable texture coordinate attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    }
    unsigned int cdTexture;
    glGenTextures(1, &cdTexture);
    glBindTexture(GL_TEXTURE_2D, cdTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width1, height1, nrChannels1;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data1 = stbi_load(FileSystem::getPath("resources/textures/mosaic.jpg").c_str(), &width1, &height1, &nrChannels1, 0);
    if (data1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data1);

    glBindVertexArray(cylinderVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cdTexture);
    glDrawElements(GL_TRIANGLE_STRIP, indexCountc, GL_UNSIGNED_INT, 0);
}
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                if (yPos >= 0.0) {
                    positions.push_back(glm::vec3(xPos, yPos, zPos));
                    uv.push_back(glm::vec2(xSegment, ySegment));
                    normals.push_back(glm::vec3(xPos, yPos, zPos));
                }
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}
void renderInsideOctagon(glm::mat4 view, glm::mat4 projection, glm::mat4 model) {
    float inside[]{
        // 1
     0.0f, 1.0f, -1.0f, 0.0f, 1.0f,
     0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
     0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
     0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
     0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
     0.75f, 0.0f, -0.75f, 1.0f, 0.0f,

     // 2
     0.75f, 1.0f, -0.75f, 0.0f, 1.0f,
     0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
     1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
     1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

     // 3
     1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
     1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
     0.75f, 0.0f, 0.75f, 1.0f, 0.0f,

     // 4
     0.75f, 1.0f, 0.75f, 0.0f, 1.0f,
     0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
     0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
     0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

     // 5
     0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     -0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
     0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     -0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
     -0.75f, 0.0f, 0.75f, 1.0f, 0.0f,

     // 6
     -0.75f, 1.0f, 0.75f, 0.0f, 1.0f,
     -0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
     -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     -0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
     -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

     // 7
     -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     -0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
     -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     -0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
     -0.75f, 0.0f, -0.75f, 1.0f, 0.0f,

     // 8
     -0.75f, 1.0f, -0.75f, 0.0f, 1.0f,
     -0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
     0.0f, 1.0f, -1.0f, 1.0f, 1.0f,
     -0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
     0.0f, 1.0f, -1.0f, 1.0f, 1.0f,
     0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
    };

    Shader rockShader("1.1.depth_testing.vs", "1.1.depth_testing.fs");

    rockShader.use();
    rockShader.setInt("texture1", 0); // First texture
  
    unsigned int inVBO, inVAO;
    glGenVertexArrays(1, &inVAO);
    glGenBuffers(1, &inVBO);
    glBindVertexArray(inVAO);
    glBindBuffer(GL_ARRAY_BUFFER, inVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(inside), inside, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    unsigned int domeTexture2 = loadTexture(FileSystem::getPath("resources/textures/in.jpg").c_str());

    rockShader.use();
    unsigned int modelLoc = glGetUniformLocation(rockShader.ID, "model");
    unsigned int viewLoc = glGetUniformLocation(rockShader.ID, "view");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    rockShader.setMat4("projection", projection);
    glBindVertexArray(inVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, domeTexture2);
    glDrawArrays(GL_TRIANGLES, 0, 48);

    glDeleteVertexArrays(1, &inVAO);
    glDeleteBuffers(1, &inVBO);
}

void renderOctagon(glm::mat4 view, glm::mat4 projection, glm::mat4 model) {
   
    float vertices[] = {
        // bottom
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.75f, 0.0f, 0.75f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.75f, 0.0f, 0.75f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.75f, 0.0f, -0.75f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.75f, 0.0f, -0.75f, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.75f, 0.0f, -0.75f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.75f, 0.0f, -0.75f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.75f, 0.0f, 0.75f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.75f, 0.0f, 0.75f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,

        // top
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.75f, 1.0f, 0.75f, 0.0f, 0.1f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.75f, 1.0f, 0.75f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.75f, 1.0f, -0.75f, 0.0f, 0.1f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.75f, 1.0f, -0.75f, 0.0f, 1.0f,
        0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.75f, 1.0f, -0.75f, 0.0f, 0.1f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.75f, 1.0f, -0.75f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.75f, 1.0f, 0.75f, 0.0f, 0.1f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.75f, 1.0f, 0.75f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f,

        // 1
        0.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
        0.75f, 0.0f, -0.75f, 1.0f, 0.0f,

        // 2
        0.75f, 1.0f, -0.75f, 0.0f, 1.0f,
        0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        // 3
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
        0.75f, 0.0f, 0.75f, 1.0f, 0.0f,

        // 4
        0.75f, 1.0f, 0.75f, 0.0f, 1.0f,
        0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

        // 5
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.75f, 1.0f, 0.75f, 1.0f, 1.0f,
        -0.75f, 0.0f, 0.75f, 1.0f, 0.0f,

        // 6
        -0.75f, 1.0f, 0.75f, 0.0f, 1.0f,
        -0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.75f, 0.0f, 0.75f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        // 7
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.75f, 1.0f, -0.75f, 1.0f, 1.0f,
        -0.75f, 0.0f, -0.75f, 1.0f, 0.0f,

        // 8
        -0.75f, 1.0f, -0.75f, 0.0f, 1.0f,
        -0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
        0.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        -0.75f, 0.0f, -0.75f, 0.0f, 0.0f,
        0.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 1.0f, 0.0f,


    };
   

    Shader rockShader("1.1.depth_testing.vs", "1.1.depth_testing.fs");

    rockShader.use();
    rockShader.setInt("texture1", 0); // First texture
    rockShader.setInt("texture2", 1); // Second texture


    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int domeTexture = loadTexture(FileSystem::getPath("resources/textures/dome1.png").c_str());
    rockShader.use();
    unsigned int modelLoc = glGetUniformLocation(rockShader.ID, "model");
    unsigned int viewLoc = glGetUniformLocation(rockShader.ID, "view");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    rockShader.setMat4("projection", projection);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, domeTexture);
    glDrawArrays(GL_TRIANGLES, 0, 96);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);


}



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shader("6.2.cubemaps.vs", "6.2.cubemaps.fs");
    Shader skyboxShader("6.2.skybox.vs", "6.2.skybox.fs");
    Shader ourShader("1.1.depth_testing.vs", "1.1.depth_testing.fs");
  //  Shader rockShader("C:\\Users\\Jeda\\Desktop\\LearnOpenGLTry\\src\\1.getting_started\\6.2.coordinate_systems_depth\\6.2.coordinate_systems.vs", "C:\\Users\\Jeda\\Desktop\\LearnOpenGLTry\\src\\1.getting_started\\6.2.coordinate_systems_depth\\6.2.coordinate_systems.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // normals
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    float leftRoad[] = {
6.0f, -1.0f, 10.0f, 2.0f, 0.0f,
-8.0f, -1.0f, 10.0f, 0.0f, 0.0f,
-8.0f, -1.0f, -10.0f, 0.0f, 10.0f,

6.0f, -1.0f, 10.0f, 2.0f, 0.0f,
-8.0f, -1.0f, -10.0f, 0.0f, 10.0f,
6.0f, -1.0f, -10.0f, 2.0f, 10.0f
    };
    float rightRoad[] = {
8.0f, -1.0f, 10.0f, 2.0f, 0.0f,
6.0f, -1.0f, 10.0f, 0.0f, 0.0f,
6.0f, -1.0f, -10.0f, 0.0f, 10.0f,

8.0f, -1.0f, 10.0f, 2.0f, 0.0f,
6.0f, -1.0f, -10.0f, 0.0f, 10.0f,
8.0f, -1.0f, -10.0f, 2.0f, 10.0f
    };
    float base[] = {
6.0f, -1.0f, 10.0f, 15.0f, 0.0f,
-6.0f, -1.0f, 10.0f, 0.0f, 0.0f,
-6.0f, -1.0f, -10.0f, 0.0f, 15.0f,

6.0f, -1.0f, 10.0f, 15.0f, 0.0f,
-6.0f, -1.0f, -10.0f, 0.0f, 15.0f,
6.0f, -1.0f, -10.0f, 15.0f, 15.0f
    };
    float grass[] = {
4.0f, -0.9999f, 6.0f, 15.0f, 0.0f,
-4.0f, -0.9999f, 6.0f, 0.0f, 0.0f,
-4.0f, -0.9999f, -5.0f, 0.0f, 15.0f,

4.0f, -0.9999f, 6.0f, 20.0f, 0.0f,
-4.0f, -0.9999f, -5.0f, 0.0f, 20.0f,
4.0f, -0.9999f, -5.0f, 20.0f, 20.0f
    };
    float yard[] = {
2.0f, -0.8f, 3.0f, 10.0f, 0.0f,
-2.0f, -0.8f, 3.0f, 0.0f, 0.0f,
-2.0f, -0.8f, -3.0f, 0.0f, 10.0f,

2.0f, -0.8f, 3.0f, 10.0f, 0.0f,
-2.0f, -0.8f, -3.0f, 0.0f, 10.0f,
2.0f, -0.8f, -3.0f, 10.0f, 10.0f
    };
    float backWall[] = {
6.0f, 0.5f, 10.0f, 5.0f, 0.0f,
-6.0f, 0.5f, 10.0f, 0.0f, 0.0f,
-6.0f, -1.0f, 10.0f, 0.0f, 1.0f,

6.0f, 0.5f, 10.0f, 5.0f, 0.0f,
-6.0f, -1.0f, 10.0f, 0.0f, 1.0f,
6.0f, -1.0f, 10.0f, 5.0f, 1.0f
    };
    float frontWall[] = {
6.0f, 0.5f, -10.0f, 5.0f, 0.0f,
-6.0f, 0.5f, -10.0f, 0.0f, 0.0f,
-6.0f, -1.0f, -10.0f, 0.0f, 1.0f,

6.0f, 0.5f, -10.0f, 5.0f, 0.0f,
-6.0f, -1.0f, -10.0f, 0.0f, 1.0f,
6.0f, -1.0f, -10.0f, 5.0f, 1.0f
    };
    float leftWall[] = {
-6.0f, 0.5f, 10.0f, 5.0f, 0.0f,
-6.0f, 0.5f, -10.0f, 0.0f, 0.0f,
-6.0f, -1.0f, -10.0f, 0.0f, 1.0f,

-6.0f, 0.5f, 10.0f, 5.0f, 0.0f,
-6.0f, -1.0f, -10.0f, 0.0f, 1.0f,
-6.0f, -1.0f, 10.0f, 5.0f, 1.0f
    };
    float rightWall[] = {
6.0f, 0.5f, 10.0f, 5.0f, 0.0f,
6.0f, 0.5f, -10.0f, 0.0f, 0.0f,
6.0f, -1.0f, -10.0f, 0.0f, 1.0f,

6.0f, 0.5f, 10.0f, 5.0f, 0.0f,
6.0f, -1.0f, -10.0f, 0.0f, 1.0f,
6.0f, -1.0f, 10.0f, 5.0f, 1.0f
    };
    float backWallYard[] = {
2.0f, 0.0f, 3.0f, 5.0f, 0.0f,
-2.0f, 0.0f, 3.0f, 0.0f, 0.0f,
-2.0f, -1.0f, 3.0f, 0.0f, 1.0f,

2.0f, 0.0f, 3.0f, 5.0f, 0.0f,
-2.0f, -1.0f, 3.0f, 0.0f, 1.0f,
2.0f, -1.0f, 3.0f, 5.0f, 1.0f
    };
    float frontWallYard[] = {
2.0f, 0.0f, -3.0f, 2.0f, 0.0f,
0.5f, 0.0f, -3.0f, 0.0f, 0.0f,
0.5f, -1.0f, -3.0f, 0.0f, 1.0f,

2.0f, 0.0f, -3.0f, 2.0f, 0.0f,
0.5f, -1.0f, -3.0f, 0.0f, 1.0f,
2.0f, -1.0f, -3.0f, 2.0f, 1.0f,

-0.5f, 0.0f, -3.0f, 2.f, 0.0f,
-2.0f, 0.0f, -3.0f, 0.0f, 0.0f,
-2.0f, -1.0f, -3.0f, 0.0f, 1.0f,

-0.5f, 0.0f, -3.0f, 2.0f, 0.0f,
-2.0f, -1.0f, -3.0f, 0.0f, 1.0f,
-0.5f, -1.0f, -3.0f, 2.0f, 1.0f

    };
    float leftWallYard[] = {
-2.0f, 0.0f, 3.0f, 5.0f, 0.0f,
-2.0f, 0.0f, -3.0f, 0.0f, 0.0f,
-2.0f, -1.0f, -3.0f, 0.0f, 1.0f,

-2.0f, 0.0f, 3.0f, 5.0f, 0.0f,
-2.0f, -1.0f, -3.0f, 0.0f, 1.0f,
-2.0f, -1.0f, 3.0f, 5.0f, 1.0f
    };
    float rightWallYard[] = {
2.0f, 0.0f, 3.0f, 5.0f, 0.0f,
2.0f, 0.0f, -3.0f, 0.0f, 0.0f,
2.0f, -1.0f, -3.0f, 0.0f, 1.0f,

2.0f, 0.0f, 3.0f, 5.0f, 0.0f,
2.0f, -1.0f, -3.0f, 0.0f, 1.0f,
2.0f, -1.0f, 3.0f, 5.0f, 1.0f
    };
    float gate[] = {
0.5f, 0.0f, -3.0f, 1.0f, 0.0f,
-0.5f, 0.0f, -3.0f, 0.0f, 0.0f,
-0.5f, -1.0f, -3.0f, 0.0f, 1.0f,

0.5f, 0.0f, -3.0f, 1.0f, 0.0f,
-0.5f, -1.0f, -3.0f, 0.0f, 1.0f,
0.5f, -1.0f, -3.0f, 1.0f, 1.0f
    };
    float boxVertices[] = {
        // bottom
        0.0f, 0.0f, 0.9f, 0.0f, 0.0f,
        -0.65f, 0.0f, 0.65f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.65f, 0.0f, 0.65f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.9f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.65f, 0.0f, -0.65f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.65f, 0.0f, -0.65f, 0.0f, 1.0f,
        0.0f, 0.0f, -0.9f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, -0.9f, 0.0f, 0.0f,
        0.65f, 0.0f, -0.65f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.65f, 0.0f, -0.65f, 0.0f, 1.0f,
        0.9f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.9f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.65f, 0.0f, 0.65f, 0.0f, 0.1f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.65f, 0.0f, 0.65f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.9f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f,

        // top
         0.0f, 0.9f, 0.9f, 0.0f, 0.0f,
        -0.65f, 0.9f, 0.65f, 0.0f, 0.1f,
        0.0f, 0.9f, 0.0f, 1.0f, 0.0f,
        -0.65f, 0.9f, 0.65f, 0.0f, 1.0f,
        -0.9f,0.9f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.9f, 0.0f, 1.0f, 1.0f,
        -0.9f, 0.9f, 0.0f, 0.0f, 0.0f,
        -0.65f, 0.9f, -0.65f, 0.0f, 0.1f,
        0.0f, 0.9f, 0.0f, 1.0f, 0.0f,
        -0.65f, 0.9f, -0.65f, 0.0f, 1.0f,
        0.0f, 0.9f, -0.9f, 1.0f, 0.0f,
        0.0f, 0.9f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.9f, -0.9f, 0.0f, 0.0f,
        0.65f, 0.9f, -0.65f, 0.0f, 0.1f,
        0.0f, 0.9f, 0.0f, 1.0f, 0.0f,
        0.65f, 0.9f, -0.65f, 0.0f, 1.0f,
        0.9f, 0.9f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.9f, 0.0f, 1.0f, 1.0f,
        0.9f, 0.9f, 0.0f, 0.0f, 0.0f,
        0.65f, 0.9f, 0.65f, 0.0f, 0.1f,
        0.0f, 0.9f, 0.0f, 1.0f, 0.0f,
        0.65f, 0.9f, 0.65f, 0.0f, 1.0f,
        0.0f, 0.9f, 0.9f, 1.0f, 0.0f,
        0.0f, 0.9f, 0.0f, 1.0f, 1.0f,

        // 1
        0.0f, 0.9f, -0.9f, 0.0f, 1.0f,
        0.0f, 0.0f, -0.9f, 0.0f, 0.0f,
        0.65f, 0.9f, -0.65f, 1.0f, 1.0f,
        0.0f, 0.0f, -0.9f, 0.0f, 0.0f,
        0.65f, 0.9f, -0.65f, 1.0f, 1.0f,
        0.65f, 0.0f, -0.65f, 1.0f, 0.0f,

        // 2
        0.65f, 0.9f, -0.65f, 0.0f, 1.0f,
        0.65f, 0.0f, -0.65f, 0.0f, 0.0f,
        0.9f, 0.9f, 0.0f, 1.0f, 1.0f,
        0.65f, 0.0f, -0.65f, 0.0f, 0.0f,
        0.9f, 0.9f, 0.0f, 1.0f, 1.0f,
        0.9f, 0.0f, 0.0f, 1.0f, 0.0f,

        // 3
        0.9f, 0.9f, 0.0f, 0.0f, 1.0f,
        0.9f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.65f, 0.9f, 0.65f, 1.0f, 1.0f,
        0.9f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.65f, 0.9f, 0.65f, 1.0f, 1.0f,
        0.65f, 0.0f, 0.65f, 1.0f, 0.0f,

        // 4
        0.65f, 0.9f, 0.65f, 0.0f, 1.0f,
        0.65f, 0.0f, 0.65f, 0.0f, 0.0f,
        0.0f, 0.9f, 0.9f, 1.0f, 1.0f,
        0.65f, 0.0f, 0.65f, 0.0f, 0.0f,
        0.0f, 0.9f, 0.9f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.9f, 1.0f, 0.0f,

        // 5
        0.0f, 0.9f, 0.9f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.9f, 0.0f, 0.0f,
        -0.65f, 0.9f, 0.65f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.9f, 0.0f, 0.0f,
        -0.65f, 0.9f, 0.65f, 1.0f, 1.0f,
        -0.65f, 0.0f, 0.65f, 1.0f, 0.0f,

        // 6
        -0.65f, 0.9f, 0.65f, 0.0f, 1.0f,
        -0.65f, 0.0f, 0.65f, 0.0f, 0.0f,
        -0.9f, 0.9f, 0.0f, 1.0f, 1.0f,
        -0.65f, 0.0f, 0.65f, 0.0f, 0.0f,
        -0.9f, 0.9f, 0.0f, 1.0f, 1.0f,
        -0.9f, 0.0f, 0.0f, 1.0f, 0.0f,

        // 7
        -0.9f, 0.9f, 0.0f, 0.0f, 1.0f,
        -0.9f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.65f, 0.9f, -0.65f, 1.0f, 1.0f,
        -0.9f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.65f, 0.9f, -0.65f, 1.0f, 1.0f,
        -0.65f, 0.0f, -0.65f, 1.0f, 0.0f,

        // 8
        -0.65f, 0.9f, -0.65f, 0.0f, 1.0f,
        -0.65f, 0.0f, -0.65f, 0.0f, 0.0f,
        0.0f, 0.9f, -0.9f, 1.0f, 1.0f,
        -0.65f, 0.0f, -0.65f, 0.0f, 0.0f,
        0.0f, 0.9f, -0.9f, 1.0f, 1.0f,
        0.0f, 0.0f, -0.9f, 1.0f, 0.0f,


    };
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
 /*   unsigned int boxVAO, boxVBO;
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);
    glBindVertexArray(boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), &boxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);*/
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(base), &base, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int grassVAO, grassVBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grass), &grass, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int yardVAO, yardVBO;
    glGenVertexArrays(1, &yardVAO);
    glGenBuffers(1, &yardVBO);
    glBindVertexArray(yardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, yardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(yard), &yard, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int backWallVAO, backWallVBO;
    glGenVertexArrays(1, &backWallVAO);
    glGenBuffers(1, &backWallVBO);
    glBindVertexArray(backWallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, backWallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backWall), &backWall, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int frontWallVAO, frontWallVBO;
    glGenVertexArrays(1, &frontWallVAO);
    glGenBuffers(1, &frontWallVBO);
    glBindVertexArray(frontWallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, frontWallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontWall), &frontWall, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int leftWallVAO, leftWallVBO;
    glGenVertexArrays(1, &leftWallVAO);
    glGenBuffers(1, &leftWallVBO);
    glBindVertexArray(leftWallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leftWallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftWall), &leftWall, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int rightWallVAO, rightWallVBO;
    glGenVertexArrays(1, &rightWallVAO);
    glGenBuffers(1, &rightWallVBO);
    glBindVertexArray(rightWallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rightWallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightWall), &rightWall, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int backWallYardVAO, backWallYardVBO;
    glGenVertexArrays(1, &backWallYardVAO);
    glGenBuffers(1, &backWallYardVBO);
    glBindVertexArray(backWallYardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, backWallYardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backWallYard), &backWallYard, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int frontWallYardVAO, frontWallYardVBO;
    glGenVertexArrays(1, &frontWallYardVAO);
    glGenBuffers(1, &frontWallYardVBO);
    glBindVertexArray(frontWallYardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, frontWallYardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontWallYard), &frontWallYard, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int leftWallYardVAO, leftWallYardVBO;
    glGenVertexArrays(1, &leftWallYardVAO);
    glGenBuffers(1, &leftWallYardVBO);
    glBindVertexArray(leftWallYardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leftWallYardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftWallYard), &leftWallYard, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int rightWallYardVAO, rightWallYardVBO;
    glGenVertexArrays(1, &rightWallYardVAO);
    glGenBuffers(1, &rightWallYardVBO);
    glBindVertexArray(rightWallYardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rightWallYardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightWallYard), &rightWallYard, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int gateVAO, gateVBO;
    glGenVertexArrays(1, &gateVAO);
    glGenBuffers(1, &gateVBO);
    glBindVertexArray(gateVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gateVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gate), &gate, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int leftRoadVAO, leftRoadVBO;
    glGenVertexArrays(1, &leftRoadVAO);
    glGenBuffers(1, &leftRoadVBO);
    glBindVertexArray(leftRoadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leftRoadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftRoad), &leftRoad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int rightRoadVAO, rightRoadVBO;
    glGenVertexArrays(1, &rightRoadVAO);
    glGenBuffers(1, &rightRoadVBO);
    glBindVertexArray(rightRoadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rightRoadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightRoad), &rightRoad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    // load textures
    // -------------
    vector<std::string> faces
    {
        FileSystem::getPath("resources/textures/skybox/right.png"),
        FileSystem::getPath("resources/textures/skybox/left.png"),
        FileSystem::getPath("resources/textures/skybox/top.png"),
        FileSystem::getPath("resources/textures/skybox/bottom.png"),
        FileSystem::getPath("resources/textures/skybox/front.png"),
        FileSystem::getPath("resources/textures/skybox/back.png"),
    };
  
    unsigned int cubemapTexture = loadCubemap(faces);

    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/sand.jpg").c_str());
    unsigned int grassTexture = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str());
    unsigned int yardTexture = loadTexture(FileSystem::getPath("resources/textures/yard.png").c_str());
    unsigned int wallTexture = loadTexture(FileSystem::getPath("resources/textures/wall.png").c_str());
    unsigned int yardWallTexture = loadTexture(FileSystem::getPath("resources/textures/yardWall.png").c_str());
    unsigned int gateTexture = loadTexture(FileSystem::getPath("resources/textures/gate.png").c_str());
    unsigned int goldTexture = loadTexture(FileSystem::getPath("resources/textures/gold.jpg").c_str());
    unsigned int roadTexture = loadTexture(FileSystem::getPath("resources/textures/road.jpg").c_str());

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("skybox", 0);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    ourShader.use();
    ourShader.setInt("texture1", 0);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw scene as normal
        //shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
       // view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(leftRoadVAO);
        glBindTexture(GL_TEXTURE_2D, roadTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(rightRoadVAO);
        glBindTexture(GL_TEXTURE_2D, roadTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(grassVAO);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();

        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(yardVAO);
        glBindTexture(GL_TEXTURE_2D, yardTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
       ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(backWallVAO);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(frontWallVAO);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(leftWallVAO);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(rightWallVAO);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(backWallYardVAO);
        glBindTexture(GL_TEXTURE_2D, yardWallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(frontWallYardVAO);
        glBindTexture(GL_TEXTURE_2D, yardWallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(leftWallYardVAO);
        glBindTexture(GL_TEXTURE_2D, yardWallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glBindVertexArray(rightWallYardVAO);
        glBindTexture(GL_TEXTURE_2D, yardWallTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glBindVertexArray(gateVAO);
        glBindTexture(GL_TEXTURE_2D, gateTexture);
        ourShader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        ourShader.use();
        unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        model = glm::scale(model, glm::vec3(0.35f, 0.35f, 0.35f));
       view = glm::translate(view, glm::vec3(0.0f, -0.2f, 0.0f));
       glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
       ourShader.setMat4("projection", projection);
        renderCylinder(); 
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(0);
        ourShader.use();
        modelLoc = glGetUniformLocation(ourShader.ID, "model");
        view = glm::translate(view, glm::vec3(0.0f, 0.05f, 0.0f));
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, goldTexture);
        renderSphere();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(0);
        ourShader.use();
        modelLoc = glGetUniformLocation(ourShader.ID, "model");
        view = glm::translate(camera.GetViewMatrix(), glm::vec3(0.0f, -0.79f, 0.0f));
         model = glm::scale(model, glm::vec3(1.7f, 1.7f, 1.7f));
        ourShader.setMat4("projection", projection);
        renderOctagon(view,projection,model);
        ourShader.setMat4("view", view);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(0);
       ourShader.use();
        modelLoc = glGetUniformLocation(ourShader.ID, "model");
        view = glm::translate(camera.GetViewMatrix(), glm::vec3(0.0f, -0.79f, 0.0f));
       model = glm::scale(model, glm::vec3(0.9f, 0.9, 0.9f));
        ourShader.setMat4("projection", projection);
        renderInsideOctagon(view, projection, model);
        ourShader.setMat4("view", view);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(0);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

       // glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

      


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &grassVAO);
    glDeleteVertexArrays(1, &rightRoadVAO);
    glDeleteVertexArrays(1, &leftRoadVAO);
    glDeleteVertexArrays(1, &gateVAO);
    glDeleteVertexArrays(1, &rightWallYardVAO);
    glDeleteVertexArrays(1, &leftWallYardVAO);
    glDeleteVertexArrays(1, &frontWallYardVAO);
    glDeleteVertexArrays(1, &backWallYardVAO);
    glDeleteVertexArrays(1, &backWallVAO);
    glDeleteVertexArrays(1, &frontWallVAO);
    glDeleteVertexArrays(1, &leftWallVAO);
    glDeleteVertexArrays(1, &rightWallVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &grassVBO);
    glDeleteBuffers(1, &rightRoadVBO);
    glDeleteBuffers(1, &leftRoadVBO);
    glDeleteBuffers(1, &gateVBO);
    glDeleteBuffers(1, &rightWallYardVBO);
    glDeleteBuffers(1, &leftWallYardVBO);
    glDeleteBuffers(1, &frontWallYardVBO);
    glDeleteBuffers(1, &backWallYardVBO);
    glDeleteBuffers(1, &backWallVBO);
    glDeleteBuffers(1, &frontWallVBO);
    glDeleteBuffers(1, &leftWallVBO);
    glDeleteBuffers(1, &rightWallVBO);



    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 3);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
