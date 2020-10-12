#include "global.hpp"

class I_GLtex
{
private:
public:
    I_GLtex(){};
    virtual ~I_GLtex(){};

    virtual GLuint ID() = 0;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    template <typename ReturnType, typename ... Args>
    void bindAndDo(std::function<ReturnType(Args ...)> const & callBack, Args ... params)
    {
        bind();
        callBack(params...);
        unbind();
    };

    friend class GLtex2D;
};


class GLtex2D : I_GLtex
{
private:
    GLuint id = 0;
    const GLint internalFormat;
    const GLenum format;
    const GLenum glDataType;
    const GLsizei width;
    const GLsizei height;
    const GLint level;
    const GLvoid * data;
public:
    GLtex2D(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level = 0, const GLvoid * data = NULL);
    ~GLtex2D() override {
        GL(glDeleteTextures(1, &id));
    };
    void bind() override 
    {
        GL(glBindTexture(GL_TEXTURE_2D, id));
    };
    void unbind() override
    {
        GL(glBindTexture(GL_TEXTURE_2D, 0));
    };
    GLuint ID() override
    {
        return id;
    };
};

class GLtexCubeMap : I_GLtex
{
private:
    GLuint id = 0;
    const GLint internalFormat;
    const GLenum format;
    const GLenum glDataType;
    const GLsizei width;
    const GLsizei height;
    const GLint level;
    const GLvoid * data;
public:
    GLtexCubeMap(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level = 0, const GLvoid * data = NULL);
    ~GLtexCubeMap() override
    {
        GL(glDeleteTextures(1, &id));
    };
    void bind() override 
    {
        GL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
    };
    void unbind() override
    {
        GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    };
    GLuint ID() override
    {
        return id;
    };
};

class GLframebuffer
{
private:
    GLuint id;
public:
    GLframebuffer()
    {
        GL(glGenFramebuffers(1, &id));
    };

    ~GLframebuffer()
    {
        GL(glDeleteFramebuffers(1, &id));
    };

    void bind()
    {
        GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    };

    void unbind(){
        GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    };

    template <typename ReturnType, typename ... Args>
    void bindAndDo(std::function<ReturnType(Args ...)> const & callBack, Args & ... params)
    {
        bind();
        callBack(params...);
        unbind();
    };
};