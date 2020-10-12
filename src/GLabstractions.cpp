#include "GLabstractions.hpp"

GLtex2D::GLtex2D(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level, const GLvoid *data)
    : internalFormat(internalFormat), format(format), glDataType(type), width(width), height(height), level(level), data(data)
{

    GL(glGenTextures(1, &id));
    bind();
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
    unbind();
}

GLtexCubeMap::GLtexCubeMap(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level, const GLvoid *data)
    : internalFormat(internalFormat), format(format), glDataType(type), width(width), height(height), level(level), data(data)
{
    GL(glGenTextures(1, &id));
    bind();
    for ( GLuint face = 0; face < 6; face++) {
        GL(glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFormat,
                      width, height, 0, format, type, data ));
    }
    GL(glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ));
    GL(glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
    GL(glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ));
    GL(glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ));
    GL(glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE ));
    GL(glGenerateMipmap( GL_TEXTURE_CUBE_MAP ));
    unbind();
}