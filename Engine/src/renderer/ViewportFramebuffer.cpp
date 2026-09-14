#include <renderer/ViewportFramebuffer.h>

#include <glad/glad.h>
#include <iostream>

ViewportFramebuffer::~ViewportFramebuffer() {
    if (m_RBO) glDeleteRenderbuffers(1, &m_RBO);
    if (m_ColorTexture) glDeleteTextures(1, &m_ColorTexture);
    if (m_FBO) glDeleteFramebuffers(1, &m_FBO);
}

void ViewportFramebuffer::Init(unsigned int width, unsigned int height) {
    m_Width = width;
    m_Height = height;
    Invalidate();
}

void ViewportFramebuffer::Resize(unsigned int width, unsigned int height) {
    if (width == 0 || height == 0)
        return;

    if (width == m_Width && height == m_Height)
        return;

    m_Width = width;
    m_Height = height;
    Invalidate();
}

void ViewportFramebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_Width, m_Height);
}

void ViewportFramebuffer::Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportFramebuffer::Invalidate() {
    if (m_FBO) {
        glDeleteFramebuffers(1, &m_FBO);
        glDeleteTextures(1, &m_ColorTexture);
        glDeleteRenderbuffers(1, &m_RBO);

        m_FBO = 0;
        m_ColorTexture = 0;
        m_RBO = 0;
    }

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_ColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        m_Width,
        m_Height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_ColorTexture,
        0
    );

    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        m_RBO
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Viewport framebuffer is not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
