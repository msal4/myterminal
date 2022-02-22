#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include <glad/glad.h>
#include FT_FREETYPE_H
#include <cglm/cglm.h>
#include <stdio.h>

#include "shader.h"

typedef unsigned int uint;

static const int scr_width = 800, scr_height = 600;

typedef uint uivec2[2];

typedef struct {
  uint texture_id;
  uivec2 size;
  uivec2 bearing;
  uint advance;
} character;

character chars[143859];

static void error_callback(int err, const char *desc);
static void render_text(uint shader, const char *text, float x, float y, float scale, vec3 color);
static int setup_font(const char *path, int width, int height, character chars[]);

uint vao, vbo;

int main(void) {
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    printf("[GLFW] Failed to initialize glfw\n");
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "TERM", NULL, NULL);
  if (!window) {
    printf("[GLFW] Failed to create window\n");
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("[GLAD] FAiled to initialize GLAD\n");
    return 1;
  }

  // OpenGL state
  // ------------
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  if (setup_font("../fonts/arial.ttf", 0, 48, chars))
    return EXIT_FAILURE;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  uint shader_program = create_shader("../gl/vertex.glsl", "../gl/fragment.glsl");
  if (!shader_program) {
    return EXIT_FAILURE;
  }

  mat4 proj;
  glm_ortho(0.0f, (float)scr_width, 0.0f, (float)scr_height, -1.0f, 1.0f, proj);
  glUseProgram(shader_program);
  glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, *proj);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    render_text(shader_program, "Hello World: I am some simple text rendered on the screen", 0.0f,
                (float)scr_height - 100.0f, 1.0f, (vec3){0.5f, 0.8f, 0.2f});

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();

  return EXIT_SUCCESS;
}

static void error_callback(int err, const char *desc) { fprintf(stderr, "[GLFW ERROR %i] %s\n", err, desc); }

static void render_cell(uint shader, cell c) {}

static void render_text(uint shader, const char *text, float x, float y, float scale, vec3 color) {
  glUseProgram(shader);
  glUniform3fv(glGetUniformLocation(shader, "text_color"), 1, color);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(vao);

  while (*text) {
    character ch = chars[*text];

    float xpos = x + (float)ch.bearing[0] * scale;
    float ypos = y - (float)(ch.size[1] - ch.bearing[1]) * scale;
    float w = (float)ch.size[0] * scale;
    float h = (float)ch.size[1] * scale;

    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},     {xpos + w, ypos, 1.0f, 1.0f},

        {xpos, ypos + h, 0.0f, 0.0f}, {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};

    glBindTexture(GL_TEXTURE_2D, ch.texture_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    x += (float)(ch.advance >> 6) * scale;

    text++;
  }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static int setup_font(const char *path, int width, int height, character chars[]) {
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    printf("[FREETYPE] Failed to initialize FreeType Library\n");
    return -1;
  }
  FT_Face face;
  if (FT_New_Face(ft, "../fonts/arial.ttf", 0, &face)) {
    printf("[FREETYPE] Failed to load font\n");
    return -1;
  }
  FT_Set_Pixel_Sizes(face, 0, 48);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte alignment restriction

  for (unsigned char c = 0; c < 128; c++) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      printf("[FREETYPE] Failed to load glyph: %c\n", c);
      continue;
    }

    uint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows, 0, GL_RED,
                 GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    character ch = {
        .texture_id = texture,
        .size = {face->glyph->bitmap.width, face->glyph->bitmap.rows},
        .bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top},
        .advance = face->glyph->advance.x,
    };
    chars[c] = ch;
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  // free resources
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return 0;
}