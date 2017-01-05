#include <check.h>
#include <stdlib.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

const char *vertex_shader =
    "#version 410\n"
    "layout (location = 0) in vec2 v;"
    "void main() {"
    "   gl_Position = vec4(v, 0.0, 1.0);"
    "}";

const char *fragment_shader =
    "#version 410\n"
    "layout (location = 0) out vec4 frag_color;"
    "void main() {"
    "   frag_color = vec4(1.0, 1.0, 1.0, 1.0);" // All white color
    "}";

const float triangle[] = {
    0.0f,  0.5f,
    0.5f, -0.5f,
   -0.5f, -0.5f
};

START_TEST(we_can_use_a_shader_program_and_issue_a_draw_call)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint framebuffer_name = 0;
    glGenFramebuffers(1, &framebuffer_name);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_name);

    GLuint width = 512;
    GLuint height = 512;

    GLuint color_renderbuffer;
    glGenRenderbuffers(1, &color_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_renderbuffer);

    GLuint depth_renderbuffer;
    glGenRenderbuffers(1, &depth_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

    GLenum draw_buffers = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &draw_buffers);

    GLenum err1 = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glBindVertexArray(vao);
    glUseProgram(shader_program);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(0);

    GLubyte pixels[4] = { };
    glReadPixels(256, 256, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels); // read the middle pixel which should be white

    glDeleteProgram(shader_program);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &framebuffer_name);

    GLenum err2 = glGetError();

    CGLDestroyContext(context);

    ck_assert_int_eq(err1, GL_FRAMEBUFFER_COMPLETE);
    ck_assert_int_eq(err2, GL_NO_ERROR);
    ck_assert_int_eq(pixels[0], 0xFF);
    ck_assert_int_eq(pixels[1], 0xFF);
    ck_assert_int_eq(pixels[2], 0xFF);
    ck_assert_int_eq(pixels[3], 0xFF);
}
END_TEST

START_TEST(we_can_read_from_a_fbo_with_glReadPixels)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint framebuffer_name = 0;
    glGenFramebuffers(1, &framebuffer_name);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_name);

    GLuint rendered_texture;
    glGenTextures(1, &rendered_texture);
    glBindTexture(GL_TEXTURE_2D, rendered_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendered_texture, 0);

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers); // "1" is the size of DrawBuffers

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    GLubyte pixels[4] = { };
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    GLenum err = glGetError();
    CGLDestroyContext(context);

    ck_assert_int_eq(err, GL_NO_ERROR);
    ck_assert_int_eq(pixels[0], 0xFF);
    ck_assert_int_eq(pixels[1], 0xFF);
    ck_assert_int_eq(pixels[2], 0xFF);
    ck_assert_int_eq(pixels[3], 0xFF);
}
END_TEST

START_TEST(we_can_bind_a_buffer_to_the_transform_feedback_target)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint buffer;

    glGenBuffers(1, &buffer);
    GLenum err1 = glGetError();

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffer);
    GLenum err2 = glGetError();

    glDeleteBuffers(1, &buffer);
    GLenum err3 = glGetError();

    CGLDestroyContext(context);

    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);
    ck_assert_int_eq(err3, GL_NO_ERROR);
}
END_TEST

START_TEST(we_can_create_a_vertex_array_object)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint vao;

    glGenVertexArrays(1, &vao);
    GLenum err1 = glGetError();
    glBindVertexArray(vao);
    GLenum err2 = glGetError();
    glEnableVertexAttribArray(0);
    GLenum err3 = glGetError();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    GLenum err4 = glGetError();
    glDeleteVertexArrays(1, &vao);
    GLenum err5 = glGetError();

    CGLDestroyContext(context);

    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);
    ck_assert_int_eq(err3, GL_NO_ERROR);
    ck_assert_int_eq(err4, GL_NO_ERROR);
    ck_assert_int_eq(err5, GL_NO_ERROR);
}
END_TEST

START_TEST(we_can_bind_a_framebuffer_to_a_renderbuffer)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint framebuffer_name = 0;
    glGenFramebuffers(1, &framebuffer_name);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_name);

    GLuint width = 256;
    GLuint height = 256;

    GLuint color_renderbuffer;
    glGenRenderbuffers(1, &color_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_renderbuffer);

    GLenum err1 = glGetError();

    GLuint depth_renderbuffer;
    glGenRenderbuffers(1, &depth_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

    GLenum err2 = glGetError();

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers); // "1" is the size of DrawBuffers

    GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glDeleteFramebuffers(1, &framebuffer_name);
    CGLDestroyContext(context);

    ck_assert_int_eq(err, GL_FRAMEBUFFER_COMPLETE);
    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);

}
END_TEST


START_TEST(we_can_bind_a_framebuffer_to_a_texture_for_drawing)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint framebuffer_name = 0;
    glGenFramebuffers(1, &framebuffer_name);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_name);

    GLuint rendered_texture;
    glGenTextures(1, &rendered_texture);
    glBindTexture(GL_TEXTURE_2D, rendered_texture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1, 1, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
    GLenum err1 = glGetError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLenum err2 = glGetError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLenum err3 = glGetError();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendered_texture, 0);
    GLenum err4 = glGetError();

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers); // "1" is the size of DrawBuffers

    GLenum err5 = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);
    ck_assert_int_eq(err3, GL_NO_ERROR);
    ck_assert_int_eq(err4, GL_NO_ERROR);
    ck_assert_int_eq(err5, GL_FRAMEBUFFER_COMPLETE);

    glDeleteFramebuffers(1, &framebuffer_name);

    CGLDestroyContext(context);
}
END_TEST

START_TEST(glGetError_returns_an_erorr_code_when_there_is_an_error)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR); // this is not a valid input for glClear. Should be GL_COLOR_BUFFER_BIT

    GLenum err = glGetError();

    ck_assert_int_eq(err, GL_INVALID_VALUE);

    CGLDestroyContext(context);
}
END_TEST

START_TEST(we_can_create_a_texture)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint texture;
    glGenTextures(1, &texture);
    GLenum err1 = glGetError();
    glBindTexture(GL_TEXTURE_2D, texture);
    GLenum err2 = glGetError();
    glDeleteTextures(1, &texture);
    GLenum err3 = glGetError();

    CGLDestroyContext(context);
    GLenum err4 = glGetError();

    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);
    ck_assert_int_eq(err3, GL_NO_ERROR);
    ck_assert_int_eq(err4, GL_NO_ERROR);
}
END_TEST

START_TEST(we_can_create_a_frame_buffer_object)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint framebuffer_name = 0;
    glGenFramebuffers(1, &framebuffer_name);
    GLenum err1 = glGetError();

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_name);
    GLenum err2 = glGetError();

    glDeleteFramebuffers(1, &framebuffer_name);
    GLenum err3 = glGetError();

    CGLDestroyContext(context);
    GLenum err4 = glGetError();

    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);
    ck_assert_int_eq(err3, GL_NO_ERROR);
    ck_assert_int_eq(err4, GL_NO_ERROR);
}
END_TEST

START_TEST(we_can_put_data_into_and_get_data_out_of_a_buffer)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    float data[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float output[5] = { };

    GLuint buffer;

    glGenBuffers(1, &buffer);
    GLenum err1 = glGetError();

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    GLenum err2 = glGetError();

    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(output), output);
    GLenum err3 = glGetError();

    glDeleteBuffers(1, &buffer);
    GLenum err4 = glGetError();

    CGLDestroyContext(context);

    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);
    ck_assert_int_eq(err3, GL_NO_ERROR);
    ck_assert_int_eq(err4, GL_NO_ERROR);

    ck_assert(data[0] ==  output[0]);
    ck_assert(data[1] ==  output[1]);
    ck_assert(data[2] ==  output[2]);
    ck_assert(data[3] ==  output[3]);
    ck_assert(data[4] ==  output[4]);
}
END_TEST

START_TEST(we_can_create_a_buffer)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint buffer;

    glGenBuffers(1, &buffer);
    GLenum err1 = glGetError();

    glDeleteBuffers(1, &buffer);
    GLenum err2 = glGetError();

    CGLDestroyContext(context);

    ck_assert_int_eq(err1, GL_NO_ERROR);
    ck_assert_int_eq(err2, GL_NO_ERROR);
}
END_TEST

START_TEST(we_can_compile_a_shader)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLint vs_is_compiled = 0;
    GLint fs_is_compiled = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &vs_is_compiled);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &fs_is_compiled);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint is_linked = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &is_linked);

    glDeleteProgram(shader_program);
    CGLDestroyContext(context);

    ck_assert_int_eq(vs_is_compiled, GL_TRUE);
    ck_assert_int_eq(fs_is_compiled, GL_TRUE);
    ck_assert_int_eq(is_linked, GL_TRUE);
}
END_TEST

START_TEST(we_can_query_for_the_OpenGL_version)
{
    CGLPixelFormatAttribute attribs[3] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    GLint major_version = 0;
    GLint minor_version = 0;

    CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    CGLCreateContext(pixel_format, NULL, &context);
    CGLDestroyPixelFormat(pixel_format);
    CGLSetCurrentContext(context);

    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);

    CGLDestroyContext(context);
    ck_assert_int_eq(major_version, 4);
    ck_assert_int_eq(minor_version, 1);
}
END_TEST

#ifndef __travis__ /* This test FAILS on travis */
START_TEST(we_can_create_an_accelerated_OpenGL_context)
{
    CGLError err1, err2, err3, err4, err5;
    CGLPixelFormatAttribute attribs[] =
    {
        kCGLPFAAccelerated,
        0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    err1 = CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    err2 = CGLCreateContext(pixel_format, NULL, &context);
    err3 = CGLDestroyPixelFormat(pixel_format);
    err4 = CGLSetCurrentContext(context);
    err5 = CGLDestroyContext(context);

    ck_assert_int_eq(err1, kCGLNoError);
    ck_assert_int_eq(err2, kCGLNoError);
    ck_assert_int_eq(err3, kCGLNoError);
    ck_assert_int_eq(err4, kCGLNoError);
    ck_assert_int_eq(err5, kCGLNoError);
}
END_TEST
#endif

START_TEST(we_can_create_an_OpenGL_context_with_double_buffering)
{
    CGLError err1, err2, err3, err4, err5;
    CGLPixelFormatAttribute attribs[] =
    {
        kCGLPFADoubleBuffer,
        0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    err1 = CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    err2 = CGLCreateContext(pixel_format, NULL, &context);
    err3 = CGLDestroyPixelFormat(pixel_format);
    err4 = CGLSetCurrentContext(context);
    err5 = CGLDestroyContext(context);

    ck_assert_int_eq(err1, kCGLNoError);
    ck_assert_int_eq(err2, kCGLNoError);
    ck_assert_int_eq(err3, kCGLNoError);
    ck_assert_int_eq(err4, kCGLNoError);
    ck_assert_int_eq(err5, kCGLNoError);
}
END_TEST

START_TEST(we_can_create_an_OpenGL_context)
{
    CGLError err1, err2, err3, err4, err5;
    CGLPixelFormatAttribute attribs[] =
    {
        0
    };
    CGLPixelFormatObj pixel_format;
    GLint number_pixel_formats = 0;
    CGLContextObj context;

    err1 = CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);
    err2 = CGLCreateContext(pixel_format, NULL, &context);
    err3 = CGLDestroyPixelFormat(pixel_format);
    err4 = CGLSetCurrentContext(context);
    err5 = CGLDestroyContext(context);

    ck_assert_int_eq(err1, kCGLNoError);
    ck_assert_int_eq(err2, kCGLNoError);
    ck_assert_int_eq(err3, kCGLNoError);
    ck_assert_int_eq(err4, kCGLNoError);
    ck_assert_int_eq(err5, kCGLNoError);
}
END_TEST

START_TEST(we_can_choose_an_OpenGL_pixel_format)
{
    CGLError err = 0;
    CGLPixelFormatAttribute attribs[] = {(CGLPixelFormatAttribute) 0};
    CGLPixelFormatObj pixel_format;

    GLint number_pixel_formats = 0;

    err = CGLChoosePixelFormat(attribs, &pixel_format, &number_pixel_formats);

    ck_assert_int_eq(err, 0);

    CGLDestroyPixelFormat(pixel_format);
}
END_TEST

Suite *make_engine_suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("OpenGL CI Test");
    tc = tcase_create("Core");

#ifndef __travis__
    tcase_add_test(tc, we_can_create_an_accelerated_OpenGL_context);
#endif
    tcase_add_test(tc, we_can_choose_an_OpenGL_pixel_format);
    tcase_add_test(tc, we_can_create_an_OpenGL_context);
    tcase_add_test(tc, we_can_create_an_OpenGL_context_with_double_buffering);
    tcase_add_test(tc, we_can_query_for_the_OpenGL_version);
    tcase_add_test(tc, we_can_compile_a_shader);
    tcase_add_test(tc, we_can_create_a_buffer);
    tcase_add_test(tc, we_can_put_data_into_and_get_data_out_of_a_buffer);
    tcase_add_test(tc, we_can_create_a_frame_buffer_object);
    tcase_add_test(tc, we_can_create_a_texture);
    tcase_add_test(tc, glGetError_returns_an_erorr_code_when_there_is_an_error);
    tcase_add_test(tc, we_can_bind_a_framebuffer_to_a_texture_for_drawing);
    tcase_add_test(tc, we_can_bind_a_framebuffer_to_a_renderbuffer);
    tcase_add_test(tc, we_can_create_a_vertex_array_object);
    tcase_add_test(tc, we_can_bind_a_buffer_to_the_transform_feedback_target);
    tcase_add_test(tc, we_can_read_from_a_fbo_with_glReadPixels);
    tcase_add_test(tc, we_can_use_a_shader_program_and_issue_a_draw_call);

    suite_add_tcase(s, tc);

    return s;
}

int main()
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = make_engine_suite();

    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed==0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
