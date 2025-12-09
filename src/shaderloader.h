#pragma once

#include "GL/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include <QFile>
#include <QTextStream>
#include <camera/camera.h>
#include <utils/sceneparser.h>

class ShaderLoader
{
public:
    static GLuint createShaderProgram(const char *vertex_file_path, const char *fragment_file_path)
    {
        // Create and compile the shaders.
        GLuint vertexShaderID = createShader(GL_VERTEX_SHADER, vertex_file_path);
        GLuint fragmentShaderID = createShader(GL_FRAGMENT_SHADER, fragment_file_path);
        // Link the shader program.
        GLuint programID = glCreateProgram();
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);
        glLinkProgram(programID);
        // Print the info log if error
        GLint status;
        glGetProgramiv(programID, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            GLint length;
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);
            std::string log(length, '\0');
            glGetProgramInfoLog(programID, length, nullptr, &log[0]);
            glDeleteProgram(programID);
            throw std::runtime_error(log);
        }
        // Shaders no longer necessary, stored in program
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return programID;
    }

    static void passLightValues(GLuint shader, RenderData &rend) {
        GLuint loc;
        for (int i = 0; i < rend.lights.size(); i++) {
            SceneLightData l = rend.lights[i];
            std::string uniformName = "lights[" + std::to_string(i) + "]";

            loc = glGetUniformLocation(shader, (uniformName + ".pos").c_str());
            glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(l.pos)));

            loc = glGetUniformLocation(shader, (uniformName + ".dir").c_str());
            glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(l.dir)));

            loc = glGetUniformLocation(shader, (uniformName + ".color").c_str());
            glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(l.color)));

            loc = glGetUniformLocation(shader, (uniformName + ".function").c_str());
            glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(l.function)));

            loc = glGetUniformLocation(shader, (uniformName + ".penumbra").c_str());
            glUniform1f(loc, l.penumbra);

            loc = glGetUniformLocation(shader, (uniformName + ".angle").c_str());
            glUniform1f(loc, l.angle);

            loc = glGetUniformLocation(shader, (uniformName + ".type").c_str());
            switch (rend.lights[i].type) {
                case LightType::LIGHT_SPOT:
                    glUniform1i(loc, 2);
                    break;
                case LightType::LIGHT_DIRECTIONAL:
                    glUniform1i(loc, 1);
                    break;
                case LightType::LIGHT_POINT:
                    glUniform1i(loc, 0);
                    break;
            }

        }
        loc = glGetUniformLocation(shader, "lightCount");
        glUniform1i(loc, rend.lights.size());
    }

    static void passShaderValues(GLuint shader, RenderShapeData &o, RenderData &rend, Camera *cam) {
        GLuint loc;
        // Task 6: pass in m_model as a uniform into the shader program
        loc = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(o.ctm));

        loc = glGetUniformLocation(shader, "inv_t_model");
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(o.invCTMT));

        // Task 7: pass in m_view and m_proj
        loc = glGetUniformLocation(shader, "view");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(cam->getViewMatrix()));

        loc = glGetUniformLocation(shader, "proj");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(cam->getProjectionMatrix()));

        loc = glGetUniformLocation(shader, "cam");
        glUniform3fv(loc, 1, glm::value_ptr(cam->getPosition()));

        // Task 12: pass m_ka into the fragment shader as a uniform
        loc = glGetUniformLocation(shader, "ka");
        glUniform1f(loc, rend.globalData.ka);

        // Task 13: pass light position and m_kd into the fragment shader as a uniform
        loc = glGetUniformLocation(shader, "kd");
        glUniform1f(loc, rend.globalData.kd);

        // Task 14: pass shininess, m_ks, and world-space camera position
        loc = glGetUniformLocation(shader, "ks");
        glUniform1f(loc, rend.globalData.ks);

        loc = glGetUniformLocation(shader, "shininess");
        glUniform1f(loc, o.primitive.material.shininess);

        loc = glGetUniformLocation(shader, "diffuse");
        glUniform4fv(loc, 1, glm::value_ptr(o.primitive.material.cDiffuse));

        loc = glGetUniformLocation(shader, "ambient");
        glUniform4fv(loc, 1, glm::value_ptr(o.primitive.material.cAmbient));

        loc = glGetUniformLocation(shader, "specular");
        glUniform4fv(loc, 1, glm::value_ptr(o.primitive.material.cSpecular));
    }

private:
    static GLuint createShader(GLenum shaderType, const char *filepath)
    {
        GLuint shaderID = glCreateShader(shaderType);
        // Read shader file.
        std::string code;
        QString filepathStr = QString(filepath);
        QFile file(filepathStr);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            code = stream.readAll().toStdString();
        } else {
            throw std::runtime_error(std::string("Failed to open shader: ") + filepath);
        }
        // Compile shader code.
        const char *codePtr = code.c_str();
        glShaderSource(shaderID, 1, &codePtr, nullptr); // Assumes code is null terminated
        glCompileShader(shaderID);
        // Print info log if shader fails to compile.
        GLint status;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint length;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
            std::string log(length, '\0');
            glGetShaderInfoLog(shaderID, length, nullptr, &log[0]);
            glDeleteShader(shaderID);
            throw std::runtime_error(log);
        }
        return shaderID;
    }
};
