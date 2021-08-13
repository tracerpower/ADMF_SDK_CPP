//
//  admf_custom_internal.cpp
//  admfsdk
//
//  Created by yushihang on 2021/3/22.
//

#include "admf_internal_header.h"
#include "bson.h"
using namespace admf_internal;
using namespace admf;
#ifndef __APPLE__
#include <assert.h>
#endif
void BaseColor_internal::load(bson_iter_t *iter) //save
{
    if (iter == nullptr)
        return;

    if (!BSON_ITER_HOLDS_DOCUMENT(iter))
        return;

    bson_iter_t child;
    if (!bson_iter_recurse(iter, &child))
        return;

    std::string textureKey = getNewKey("texture");
    std::string colorKey = getNewKey("color");

    while (bson_iter_next(&child))
    {
        std::string keyName = bson_iter_key(&child);
        assert(bson_iter_value(&child) != nullptr);
        //printf("Found element key: \"%s\"\n", keyName.c_str());
        if (keyName == textureKey)
        {
            texture_ = std::make_shared<Texture_internal>(admfIndex_, &child);
            texture_->setType(getTextureType());
        }
        else if (keyName == colorKey)
        {
            data_ = std::make_shared<BaseColorData_internal>(admfIndex_, &child);
        }
    }
}

void BaseColor_internal::initMissed()
{
    if (!texture_)
    {
        texture_ = std::make_shared<Texture_internal>(admfIndex_);
        texture_->setType(getTextureType());
    }

    if (!data_)
        data_ = std::make_shared<BaseColorData_internal>(admfIndex_);
}
#ifdef ADMF_EDIT
void BaseColor_internal::save(bson_t *doc)
{
    texture_->setType(getTextureType());
    std::string textureKey = getNewKey("texture");
    std::string colorKey = getNewKey("color");

    ADMF_BSON_APPEND_DOCUMENT(doc, textureKey, texture_);
    ADMF_BSON_APPEND_DOCUMENT(doc, colorKey, data_);
}
#endif
BaseColorData BaseColor_internal::getData()
{
    return BaseColorData(data_);
}

Texture BaseColor_internal::getTexture()
{
    return Texture(texture_);
}

void Specular_internal::load(bson_iter_t *iter) //save
{
    if (iter == nullptr)
        return;

    if (!BSON_ITER_HOLDS_DOCUMENT(iter))
        return;

    bson_iter_t child;
    if (!bson_iter_recurse(iter, &child))
        return;

    std::string textureKey = getNewKey("texture");
    std::string colorKey = getNewKey("color");

    while (bson_iter_next(&child))
    {
        std::string keyName = bson_iter_key(&child);
        assert(bson_iter_value(&child) != nullptr);
        //printf("Found element key: \"%s\"\n", keyName.c_str());
        if (keyName == textureKey)
        {
            texture_ = std::make_shared<Texture_internal>(admfIndex_, &child);
            texture_->setType(getTextureType());
        }
        else if (keyName == colorKey)
        {
            color_ = std::make_shared<ColorRGB_internal>(admfIndex_, &child);
        }
    }
}

void Specular_internal::initMissed()
{
    if (!texture_)
    {
        texture_ = std::make_shared<Texture_internal>(admfIndex_);
        texture_->setType(getTextureType());
    }

    if (!color_)
        color_ = std::make_shared<ColorRGB_internal>(admfIndex_);
}
#ifdef ADMF_EDIT
void Specular_internal::save(bson_t *doc)
{
    texture_->setType(getTextureType());
    std::string textureKey = getNewKey("texture");
    std::string colorKey = getNewKey("color");

    ADMF_BSON_APPEND_DOCUMENT(doc, textureKey, texture_);
    ADMF_BSON_APPEND_DOCUMENT(doc, colorKey, color_);
}
#endif
Texture Specular_internal::getTexture()
{
    return Texture(texture_);
}

ColorRGB Specular_internal::getColor()
{
    return ColorRGB(color_);
}
#ifdef ADMF_EDIT
#define Save_Implementation(classname)  \
void classname::save(bson_t* doc)                                             \
{                                                                             \
texture_->setType(getTextureType());                                           \
std::string textureKey = getNewKey("texture");                            \
std::string valueKey = getNewKey("value");                                \
ADMF_BSON_APPEND_DOCUMENT(doc, textureKey, texture_);                      \
ADMF_BSON_APPEND_DOUBLE(doc, valueKey, value_);                            \
} 
#else
#define Save_Implementation(x)
#endif

#define TextureAndValueContainer_Internal_Implementation(classname)               \
    void classname::load(bson_iter_t *iter)                                       \
    {                                                                             \
        if (iter == nullptr)                                                      \
            return;                                                               \
                                                                                  \
        if (!BSON_ITER_HOLDS_DOCUMENT(iter))                                      \
            return;                                                               \
                                                                                  \
        bson_iter_t child;                                                        \
        if (!bson_iter_recurse(iter, &child))                                     \
            return;                                                               \
                                                                                  \
        std::string textureKey = getNewKey("texture");                            \
        std::string valueKey = getNewKey("value");                                \
                                                                                  \
        while (bson_iter_next(&child))                                            \
        {                                                                         \
            std::string keyName = bson_iter_key(&child);                          \
            assert(bson_iter_value(&child) != nullptr);                           \
            /*printf("Found element key: \"%s\"\n", keyName.c_str()); */              \
            if (keyName == textureKey)                                            \
            {                                                                     \
                texture_ = std::make_shared<Texture_internal>(admfIndex_, &child); \
                texture_->setType(getTextureType());                              \
            }                                                                     \
            else if (keyName == valueKey)                                         \
            {                                                                     \
                value_ = (ADMF_FLOAT)bson_iter_as_double(&child);                  \
            }                                                                     \
        }                                                                         \
    }                                                                             \
                                                                                  \
    Texture classname::getTexture()                                               \
    {                                                                             \
        return Texture(texture_);                                                 \
    }                                                                             \
                                                                                  \
    ADMF_FLOAT classname::getValue()                                               \
    {                                                                             \
        return value_;                                                            \
    }                                                                             \
                                                                                  \
    void classname::initMissed()                                                  \
    {                                                                             \
        if (!texture_)                                                            \
        {                                                                         \
            texture_ = std::make_shared<Texture_internal>(admfIndex_);             \
            texture_->setType(getTextureType());                                  \
        }                                                                         \
    }                                                                             \
                                                                                  \
    Save_Implementation(classname)                                                          \

TextureAndValueContainer_Internal_Implementation(Normal_internal);
TextureAndValueContainer_Internal_Implementation(Alpha_internal);
TextureAndValueContainer_Internal_Implementation(Metalness_internal);
TextureAndValueContainer_Internal_Implementation(Roughness_internal);
TextureAndValueContainer_Internal_Implementation(Glossiness_internal);
TextureAndValueContainer_Internal_Implementation(Anisotropy_internal);
TextureAndValueContainer_Internal_Implementation(AnisotropyRotation_internal);

