#include "global.hpp"

class I_GLtex
{
private:
public:
    I_GLtex(){};
    virtual ~I_GLtex(){};

    virtual GLuint ID() const = 0;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;


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
    const GLvoid *data;

public:
    GLtex2D(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level = 0, const GLvoid *data = NULL);
    ~GLtex2D() override
    {
        GL(glDeleteTextures(1, &id));
    };
    void bind() const override
    {
        GL(glBindTexture(GL_TEXTURE_2D, id));
    };
    void unbind() const override
    {
        GL(glBindTexture(GL_TEXTURE_2D, 0));
    };
    GLuint ID() const override
    {
        return id;
    };

    template <typename ReturnType, typename... Args>
    void bindAndDo(std::function<ReturnType(Args...)> const &callBack, Args... params) const
    {
        bind();
        callBack(params...);
        unbind();
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
    const GLvoid *data;

public:
    GLtexCubeMap(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level = 0, const GLvoid *data = NULL);
    ~GLtexCubeMap() override
    {
        GL(glDeleteTextures(1, &id));
    };
    void bind() const override
    {
        GL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
    };
    void unbind() const override
    {
        GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    };
    GLuint ID() const override
    {
        return id;
    };

    template <typename ReturnType, typename... Args>
    void bindAndDo(std::function<ReturnType(Args...)> const &callBack, Args... params) const
    {
        bind();
        callBack(params...);
        unbind();
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

    void bind() const
    {
        GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    };

    void unbind() const
    {
        GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    };

    template <typename ReturnType, typename... Args>
    void bindAndDo(std::function<ReturnType(Args...)> const &callBack, Args &... params) const
    {
        bind();
        callBack(params...);
        unbind();
    };
};

class GLrenderbuffer
{
private:
    GLuint id;

public:
    GLrenderbuffer()
    {
        GL(glGenRenderbuffers(1, &id));
    }

    ~GLrenderbuffer()
    {
        GL(glDeleteRenderbuffers(1, &id));
    }

    GLuint ID() const
    {
        return id;
    };

    void bind() const
    {
        GL(glBindRenderbuffer(GL_RENDERBUFFER, id));
    };

    void unbind() const
    {
        GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    };

    template <typename ReturnType, typename... Args>
    void bindAndDo(std::function<ReturnType(Args...)> const &callBack, Args &... params) const
    {
        bind();
        callBack(params...);
        unbind();
    };
};

void attachTexToFramebuffer(const GLtex2D &tex, GLenum attachment);