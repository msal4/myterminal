//
// Created by Mohammed Salman on 05/12/2021.
//

#include "shader.h"

#include <glad/glad.h>
#include <stdio.h>
#include <string.h>

static int read_entire_file(const char *path, char *out) {
  FILE *f = fopen(path, "r");
  if (!f) {
    perror("[FILE] Failed to open file");
    return -1;
  }

  char buf[1024];

  while (fgets(buf, sizeof(buf), f)) {
    strcat(out, buf);
  }

  fclose(f);

  return 0;
}

static unsigned int create_and_compile_shader(const char *path, int shader_type) {
  char shader_src[1024] = {};
  if (read_entire_file(path, shader_src) == -1) {
    return 0;
  }
  const char *shader_src_ptr = shader_src;

  unsigned int shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &shader_src_ptr, NULL);
  glCompileShader(shader);

  int status;
  char infoLog[1024];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (!status) {
    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
    printf("[create_and_compile_shader(\"%s\", %d)] Compiling shader: %s\n", path, shader_type, infoLog);
    return 0;
  }

  return shader;
}

// Create a shader program from supplied shaders. After linking the program, shaders are deleted.
static unsigned int create_and_link_program(unsigned int shaders[], int num_shaders) {
  unsigned int shader_program = glCreateProgram();
  if (!shader_program) {
    printf("[create_and_link_program(...)] Failed to create shader program\n");
    return 0;
  }

  for (int i = 0; i < num_shaders; i++) {
    glAttachShader(shader_program, shaders[i]);
  }

  glLinkProgram(shader_program);
  int status;
  char infoLog[1024];
  glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
  if (!status) {
    glGetProgramInfoLog(shader_program, sizeof(infoLog), NULL, infoLog);
    printf("[GL] Linking program: %s\n", infoLog);
    return 0;
  }

  for (int i = 0; i < num_shaders; i++) {
    glDeleteShader(shaders[i]);
  }

  return shader_program;
}

unsigned int create_shader(const char *vertex_path, const char *fragment_path) {
  const unsigned int vertex_shader = create_and_compile_shader(vertex_path, GL_VERTEX_SHADER);
  if (!vertex_shader)
    return 0;
  const unsigned int fragment_shader = create_and_compile_shader(fragment_path, GL_FRAGMENT_SHADER);
  if (!fragment_shader)
    return 0;

  return create_and_link_program((unsigned int[]){vertex_shader, fragment_shader}, 2);
}