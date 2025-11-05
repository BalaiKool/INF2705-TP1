#include "textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

Texture2D::Texture2D()
: m_id(0)
{

}

void Texture2D::load(const char* path)
{
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned char* data = stbi_load(path, &width, &height, &nChannels, 0);
    if (data == NULL)
        std::cout << "Error loading texture \"" << path << "\": " << stbi_failure_reason() << std::endl;

    // TODO: Chargement de la texture en mémoire graphique.
    //       Attention au format des pixels de l'image!
    //       Toutes les variables devraient être utilisées (width, height, nChannels, data).
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    GLenum format;
    if (nChannels == 3) {
        format = GL_RGB;
    }
    else if (nChannels == 4) {
        format = GL_RGBA;
    }
    else {
        std::cout << "Erroneous number of channels (" << nChannels << ") for texture : " << path << "" << std::endl;
        return;
    }
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    // TODO: Libérer les ressources allouées.
    glDeleteTextures(1, &m_id);
    m_id = 0;
}

void Texture2D::setFiltering(GLenum filteringMode)
{
    // TODO: Configurer le filtre min et le mag avec le mode en paramètre.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filteringMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture2D::setWrap(GLenum wrapMode)
{
    // TODO: Configurer le wrap S et T avec le mode en paramètre.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
}

void Texture2D::enableMipmap()
{
    // TODO: Génère le mipmap et configure les paramètres pour l'utiliser.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::use()
{
    // TODO: Met la texture active pour être utilisée dans les prochaines commandes de dessins.
    glBindTexture(GL_TEXTURE_2D, m_id);
}

//
// Cubemap
//

TextureCubeMap::TextureCubeMap()
: m_id(0)
{

}

void TextureCubeMap::load(const char** pathes)
{
    const size_t N_TEXTURES = 6;
    unsigned char* datas[N_TEXTURES];
    int widths[N_TEXTURES];
    int heights[N_TEXTURES];
    int nChannels[N_TEXTURES];
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < 6; i++)
    {
        datas[i] = stbi_load(pathes[i], &widths[i], &heights[i], &nChannels[i], 0);
        if (datas[i] == NULL)
            std::cout << "Error loading texture \"" << pathes[i] << "\": " << stbi_failure_reason() << std::endl;
    }

    // TODO: Chargement de la texture en mémoire graphique.
    //       Faites la configuration des min et mag filtering et du wrap S, T, R directement, ils
    //       ne seront pas modifiés ailleurs.
    
    for (unsigned int i = 0; i < 6; i++)
    {
        // TODO
    }

    for (unsigned int i = 0; i < 6; i++)
    {
        stbi_image_free(datas[i]);
    }
}

TextureCubeMap::~TextureCubeMap()
{
    // TODO: Libérer les ressources allouées.
}

void TextureCubeMap::use()
{
    // TODO: Met la texture active pour être utilisée dans les prochaines commandes de dessins.
}

