#pragma once

#include "GL/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include <QFile>
#include <QImage>
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
        GLint shapeTexUnitArray[8] = { 0 };
        GLuint shapeTextures[8] = { 0 };

        // Initialize to -1 or 0 — ensures well-defined values
        for (int i = 0; i < 8; ++i) shapeTexUnitArray[i] = -1;

        for (int i = 0; i < numShapes; i++) {
            if (shapeTextures[i] != 0) {
                GLint unit = i; // GL_TEXTURE0 + i
                shapeTexUnitArray[i] = unit;

                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, shapeTextures[i]);

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
        if (locArray >= 0) glUniform1iv(locArray, numShapes, shapeTexUnitArray);

        glUniform1i(glGetUniformLocation(shader, "numShapes"), numShapes);
    }

private:
    static GLuint loadTexture(const QString &filePath)
    {
        QImage img(filePath);
        if (img.isNull()) {
        qWarning("Failed to load texture: %s", qPrintable(filePath));
        return 0;
        }

        img = img.convertToFormat(QImage::Format_RGBA8888); // ensure 4 channels

        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
             GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);
        return texID;
    }

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
