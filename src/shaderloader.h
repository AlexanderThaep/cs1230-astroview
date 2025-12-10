#pragma once

#include "GL/glew.h"

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
        float ka = rend.globalData.ka;
        float kd = rend.globalData.kd;
        float ks = rend.globalData.ks;

        glUniform1f(glGetUniformLocation(shader, "ka"), ka);
        glUniform1f(glGetUniformLocation(shader, "kd"), kd);
        glUniform1f(glGetUniformLocation(shader, "ks"), ks);

        int numLights = std::fmin(rend.lights.size(), 8);
        glUniform1i(glGetUniformLocation(shader, "numLights"), (GLint) numLights);

        for (int i = 0; i < numLights; i++) {
            auto& light = rend.lights[i];
            std::string prefix = "lights[" + std::to_string(i) + "].";

            glUniform1i(glGetUniformLocation(shader, (prefix + "type").c_str()), (int) light.type);
            glUniform3f(glGetUniformLocation(shader, (prefix + "color").c_str()), light.color.r, light.color.g, light.color.b);
            glUniform3f(glGetUniformLocation(shader, (prefix + "function").c_str()), light.function.x, light.function.y, light.function.z);
            glUniform4f(glGetUniformLocation(shader, (prefix + "pos").c_str()), light.pos.x, light.pos.y, light.pos.z, light.pos.w);
            glUniform4f(glGetUniformLocation(shader, (prefix + "dir").c_str()), light.dir.x, light.dir.y, light.dir.z, light.dir.w);
            glUniform1f(glGetUniformLocation(shader, (prefix + "penumbra").c_str()), light.penumbra);
            glUniform1f(glGetUniformLocation(shader, (prefix + "angle").c_str()), light.angle);
        }
    }

    static void passShaderValues(GLuint shader, RenderData &rend) {
        int numShapes = std::min((int) rend.shapes.size(), 16);

        // Now bind each texture that's available and set sampler mapping
        // Initialize to -1 or 0 — ensures well-defined values
        for (int i = 0; i < 8; ++i) rend.shapeTexUnitArray[i] = -1;

        for (int i = 0; i < numShapes; i++) {
            if (rend.shapeTextures[i] != 0) {
                GLint unit = i; // GL_TEXTURE0 + i
                rend.shapeTexUnitArray[i] = unit;

                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, rend.shapeTextures[i]);

                // Also set the sampler uniform "uShapeTex[i]" to that unit
                std::string name = "uShapeTex[" + std::to_string(i) + "]";
                GLint loc = glGetUniformLocation(shader, name.c_str());
                if (loc >= 0) glUniform1i(loc, unit);
            }

            const auto& shape = rend.shapes[i];
            std::string base = "shapes[" + std::to_string(i) + "].";

            // primitive type
            glUniform1i(glGetUniformLocation(shader, (base + "primitive").c_str()), (GLuint) shape.primitive.type);

            // inverse CTM
            glUniformMatrix4fv(glGetUniformLocation(shader, (base + "invCTM").c_str()), 1, GL_FALSE, &glm::inverse(shape.ctm)[0][0]);

            // material properties: ambient/diffuse/specular and shininess
            glUniform3f(glGetUniformLocation(shader, (base + "ambient").c_str()),shape.primitive.material.cAmbient.r, shape.primitive.material.cAmbient.g, shape.primitive.material.cAmbient.b);
            glUniform3f(glGetUniformLocation(shader, (base + "diffuse").c_str()),shape.primitive.material.cDiffuse.r, shape.primitive.material.cDiffuse.g, shape.primitive.material.cDiffuse.b);
            glUniform3f(glGetUniformLocation(shader, (base + "specular").c_str()),shape.primitive.material.cSpecular.r, shape.primitive.material.cSpecular.g, shape.primitive.material.cSpecular.b);
            glUniform1f(glGetUniformLocation(shader, (base + "shininess").c_str()),shape.primitive.material.shininess);
            glUniform1f(glGetUniformLocation(shader, (base + "blend").c_str()),shape.primitive.material.blend);
            glUniform1i(glGetUniformLocation(shader, (base + "hasTexture").c_str()),shape.hasTexture ? 1 : 0);
        }

        GLint locArray = glGetUniformLocation(shader, "uShapeTexUnits");
        if (locArray >= 0) glUniform1iv(locArray, numShapes, rend.shapeTexUnitArray);

        glUniform1i(glGetUniformLocation(shader, "numShapes"), numShapes);

        glUniform1i(glGetUniformLocation(shader, "hasBH"), rend.hasBH ? 1 : 0);
        glUniform3f(glGetUniformLocation(shader, "bh_pos"), rend.bh_pos.x, rend.bh_pos.y, rend.bh_pos.z);
        glUniform1f(glGetUniformLocation(shader, "bh_r"), rend.bh_r);
    }

    static void passCameraValues(GLuint shader, Camera &cam) {
        //Upload all other informatio: projection + view matrices, and camera pos
        GLuint locView = glGetUniformLocation(shader, "uView"); //view matrix
        glUniformMatrix4fv(locView, 1, GL_FALSE, &cam.getViewMatrix()[0][0]);
        GLuint locProj = glGetUniformLocation(shader, "uProj"); //projection matrix
        glUniformMatrix4fv(locProj, 1, GL_FALSE, &cam.getProjectionMatrix()[0][0]);
        glm::vec3 camPos = cam.getPosition(); //camera pos
        glUniform3f(glGetUniformLocation(shader, "uCameraPos"), camPos.x, camPos.y, camPos.z);

        GLuint locInvView = glGetUniformLocation(shader, "uInvView");
        glUniformMatrix4fv(locInvView, 1, GL_FALSE, &cam.getInverseViewMatrix()[0][0]);
        GLuint locInvProj = glGetUniformLocation(shader, "uInvProj");
        glUniformMatrix4fv(locInvProj, 1, GL_FALSE, &cam.getInverseProjectionMatrix()[0][0]);
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
