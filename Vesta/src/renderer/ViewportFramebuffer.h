#pragma once

class ViewportFramebuffer {
public:
    ViewportFramebuffer() = default;
    ~ViewportFramebuffer();

    void Init(unsigned int width, unsigned int height);
    void Resize(unsigned int width, unsigned int height);

    void Bind() const;
    void Unbind() const;

    unsigned int GetColorAttachment() const { return m_ColorTexture; }
    unsigned int GetWidth() const { return m_Width; }
    unsigned int GetHeight() const { return m_Height; }

private:
    void Invalidate();

private:
    unsigned int m_FBO = 0;
    unsigned int m_ColorTexture = 0;
    unsigned int m_RBO = 0;

    unsigned int m_Width = 0;
    unsigned int m_Height = 0;
};
