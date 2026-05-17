#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <vector>
#include <iostream>
#include "shaderClass.h"

//RenderText batches the entire string into one draw call.
class TextRenderer {
public:
    struct GlyphInfo {
        float u0, v0, u1, v1;   //UV in atlas
        int   bx, by, bw, bh;   //bearing and bitmap size
        int   advance;
    };

    Shader    shader;
    GLuint    VAO, VBO;
    GLuint    atlas    = 0;
    GLuint    solidTex = 0;
    GlyphInfo glyphs[128] = {};
    int       lineHeight  = 0;

    TextRenderer(unsigned int screenW, unsigned int screenH,
                 const char* fontPath, unsigned int fontSize = 16)
        : shader("shaders/text.vert", "shaders/text.frag")
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) { std::cerr << "FreeType init failed\n"; return; }
        FT_Face face;
        if (FT_New_Face(ft, fontPath, 0, &face)) {
            std::cerr << "FreeType: cannot load " << fontPath << "\n";
            FT_Done_FreeType(ft); return;
        }
        FT_Set_Pixel_Sizes(face, 0, fontSize);

        // First pass: measure atlas dimensions
        int atlasW = 0, atlasH = 0;
        for (int c = 32; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
            atlasW += (int)face->glyph->bitmap.width + 1;
            atlasH  = std::max(atlasH, (int)face->glyph->bitmap.rows);
        }
        lineHeight = (int)(face->size->metrics.height >> 6);
        atlasH += 1;

        // Allocate one RED texture for the entire atlas
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &atlas);
        glBindTexture(GL_TEXTURE_2D, atlas);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasW, atlasH,
                     0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Second pass: copy each glyph bitmap into the atlas
        int xoff = 0;
        for (int c = 32; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
            FT_GlyphSlot g = face->glyph;
            if (g->bitmap.width > 0 && g->bitmap.rows > 0) {
                glTexSubImage2D(GL_TEXTURE_2D, 0, xoff, 0,
                    (int)g->bitmap.width, (int)g->bitmap.rows,
                    GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
            }
            glyphs[c] = {
                (float)xoff / atlasW,  0.0f,
                (float)(xoff + (int)g->bitmap.width) / atlasW,
                (float)g->bitmap.rows / atlasH,
                (int)g->bitmap_left, (int)g->bitmap_top,
                (int)g->bitmap.width, (int)g->bitmap.rows,
                (int)g->advance.x
            };
            xoff += (int)g->bitmap.width + 1;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  //restore default
        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        //1×1 white texture for DrawRect
        unsigned char white = 0xFF;
        glGenTextures(1, &solidTex);
        glBindTexture(GL_TEXTURE_2D, solidTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        //VAO/VBO — pre-sized for up to 256 chars per call
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * 256, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glm::mat4 proj = glm::ortho(0.0f, (float)screenW, 0.0f, (float)screenH);
        shader.Activate();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE,
                           glm::value_ptr(proj));
    }

    //Filled rectangle — call before RenderText to draw a background panel.
    void DrawRect(float x, float y, float w, float h, glm::vec3 color, float alpha)
    {
        shader.Activate();
        glUniform3f(glGetUniformLocation(shader.ID, "textColor"),
                    color.r, color.g, color.b);
        glUniform1f(glGetUniformLocation(shader.ID, "alphaScale"), alpha);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, solidTex);
        glBindVertexArray(VAO);

        float verts[6][4] = {
            { x,   y+h, 0,0 }, { x,   y,   0,0 }, { x+w, y,   0,0 },
            { x,   y+h, 0,0 }, { x+w, y,   0,0 }, { x+w, y+h, 0,0 }
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    //One texture bind + one draw call for the entire string.
    void RenderText(const std::string& text, float x, float y,
                    float scale, glm::vec3 color)
    {
        shader.Activate();
        glUniform3f(glGetUniformLocation(shader.ID, "textColor"),
                    color.r, color.g, color.b);
        glUniform1f(glGetUniformLocation(shader.ID, "alphaScale"), 1.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlas);
        glBindVertexArray(VAO);

        std::vector<float> verts;
        verts.reserve(text.size() * 24);  // 6 verts × 4 floats

        for (char c : text) {
            if (c < 32 || c > 127) { x += 8 * scale; continue; }
            const GlyphInfo& g = glyphs[(unsigned char)c];

            float xp = x + g.bx * scale;
            float yp = y - (g.bh - g.by) * scale;
            float w  = g.bw * scale;
            float h  = g.bh * scale;

            float q[6][4] = {
                { xp,     yp + h,  g.u0, g.v0 },
                { xp,     yp,      g.u0, g.v1 },
                { xp + w, yp,      g.u1, g.v1 },
                { xp,     yp + h,  g.u0, g.v0 },
                { xp + w, yp,      g.u1, g.v1 },
                { xp + w, yp + h,  g.u1, g.v0 }
            };
            for (auto& v : q)
                verts.insert(verts.end(), { v[0], v[1], v[2], v[3] });

            x += (g.advance >> 6) * scale;
        }

        if (!verts.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER,
                         (GLsizeiptr)(verts.size() * sizeof(float)),
                         verts.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 4));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};
