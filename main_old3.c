//
// Created by Mohammed Salman on 03/12/2021.
//

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdio.h>

#include "shader.h"
#include "stb_image.h"

typedef unsigned int uint;

// function prototypes
static void error_callback(int err, const char *desc);
static void close_callback();
static void process_input(GLFWwindow *window);
static void window_resize_callback(GLFWwindow *window, int width, int height);

typedef struct {
  struct {
    int s, t;
  } wrap;
  struct {
    int min, mag;
  } filter;
  float border_color[4];
} create_texture_options;

static uint create_texture(const create_texture_options *options);
static uint load_texture(const char *path, uint format, int flip);

static vec3 camera_pos = {0.0f, 0.0f, 3.0f};
static vec3 camera_front = {0.0f, 0.0f, -1.0f};
static vec3 camera_up = {0.0f, 1.0f, 0.0f};

int main(void) {
  // ------------------------------------------------------------
  // Initialize GLFW
  // ------------------------------------------------------------
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(800, 600, "Terminal", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwSetWindowCloseCallback(window, close_callback);

  glfwMakeContextCurrent(window);

  if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
    printf("[GLAD] Failed to initialize GLAD\n");
    return -1;
  }

  glEnable(GL_DEPTH_TEST);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  glfwSetFramebufferSizeCallback(window, window_resize_callback);

  // ------------------------------------------------------------
  // Create vertex buffer
  // ------------------------------------------------------------
  float vertices[] = {
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
    0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

    -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
    0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};
  uint vbo, vao;
  glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  const uint shader_program = create_shader("../gl/test_vs.glsl", "../gl/test_fs.glsl");
  if (!shader_program)
    return -1;

  uint texture1 = create_texture(&(create_texture_options){.wrap = {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
      .filter = {GL_NEAREST, GL_LINEAR},
      .border_color = {1.0f, 0.0f, 0.0f, 1.0f}});
  if (!texture1)
    return -1;

  if (!load_texture("../images/wooden_container.jpg", GL_RGB, 0))
    return -1;

  uint texture2 = create_texture(NULL);
  if (!load_texture("../images/awesomeface.png", GL_RGBA, 1))
    return -1;

  glUseProgram(shader_program);
  glUniform1i(glGetUniformLocation(shader_program, "tex1"), 0);
  glUniform1i(glGetUniformLocation(shader_program, "tex2"), 1);

  mat4 proj = GLM_MAT4_IDENTITY;
  glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, proj);

  const int view_loc = glGetUniformLocation(shader_program, "view");
  const int proj_loc = glGetUniformLocation(shader_program, "projection");
  if (proj_loc != -1) {
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, *proj);
  }

  vec3 cube_positions[] = {
    { 0.0f,  0.0f,  0.0f},
    { 2.0f,  5.0f, -15.0f},
    {-1.5f, -2.2f, -2.5f},
    {-3.8f, -2.0f, -12.3f},
    { 2.4f, -0.4f, -3.5f},
    {-1.7f,  3.0f, -7.5f},
    { 1.3f, -2.0f, -2.5f},
    { 1.5f,  2.0f, -2.5f},
    { 1.5f,  0.2f, -1.5f},
    {-1.3f,  1.0f, -1.5f},
  };

  int model_loc = glGetUniformLocation(shader_program, "model");

  vec3 camera_pos = { 0.0f, 0.0f, 3.0f };
  vec3 camera_target = { 0.0f, 0.0f, 0.0f }; // GLM_VEC3_ZERO
  vec3 camera_direction;
  glm_vec3_sub(camera_direction, camera_pos, camera_target);
  glm_normalize(camera_direction);
  printf("camera_direction => x: %f, y: %f, z: %f\n", camera_direction[0], camera_direction[1], camera_direction[2]);

  vec3 up = { 0.0f, 1.0f, 0.0f };
  vec3 camera_right;
  glm_vec3_cross(camera_right, up, camera_direction);
  glm_normalize(camera_right);
  printf("camera_right => x: %f, y: %f, z: %f\n", camera_right[0], camera_right[1], camera_right[2]);

  vec3 camera_up;
  glm_vec3_cross(camera_up, camera_direction, camera_right);
  glm_normalize(camera_up);
  printf("camera_up => x: %f, y: %f, z: %f\n", camera_up[0], camera_up[1], camera_up[2]);

  while (!glfwWindowShouldClose(window)) {
    process_input(window);

    glClearColor(.1f, .1f, .1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glUseProgram(shader_program);

    const float radius = 10.0f;
    const float cam_x = sin(glfwGetTime()) * radius;
    const float cam_z = cos(glfwGetTime()) * radius;

    vec3 camera_dir;
    glm_vec3_add(camera_pos, camera_front, camera_dir);

    mat4 view;
    glm_lookat(camera_pos, camera_dir, camera_up, view);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, *view);

    glBindVertexArray(vao);
    for (uint i = 0; i < 10; i++) {
      mat4 model = GLM_MAT4_IDENTITY;
      glm_translate(model, cube_positions[i]);
      glm_rotate(model, glm_rad(50.0f) * (float)(i + 1), (vec3){0.5f, 1.0f, 0.0f});
      glUniformMatrix4fv(model_loc, 1, GL_FALSE, *model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteProgram(shader_program);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  glfwTerminate();

  return 0;
}

static void error_callback(int err, const char *desc) { fprintf(stderr, "[GLFW ERROR] %s\n", desc); }

static void close_callback() { printf("[GLFW] Closing\n"); }

static void process_input(GLFWwindow *window) {
  vec3 vel;
  const float camera_speed = 500000.000f;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    glm_vec3_scale(camera_front, camera_speed, vel);
    glm_vec3_add(camera_pos, vel, camera_pos);
    printf("W: camera_pos: {%f, %f, %f}\n", camera_pos[0], camera_pos[1], camera_pos[2]);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    glm_vec3_scale(camera_front, camera_speed, vel);
    glm_vec3_sub(camera_pos, vel, camera_pos);
    printf("S: camera_pos: {%f, %f, %f}\n", camera_pos[0], camera_pos[1], camera_pos[2]);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    glm_vec3_mul(camera_front, camera_up, vel);
    glm_normalize(vel);
    glm_vec3_scale(vel, camera_speed, vel);
    glm_vec3_sub(camera_pos, vel, camera_pos);
    printf("A: camera_pos: {%f, %f, %f}\n", camera_pos[0], camera_pos[1], camera_pos[2]);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    glm_vec3_mul(camera_front, camera_up, vel);
    glm_normalize(vel);
    glm_vec3_scale(vel, camera_speed, vel);
    glm_vec3_add(camera_pos, vel, camera_pos);
    printf("D: camera_pos: {%f, %f, %f}\n", camera_pos[0], camera_pos[1], camera_pos[2]);
  }
}

static void window_resize_callback(GLFWwindow *window, int width, int height) {
  printf("[GLFW] Resized to %dx%d\n", width, height);
  glViewport(0, 0, width, height);
}

static uint load_texture(const char *path, uint format, int flip) {
  stbi_set_flip_vertically_on_load((int)flip);
  int image_width, image_height, nr_channels;
  unsigned char *image_data = stbi_load(path, &image_width, &image_height, &nr_channels, 0);
  if (!image_data) {
    printf("[load_texture(\"%s\")] Failed to load texture\n", path);
    return 0;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(image_data);

  return 1;
}

static uint create_texture(const create_texture_options *options) {
  create_texture_options opts = options ? *options
    : (create_texture_options){
      .wrap = {GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT},
        .filter = {GL_NEAREST, GL_LINEAR},
        .border_color = {0.0f, 0.0f, 0.0f, 1.0f},
    };

  uint texture;
  glGenTextures(1, &texture);
  if (!texture) {
    printf("[create_texture(...)] Failed to create texture\n");
    return 0;
  }
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, opts.wrap.s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, opts.wrap.t);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, opts.border_color);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, opts.filter.min);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, opts.filter.mag);

  return texture;
}
