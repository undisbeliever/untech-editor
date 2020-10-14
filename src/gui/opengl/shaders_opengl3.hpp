/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui_impl_opengl3.h"
#include "shaders_opengl3.h"
#include "gui/windows/message-box.h"
#include <iostream>

namespace UnTech::Gui::Shaders {

static bool g_initialized = false;

static void CheckShader(GLuint handle, const char* name)
{
    GLint status = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

    GLint logLength = 0;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);

    const bool valid = (GLboolean)status != GL_FALSE;

    if (!valid) {
        std::cerr << "ERROR compiling " << name << std::endl;
    }

    if (logLength > 1) {
        std::vector<GLchar> message(logLength);
        glGetShaderInfoLog(handle, logLength, nullptr, message.data());

        if (valid) {
            std::cerr << "INFO LOG for " << name << std::endl;
        }
        std::cerr << message.data()
                  << std::endl;
    }
}

static void CheckProgram(GLuint handle, const char* name)
{
    GLint status = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);

    GLint logLength = 0;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);

    const bool valid = (GLboolean)status != GL_FALSE;

    if (!valid) {
        std::cerr << "ERROR linking " << name << std::endl;
    }

    if (logLength > 1) {
        std::vector<GLchar> message(logLength);
        glGetProgramInfoLog(handle, logLength, nullptr, message.data());

        if (valid) {
            std::cerr << "INFO LOG for " << name << std::endl;
        }
        std::cerr << message.data()
                  << std::endl;
    }
}

namespace Tilemap {

static const GLchar* tilemap_vertex_shader = R"glsl(
#version 130

uniform mat4 ProjMtx;

in vec2 Position;
in vec2 UV;

out vec2 Frag_UV;

void main()
{
    Frag_UV = UV;

    gl_Position = ProjMtx * vec4(Position.xy, 0.0f, 1.0f);
}
)glsl";

const GLchar* tilemap_fragment_shader = R"glsl(
#version 130

uniform sampler2D Tileset;
uniform usampler2D Map;
uniform vec2 MapSize;

in vec2 Frag_UV;

void main()
{
    int tile = int(texture(Map, Frag_UV.st).x);

    ivec2 tilePos = ivec2((tile & 0xf) * 16, tile & 0xf0);

    ivec2 subPos = ivec2(Frag_UV * MapSize) & 0xf;

    gl_FragColor = texelFetch(Tileset, tilePos + subPos, 0);
}
)glsl";

static GLuint g_shaderHandle = 0;
static GLint g_uniformProjMtx = 0;
static GLint g_uniformTileset = 0;
static GLint g_uniformMap = 0;
static GLint g_uniformMapSize = 0;
static GLint g_attribPosition = 0;
static GLint g_attribUV = 0;
static GLuint g_vertexBuffer = 0;

static void initialize()
{
    if (g_initialized) {
        return;
    }

    GLuint vertHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertHandle, 1, &tilemap_vertex_shader, 0);
    glCompileShader(vertHandle);
    CheckShader(vertHandle, "MetaTile vertex shader");

    GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragHandle, 1, &tilemap_fragment_shader, 0);
    glCompileShader(fragHandle);
    CheckShader(fragHandle, "MetaTile fragment shader");

    g_shaderHandle = glCreateProgram();
    glAttachShader(g_shaderHandle, vertHandle);
    glAttachShader(g_shaderHandle, fragHandle);
    glLinkProgram(g_shaderHandle);
    CheckProgram(g_shaderHandle, "MetaTile shader program");

    g_uniformProjMtx = glGetUniformLocation(g_shaderHandle, "ProjMtx");
    g_uniformTileset = glGetUniformLocation(g_shaderHandle, "Tileset");
    g_uniformMap = glGetUniformLocation(g_shaderHandle, "Map");
    g_uniformMapSize = glGetUniformLocation(g_shaderHandle, "MapSize");

    g_attribPosition = glGetAttribLocation(g_shaderHandle, "Position");
    g_attribUV = glGetAttribLocation(g_shaderHandle, "UV");

    glGenBuffers(1, &g_vertexBuffer);

    glDeleteShader(vertHandle);
    glDeleteShader(fragHandle);
}

static void cleanup()
{
    if (g_vertexBuffer) {
        glDeleteBuffers(1, &g_vertexBuffer);
        g_vertexBuffer = 0;
    }

    if (g_shaderHandle) {
        glDeleteProgram(g_shaderHandle);
        g_shaderHandle = 0;
    }
}

}

void initialize()
{
    if (g_initialized) {
        return;
    }

    Tilemap::initialize();
}

void cleanup()
{
    Tilemap::cleanup();
    g_initialized = false;
}

void drawMtTilemap(const ImDrawList*, const ImDrawCmd* pcmd)
{
    using namespace Gui::Shaders::Tilemap;

    const MtTilemapRenderData* data = static_cast<MtTilemapRenderData*>(pcmd->UserCallbackData);

    glUseProgram(g_shaderHandle);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, data->tilesetTextureId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data->mapTextureId);

    glUniform1i(g_uniformTileset, 1);
    glUniform1i(g_uniformMap, 0);

    glUniform2f(g_uniformMapSize, data->mapSize.x, data->mapSize.y);

    glUniformMatrix4fv(g_uniformProjMtx, 1, GL_FALSE, ImGui_Projection_Matrix.data());

    const float vertices[] = {
        // Position,        UV
        data->x1, data->y1, 0.0f, 0.0f, // top left
        data->x2, data->y1, 1.0f, 0.0f, // top right
        data->x2, data->y2, 1.0f, 1.0f, // bottom right

        data->x1, data->y1, 0.0f, 0.0f, // top left
        data->x1, data->y2, 0.0f, 1.0f, // bottom left
        data->x2, data->y2, 1.0f, 1.0f, // bottom right
    };

    glBindBuffer(GL_ARRAY_BUFFER, g_vertexBuffer);
    glEnableVertexAttribArray(g_attribPosition);
    glEnableVertexAttribArray(g_attribUV);

    glVertexAttribPointer(g_attribPosition, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(g_attribUV, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

}
