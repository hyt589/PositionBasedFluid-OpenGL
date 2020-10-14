#include "GLabstractions.hpp"

GLtex2D::GLtex2D(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level, const GLvoid *data)
    : internalFormat(internalFormat), format(format), glDataType(type), width(width), height(height), level(level), data(data)
{

    GL(glGenTextures(1, &id));
    bind();
    // GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
    // GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
    GL(glTexImage2D(GL_TEXTURE_2D, level, internalFormat,
                    width, height, 0,
                    format, type, data));

    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
    GL(glGenerateMipmap(GL_TEXTURE_2D));
    unbind();
}

GLtexCubeMap::GLtexCubeMap(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level, const GLvoid *data)
    : internalFormat(internalFormat), format(format), glDataType(type), width(width), height(height), level(level), data(data)
{
    GL(glGenTextures(1, &id));
    bind();
    for (int i = 0; i <= level; i++)
    {
        for (GLuint face = 0; face < 6; face++)
        {
            GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFormat,
                            width, height, 0, format, type, data));
        }
    }
    // GL(glTexStorage2D(GL_TEXTURE_CUBE_MAP, level+1, internalFormat, width, height));
    GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0));
    GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, level));
    unbind();
}

void attachTexToFramebuffer(const GLtex2D &tex, GLenum attachment)
{
    tex.bindAndDo(std::function([&tex, attachment]() {
        GL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex.ID(), 0));
    }));
}