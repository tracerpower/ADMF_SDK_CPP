//
//  4ddat_to_admf.cpp
//  4ddat_to_admf
//
//  Created by yushihang on 2021/3/26.
//

#include "xtex_to_admf.hpp"

#include <string>
#include <memory>
#include "admf_cpp.h"
#include <iostream>
#if (defined _WIN32) || (defined __APPLE__)
#include <filesystem>
#else
#include <sys/stat.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#endif

#include <fstream>
#include <chrono>
#include "FreeImage/FreeImage.h"
#include "admf_main_internal.h"
#include "admf_base_internal.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include <iostream>
#include <thread>

#include <sstream>
#include <fstream>
#include <string>
#include "changecolor.h"
#include <iomanip>

#include <ctpl_stl.h>

#include "ZipFile.h"

#include "rapidxml_iterators.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"

#include "rapidjson/document.h"
#include "datestring_to_int.hpp"

bool g_isMetalnessPipeline = false;
extern std::string replaceAll(const char *pszSrc, const char *pszOld, const char *pszNew);

bool hasEnding(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

struct XTexMap
{
    std::string diffuse;
    std::string normal;
    std::string alpha;
    std::string roughness;
    std::string displacement;
};

enum class CompareType
{
    Match = 0,
    Surfix = 1,
    Contain = 2,
};

struct _Vec3
{
    double r, g, b;
};

std::string _getContentFromZip(const ZipArchive::Ptr &zipArchive, const std::string &target, CompareType compareType)
{
    auto count = zipArchive->GetEntriesCount();

    //parseXML
    std::istream *stream = nullptr;
    for (int i = 0; i < count; i++)
    {

        auto entry = zipArchive->GetEntry(i);
        auto &name = entry->GetName();
        switch (compareType)
        {
        case CompareType::Match:
            if (name == target)
            {
                stream = entry->GetDecompressionStream();
                return std::string(std::istreambuf_iterator<char>(*stream), {});
            }
            break;
        case CompareType::Surfix:
            if (hasEnding(name, target))
            {
                stream = entry->GetDecompressionStream();
                return std::string(std::istreambuf_iterator<char>(*stream), {});
            }
            break;
        case CompareType::Contain:
            if (name.find(target) != std::string::npos)
            {
                stream = entry->GetDecompressionStream();
                return std::string(std::istreambuf_iterator<char>(*stream), {});
            }
            break;
            break;

        default:
            break;
        }
    }

    return "";
}

std::string _getFileContentByMapType(const ZipArchive::Ptr &zipArchive, const std::string &mapType, const XTexMap &xTexMap, const std::string &suffix)
{

    return _getContentFromZip(zipArchive, suffix, CompareType::Surfix);
}

void _factorAndOffset(BYTE &byte, const std::string &mode, double factor, double offset)
{
    if (mode == "multiply")
        byte *= factor;
    else if (mode == "divide")
    {
        if (factor != 0)
            byte /= factor;
    }
    else if (mode == "add")
        byte += factor;
    else if (mode == "subtract")
        byte -= factor;
    else if (mode == "max")
        byte = std::max((double)byte, factor); //pixel channel vs factor??
    else if (mode == "min")
        byte = std::min((double)byte, factor); //pixel channel vs factor??
    else if (mode == "overlay")
    {
        //https://en.wikipedia.org/wiki/Blend_modes
        if (byte < 128)
            byte = 2.0 * byte * factor;
        else
            byte = 255 - 2 * (255 - byte) * (1.0 - factor);
    }

    byte += offset;
    byte = std::min(std::max(255, (int)byte), 0);
}

void _parseU3mTexture(const admf::LayerBasic admfLayerBasic, const admf::Texture &admfTexture_, const rapidjson::Value &u3mTexture, const std::string &mapType, const XTexMap &xTexMap, const ZipArchive::Ptr &zipArchive, bool ignoreFactorAndOffset = false)
{

    bool isRoughness = mapType == "roughness";

    admf::Texture admfTexture = admfTexture_;

    std::string imageFileSuffix = "not a valid suffix!@#$%^&*()/\\[]<>";
    if (u3mTexture.HasMember("image"))
    {
        auto &image = u3mTexture["image"];
        if (image.HasMember("path"))
        {
            std::string path = image["path"].GetString();
            auto pos = path.rfind("_");
            if (pos != std::string::npos)
            {
                imageFileSuffix = path.substr(pos);
            }
        }
    }
    std::string content = _getFileContentByMapType(zipArchive, mapType, xTexMap, imageFileSuffix);
    bool hasTextureContent = !content.empty();

    if (isRoughness && !g_isMetalnessPipeline && hasTextureContent)
    {
        admfTexture = admfLayerBasic->getGlossiness()->getTexture();
    }

    if (u3mTexture.HasMember("image"))
    {
        auto &image = u3mTexture["image"];

        if (image.HasMember("dpi"))
        {
            auto dpi = image["dpi"].GetDouble();
            admfTexture->getDpi()->setX(dpi);
            admfTexture->getDpi()->setY(dpi);
        }

        if (image.HasMember("width"))
        {
            auto width = image["width"].GetDouble();
            admfTexture->setPhysicalWidth(width * 10); //u3m是cm， admf是mm
        }

        if (image.HasMember("height"))
        {
            auto height = image["height"].GetDouble();
            admfTexture->setPhysicalHeight(height*10); //u3m是cm， admf是mm
        }
    }

    std::string mode;
    if (u3mTexture.HasMember("mode"))
    {
        mode = u3mTexture["mode"].GetString();
    }

    _Vec3 factorVec3 = {1.0, 1.0, 1.0};
    if (u3mTexture.HasMember("factor"))
    {
        auto &factor = u3mTexture["factor"];

        if (factor.IsNumber())
            factorVec3.r = factorVec3.g = factorVec3.b = factor.GetDouble();
        else
        {
            if (factor.HasMember("r"))
                factorVec3.r = factor["r"].GetDouble();
            if (factor.HasMember("g"))
                factorVec3.g = factor["g"].GetDouble();
            if (factor.HasMember("b"))
                factorVec3.b = factor["b"].GetDouble();
        }
    }

    _Vec3 offsetVec3 = {0.0, 0.0, 0.0};
    if (u3mTexture.HasMember("offset"))
    {
        auto &offset = u3mTexture["offset"];
        if (offset.IsNumber())
            offsetVec3.r = offsetVec3.g = offsetVec3.b = offset.GetDouble();
        else
        {
            if (offset.HasMember("r"))
                offsetVec3.r = offset["r"].GetDouble();
            if (offset.HasMember("g"))
                offsetVec3.g = offset["g"].GetDouble();
            if (offset.HasMember("b"))
                offsetVec3.b = offset["b"].GetDouble();
        }
    }

    float _epsilon = 0.001;

    
    
    FREE_IMAGE_FORMAT format = FIF_PNG;
    bool needConvertFormat = false;
    
    if (hasTextureContent)
    {
        bool needHandleFactorAndOffset = false;
        if (!ignoreFactorAndOffset && mode != "recolor")
        {

            if (isRoughness && !g_isMetalnessPipeline)
            {
                needHandleFactorAndOffset = true;
            }

            if (!needHandleFactorAndOffset)
            {
                if (::abs(offsetVec3.r) > _epsilon || ::abs(offsetVec3.g) > _epsilon || ::abs(offsetVec3.b) > _epsilon)
                {
                    needHandleFactorAndOffset = true;
                }
            }

            if (!needHandleFactorAndOffset)
            {
                if (mode == "overlay" || mode == "max" || mode == "min")
                {
                    //https://en.wikipedia.org/wiki/Blend_modes
                    needHandleFactorAndOffset = true;
                }
                else if (mode == "add" || mode == "subtract")
                {
                    if (::abs(factorVec3.r) > _epsilon || ::abs(factorVec3.g) > _epsilon || ::abs(factorVec3.b) > _epsilon)
                        needHandleFactorAndOffset = true;
                }
                else /*if (mode == "multiply" || mode == "divide")*/
                {
                    if ((factorVec3.r >= 0 && (::abs(factorVec3.r - 1.0) > _epsilon)) ||
                        (factorVec3.g >= 0 && (::abs(factorVec3.g - 1.0) > _epsilon)) ||
                        (factorVec3.b >= 0 && (::abs(factorVec3.b - 1.0) > _epsilon)))
                        needHandleFactorAndOffset = true;
                }
            }
            
        }
        
        
        
        admf::TextureFileType textureBinaryType = admf_internal::Texture_internal::getTypeByBinaryData((const unsigned char *)content.c_str(), (admf::ADMF_UINT)content.length());
        
        
        switch (textureBinaryType)
        {
        case admf::TextureFileType::PNG:
            format = FIF_PNG;
            break;
        case admf::TextureFileType::JPG:
            format = FIF_JPEG;
            break;
        case admf::TextureFileType::GIF:
            format = FIF_GIF;
            break;
        case admf::TextureFileType::TIFF:
            format = FIF_TIFF;
            break;
            
        default:
            break;
        }
        
        if (!needConvertFormat)
        {
            needConvertFormat = format != FIF_PNG;
        }
        

        
        bool needReadTextureSize = true; //u3m没有具体的纹理尺寸数据，而admf必须需要， 所以必须处理

        if (!needReadTextureSize && !needHandleFactorAndOffset && !needConvertFormat)
            admfTexture->getBinaryData()->updateFromData(content.c_str(), (admf::ADMF_UINT)content.length());
        else
        {

            bool success = false;

            do
            {
                FIMEMORY *stream = FreeImage_OpenMemory();
                FreeImage_WriteMemory(content.c_str(), 1, (unsigned)content.length(), stream);
                FreeImage_SeekMemory(stream, 0, SEEK_SET);
                FIBITMAP *bitmap = FreeImage_LoadFromMemory(format, stream);
                FreeImage_CloseMemory(stream);
                if (bitmap == nullptr)
                    break;

                if (needReadTextureSize)
                {
                    int width = FreeImage_GetWidth(bitmap);
                    int height = FreeImage_GetHeight(bitmap);
                    admfTexture->setWidth(width);
                    admfTexture->setHeight(height);

                    admfTexture->setElementSize(1);
                    
                    FREE_IMAGE_COLOR_TYPE colourType = FreeImage_GetColorType(bitmap);
                    
                    int channel = -1;
                    switch (colourType)
                    {
                    case FIC_MINISWHITE:
                        channel = 1;
                        break;
                    case FIC_MINISBLACK:
                        channel = 1;
                        break;
                    case FIC_RGB:
                        channel = 3;
                        break;
                    case FIC_PALETTE:
                        break;
                    case FIC_RGBALPHA:
                        channel = 4;
                        break;
                    case FIC_CMYK:
                        break;
                    default:
                        break;
                    }
                    
                    admfTexture->setChannels(channel);

                }
                if (!needHandleFactorAndOffset && !needConvertFormat)
                    break;
       
                if (needHandleFactorAndOffset)
                {
                    
                    int bpp = FreeImage_GetBPP(bitmap);
     
                    
                    
                    int channel = bpp / 8;
                    if (bpp != 32 && bpp != 24)
                    {
                        FIBITMAP *bmpTemp = nullptr;
                        if (FreeImage_IsTransparent(bitmap))
                        {
                            bmpTemp = FreeImage_ConvertTo32Bits(bitmap);
                        }
                        else
                        {
                            bmpTemp = FreeImage_ConvertTo24Bits(bitmap);
                        }
                        if (bitmap != nullptr)
                            FreeImage_Unload(bitmap);
                        bitmap = bmpTemp;
                        bmpTemp = nullptr;
                        
                        bpp = FreeImage_GetBPP(bitmap);
                        channel = bpp / 8;
                    }
                    
                    
                    int width = FreeImage_GetWidth(bitmap);
                    int height = FreeImage_GetHeight(bitmap);
                    

                    int pitch = FreeImage_GetPitch(bitmap);
                    
                    BYTE *bits = (BYTE *)malloc(height * pitch);
                    // convert the bitmap to raw bits (top-left pixel first)
                    FreeImage_ConvertToRawBits(bits, bitmap, pitch, bpp, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FALSE);
                    
                    FreeImage_Unload(bitmap);
                    
                    offsetVec3.r *= 255;
                    offsetVec3.g *= 255;
                    offsetVec3.b *= 255;
                    
                    for (int row = 0; row < height; row++)
                    {
                        for (int col = 0; col < width; col++)
                        {
                            int index = row * pitch + col * channel;
                            _factorAndOffset(bits[index], mode, factorVec3.r, offsetVec3.r);
                            _factorAndOffset(bits[index + 1], mode, factorVec3.g, offsetVec3.g);
                            _factorAndOffset(bits[index + 2], mode, factorVec3.b, offsetVec3.b);
                            
                            if (isRoughness && !g_isMetalnessPipeline)
                            {
                                bits[index] = 255 - bits[index];
                                bits[index + 1] = 255 - bits[index + 1];
                                bits[index + 2] = 255 - bits[index + 2];
                            }
                            
                            //factor和offset就没有alpha通道， 所以就算channel为4的话， alpha也不处理了
                        }
                    }
                    
                    
                    bitmap = FreeImage_ConvertFromRawBits(bits, width, height, pitch, bpp, FI_RGBA_RED_MASK,
                                                                        FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FALSE);

                }

               

   

                FIMEMORY *stream_new = FreeImage_OpenMemory();
                if (!FreeImage_SaveToMemory(FIF_PNG, bitmap, stream_new))
                {
                    FreeImage_CloseMemory(stream_new);
                    break;
                }

                FreeImage_Unload(bitmap);
                FreeImage_SeekMemory(stream_new, 0, SEEK_SET);
                BYTE *pngRawData = nullptr;
                DWORD pngRawDataSize = 0;
                BOOL acquireResult = FreeImage_AcquireMemory(stream_new, &pngRawData, &pngRawDataSize);
                if (!acquireResult)
                {
                    FreeImage_CloseMemory(stream_new);
                    break;
                }
                admfTexture->getBinaryData()->updateFromData(pngRawData, (admf::ADMF_UINT)pngRawDataSize);
                success = true;
            } while (0);

            if (!success)
                admfTexture->getBinaryData()->updateFromData(content.c_str(), (admf::ADMF_UINT)content.length());
        }
    }
}

void _parseU3mMaterialLayer(const admf::MaterialLayer &admfMaterialLayer, const rapidjson::Value &u3mLayer, const XTexMap &xTexMap, const ZipArchive::Ptr &zipArchive)
{
    if (u3mLayer.IsNull())
        return;
    auto admfBasic = admfMaterialLayer->getBasic();

    if (u3mLayer.HasMember("shader") && !u3mLayer["shader"].IsNull())
    {
        const char *shader = u3mLayer["shader"].GetString();

        admfMaterialLayer->getShader()->setString(shader);
    }

#define UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE(key, admfDataMethod)                                        \
    {                                                                                                       \
        if (u3mLayer.HasMember(key))                                                                        \
        {                                                                                                   \
            auto &u3mData = u3mLayer[key];                                                                  \
            auto admfData = admfBasic->admfDataMethod();                                                    \
            if (u3mData.HasMember("constant"))                                                              \
            {                                                                                               \
                auto &constant = u3mData["constant"];                                                       \
                if (!constant.IsNull())                                                                     \
                {                                                                                           \
                    double value = constant.GetDouble();                                                    \
                    admfData->setValue(value);                                                              \
                    if (strcmp(key, "roughness") == 0 && !g_isMetalnessPipeline)                                    \
                    {                                                                                       \
                        double newValue = 1.0 - value;                                                      \
                        newValue = std::min(1.0, newValue);                                                 \
                        newValue = std::max(0.0, newValue);                                                 \
                        admfBasic->getGlossiness()->setValue(newValue);                                     \
                    }                                                                                       \
                }                                                                                           \
            }                                                                                               \
            if (u3mData.HasMember("texture"))                                                               \
            {                                                                                               \
                auto &texture = u3mData["texture"];                                                         \
                if (!texture.IsNull())                                                                      \
                {                                                                                           \
                    _parseU3mTexture(admfBasic, admfData->getTexture(), texture, key, xTexMap, zipArchive); \
                }                                                                                           \
            }                                                                                               \
        }                                                                                                   \
    }

#define UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_COLOR(key, admfDataMethod)                                            \
    {                                                                                                           \
        if (u3mLayer.HasMember(key))                                                                            \
        {                                                                                                       \
            auto &u3mData = u3mLayer[key];                                                                      \
            auto admfData = admfBasic->admfDataMethod();                                                        \
            if (u3mData.HasMember("constant"))                                                                  \
            {                                                                                                   \
                auto &constant = u3mData["constant"];                                                           \
                if (!constant.IsNull())                                                                         \
                {                                                                                               \
                    if (constant.HasMember("r") && constant.HasMember("g") && constant.HasMember("b"))          \
                    {                                                                                           \
                        auto admfColor = admfData->getColor();                                                  \
                        admfColor->setR(constant["r"].GetDouble());                                             \
                        admfColor->setG(constant["g"].GetDouble());                                             \
                        admfColor->setB(constant["b"].GetDouble());                                             \
                    }                                                                                           \
                }                                                                                               \
                if (u3mData.HasMember("texture"))                                                               \
                {                                                                                               \
                    auto &texture = u3mData["texture"];                                                         \
                    if (!texture.IsNull())                                                                      \
                    {                                                                                           \
                        _parseU3mTexture(admfBasic, admfData->getTexture(), texture, key, xTexMap, zipArchive); \
                    }                                                                                           \
                }                                                                                               \
            }                                                                                                   \
        }                                                                                                       \
    }

#define UPDATE_ADMF_DATA_FROM_U3M_TEXTURE(key, admfDataMethod)                                                    \
    {                                                                                                             \
        if (u3mLayer.HasMember(key))                                                                              \
        {                                                                                                         \
            auto &u3mData = u3mLayer[key];                                                                        \
            auto admfData = admfBasic->admfDataMethod();                                                          \
            if (u3mData.HasMember("texture"))                                                                     \
            {                                                                                                     \
                auto &texture = u3mData["texture"];                                                               \
                if (!texture.IsNull())                                                                            \
                {                                                                                                 \
                    _parseU3mTexture(admfBasic, admfData->getTexture(), texture, key, xTexMap, zipArchive, true); \
                }                                                                                                 \
            }                                                                                                     \
        }                                                                                                         \
    }

    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("alpha", getAlpha);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("anisotropy_rotation", getAnisotropyRotation);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("anisotropy_value", getAnisotropy);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE("clearcoat_normal", getClearCoatNormal);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("clearcoat_roughness", getClearCoatRoughness);

    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("clearcoat_value", getClearCoatValue);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("displacement", getHeight);

    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("metalness", getMetalness);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE("normal", getNormal);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("roughness", getRoughness);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("sheen_tint", getSheenTint);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("sheen_value", getSheenValue);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("specular_tint", getSpecularTint);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_COLOR("subsurface_color", getSubSurfaceColor);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("subsurface_radius", getSubSurfaceRadius);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("subsurface_value", getSubSurfaceValue);
    UPDATE_ADMF_DATA_FROM_U3M_TEXTURE_VALUE("transmission", getTransmission);
    //ior不支持texture
    //https://github.com/vizoogmbh/u3m/blob/master/u3m1.0/U3M.pdf
    
#define ADD_SOLID_BLOCK(r, g, b, isOriginal) \
    {  \
        std::string solidColor = "";  \
        auto solidBlock = solidBlockArray->append();  \
        solidColor.append(std::to_string(r)).append(",").append(std::to_string(g)).append(",").append(std::to_string(b));  \
        solidBlock->getValue()->setString(solidColor.c_str());  \
        solidColor = std::string("(") + solidColor;  \
        solidColor += ")";  \
        solidBlock->getName()->setString(solidColor.c_str());  \
        solidBlock->setOriginal(isOriginal);  \
    }
    {
        const char *key = "basecolor";

        if (u3mLayer.HasMember(key))
        {
            
            auto &u3mData = u3mLayer[key];
            auto admfData = admfBasic->getBaseColor();
            bool hasTexture = false;
            
            auto baseData = admfData->getData();
            baseData->getType()->setString("solid");
            baseData->setIndex(0);
            auto solidBlockArray = baseData->getSolid()->getBlockArray();
            
            if (u3mData.HasMember("texture"))
            {
                auto &texture = u3mData["texture"];
                if (!texture.IsNull())
                {
                    hasTexture = true;
                    _parseU3mTexture(admfBasic, admfData->getTexture(), texture, key, xTexMap, zipArchive);
                    
               
                    
        
                    
                    std::string mode;
                    if (texture.HasMember("mode"))
                    {
                        mode = texture["mode"].GetString();
                    }
                    
                    if (mode == "recolor")
                    {
                       
                        if (texture.HasMember("factor"))
                        {
                            auto &factor = texture["factor"];
                            if (factor.HasMember("r") && factor.HasMember("g") && factor.HasMember("b"))
                            {
                                
                                int r = 255 * factor["r"].GetDouble();
                                int g = 255 * factor["g"].GetDouble();
                                int b = 255 * factor["b"].GetDouble();
                                ADD_SOLID_BLOCK(r, g, b, false);
                            }

                            
                        }
                    }
                }
            }
            
            if (!hasTexture && u3mData.HasMember("constant"))
            {
                auto &constant = u3mData["constant"];
                if (!constant.IsNull())
                {
                    if (constant.HasMember("r") && constant.HasMember("g") && constant.HasMember("b"))
                    {
                        int r = 255 * constant["r"].GetDouble();
                        int g = 255 * constant["g"].GetDouble();
                        int b = 255 * constant["b"].GetDouble();
                        ADD_SOLID_BLOCK(r, g, b, false);
                    }
                }
            }
            
            
            if (solidBlockArray->size() == 0)
            {
                ADD_SOLID_BLOCK(0, 0, 0, true);
            }
            
        }
    }

    {
        const char *key = "specular_value";

        if (u3mLayer.HasMember(key))
        {
            auto &u3mData = u3mLayer[key];
            if (!u3mData.IsNull())
            {
                auto admfData = admfBasic->getSpecular();

                if (u3mData.HasMember("constant"))
                {
                    auto &constant = u3mData["constant"];
                    if (!constant.IsNull())
                    {
                        double value = constant.GetDouble();
                        admfData->setValue(value);
                    }
                }
                if (u3mData.HasMember("texture"))
                {
                    auto &texture = u3mData["texture"];
                    if (!texture.IsNull())
                    {
                        _parseU3mTexture(admfBasic, admfData->getTexture(), texture, key, xTexMap, zipArchive);
                    }
                }
            }
        }
    }

    {
        const char *key = "ior";

        if (u3mLayer.HasMember(key))
        {
            auto &u3mData = u3mLayer[key];
            if (!u3mData.IsNull())
            {
                auto admfData = admfMaterialLayer->getSpec()->getRefraction();

                if (u3mData.HasMember("constant"))
                {
                    auto &constant = u3mData["constant"];

                    if (!constant.IsNull())
                    {
                        double value = constant.GetDouble();
                        admfData->setValue(value);
                    }
                }
                if (u3mData.HasMember("texture"))
                {
                    auto &texture = u3mData["texture"];
                    if (!texture.IsNull())
                    {
                        _parseU3mTexture(admfBasic, admfData->getTexture(), texture, key, xTexMap, zipArchive);
                    }
                }
            }
        }
    }

    if (admfBasic->getMetalness()->getTexture()->getBinaryData()->getDataLength() > 0)
    {
        admfMaterialLayer->getType()->setString("silk");
    }
    else
        admfMaterialLayer->getType()->setString("leather");
}

bool _parseU3m(const admf::ADMF &admf, const ZipArchive::Ptr &zipArchive, const std::string &filename, const XTexMap &xTexMap)
{
    try
    {


        rapidjson::Document doc;
        std::string u3mContent = _getContentFromZip(zipArchive, ".u3m", CompareType::Surfix);
        if (u3mContent.empty())
            return false;
        doc.Parse(u3mContent.c_str());

        if (doc.HasParseError())
            return false;

        admf::Material admfMaterial = admf->getMaterial();

        auto metadata = admfMaterial->getMetaData();
        metadata->getType()->setString("xtex");

        auto metadataSource = metadata->getSource();

#if (defined __APPLE__) || (defined _WIN32)
#ifdef __APPLE__
        namespace fs = std::__fs::filesystem;
#else
        namespace fs = std::filesystem;
#endif
        auto path = fs::path(filename);

        std::string sourceFileName = path.filename().string();
#else
        std::string sourceFileName;
        auto pos = filename.rfind("/");
        if (pos == std::string::npos)
        {
            sourceFileName = filename;
        }
        else
        {
            sourceFileName = filename.substr(pos + 1);
        }
#endif
        metadataSource->setName(sourceFileName.c_str());
        metadataSource->updateFromFile(filename.c_str(), false);

        if (doc.HasMember("schema"))
        {
            auto schema = doc["schema"].GetString();
            metadata->getVersion()->setString(schema);
        }

        if (doc.HasMember("material"))
        {
            auto &u3mMaterial = doc["material"];
            if (u3mMaterial.HasMember("created"))
            {
                std::string createdDate = u3mMaterial["created"].GetString();
                if (!hasEnding(createdDate, "Z"))
                    createdDate.append("Z");
                int64_t date;
                admfExport::bson_error_t error;
                admfExport::_bson_iso8601_date_parse(createdDate.c_str(), (int32_t)createdDate.size(), &date, &error);
                admfMaterial->setCreatedTime(date);
            }

            if (u3mMaterial.HasMember("modified"))
            {
                std::string modifiedDate = u3mMaterial["modified"].GetString();
                if (!hasEnding(modifiedDate, "Z"))
                    modifiedDate.append("Z");
                int64_t date;
                admfExport::bson_error_t error;
                admfExport::_bson_iso8601_date_parse(modifiedDate.c_str(), (int32_t)modifiedDate.size(), &date, &error);
                admfMaterial->setCreatedTime(date);
            }

            if (u3mMaterial.HasMember("id"))
            {
                auto id = u3mMaterial["id"].GetString();
                admfMaterial->getId()->setString(id);
            }

            if (u3mMaterial.HasMember("front"))
            {
                auto &front = u3mMaterial["front"];
                if (!front.IsNull())
                {
                    auto materialLayer = admfMaterial->getLayerArray()->append();
                    materialLayer->setEnabled(1);
                    _parseU3mMaterialLayer(materialLayer, front, xTexMap, zipArchive);
                }
            }

            if (u3mMaterial.HasMember("back"))
            {
                auto &back = u3mMaterial["back"];
                if (!back.IsNull())
                {
                    auto materialLayer = admfMaterial->getLayerArray()->append();
                    materialLayer->setEnabled(1);
                    _parseU3mMaterialLayer(materialLayer, back, xTexMap, zipArchive);
                }
            }
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}
/*

bool _parseXtex(const admf::ADMF &admf, const ZipArchive::Ptr &zipArchive, const std::string &filename, const XTexMap &xTexMap)
{
    try
    {
        

        std::string xtexContent = _getContentFromZip(zipArchive, ".xtex", CompareType::Surfix);
        if (xtexContent.empty())
            return false;
        //parse XML
        rapidxml::xml_document<> doc;
        doc.parse<0>((char *)(xtexContent.c_str()));


        

        
        rapidxml::xml_node<> *firstNode = doc.first_node();
        
        auto *name = firstNode->name();
        assert(strcmp(name, "swatch") == 0);
        
        {
            auto *node = firstNode->first_node("uuid");
            if (node)
            {
                auto *value = node->value();
                if (value)
                {
                    auto id = admfMaterial->getId();
                    id->setString(value);
                }
            }
        }
        
        admf::Material admfMaterial = admf->getMaterial();
        
        auto metadata = admfMaterial->getMetaData();
        metadata->getType()->setString("xtex");
        
        auto metadataSource = metadata->getSource();
        
#if (defined __APPLE__) || (defined _WIN32)
#ifdef __APPLE__
        namespace fs = std::__fs::filesystem;
#else
        namespace fs = std::filesystem;
#endif
        auto path = fs::path(filename);
        
        std::string sourceFileName = path.filename().string();
#else
        std::string sourceFileName;
        auto pos = filename.rfind("/");
        if (pos == std::string::npos)
        {
            sourceFileName = filename;
        }
        else
        {
            sourceFileName = filename.substr(pos + 1);
        }
#endif
        metadataSource->setName(sourceFileName.c_str());
        metadataSource->updateFromFile(filename.c_str(), false);
        
        if (doc.HasMember("schema"))
        {
            auto schema = doc["schema"].GetString();
            metadata->getVersion()->setString(schema);
        }
        
        if (doc.HasMember("material"))
        {
            auto &u3mMaterial = doc["material"];
            if (u3mMaterial.HasMember("created"))
            {
                std::string createdDate = u3mMaterial["created"].GetString();
                if (!hasEnding(createdDate, "Z"))
                    createdDate.append("Z");
                int64_t date;
                admfExport::bson_error_t error;
                admfExport::_bson_iso8601_date_parse(createdDate.c_str(), (int32_t)createdDate.size(), &date, &error);
                admfMaterial->setCreatedTime(date);
            }
            
            if (u3mMaterial.HasMember("modified"))
            {
                std::string modifiedDate = u3mMaterial["modified"].GetString();
                if (!hasEnding(modifiedDate, "Z"))
                    modifiedDate.append("Z");
                int64_t date;
                admfExport::bson_error_t error;
                admfExport::_bson_iso8601_date_parse(modifiedDate.c_str(), (int32_t)modifiedDate.size(), &date, &error);
                admfMaterial->setCreatedTime(date);
            }
            
            if (u3mMaterial.HasMember("id"))
            {
                auto id = u3mMaterial["id"].GetString();
                admfMaterial->getId()->setString(id);
            }
            
            if (u3mMaterial.HasMember("front"))
            {
                auto &front = u3mMaterial["front"];
                if (!front.IsNull())
                {
                    auto materialLayer = admfMaterial->getLayerArray()->append();
                    materialLayer->setEnabled(1);
                    _parseU3mMaterialLayer(materialLayer, front, xTexMap, zipArchive);
                }
            }
            
            if (u3mMaterial.HasMember("back"))
            {
                auto &back = u3mMaterial["back"];
                if (!back.IsNull())
                {
                    auto materialLayer = admfMaterial->getLayerArray()->append();
                    materialLayer->setEnabled(1);
                    _parseU3mMaterialLayer(materialLayer, back, xTexMap, zipArchive);
                }
            }
        }
    }
    catch (...)
    {
        return false;
    }
    
    return true;
}
 */

bool _parseXML(const admf::ADMF &admf, const ZipArchive::Ptr &zipArchive, const std::string &filename, XTexMap &xTexMap)
{

    try
    {
        std::string xmlContent = _getContentFromZip(zipArchive, "_Description.xml", CompareType::Surfix);
        if (xmlContent.empty())
            return false;
        //parse XML
        rapidxml::xml_document<> doc;
        doc.parse<0>((char *)(xmlContent.c_str()));

        admf::Material admfMaterial = admf->getMaterial();

        const auto p1 = std::chrono::system_clock::now();
        admf::ADMF_DATE timeStamp = (admf::ADMF_DATE)std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
        admfMaterial->setCreatedTime(timeStamp);
        admfMaterial->setModifiedTime(timeStamp);

        rapidxml::xml_node<> *firstNode = doc.first_node();

        auto *name = firstNode->name();
        assert(strcmp(name, "swatch") == 0);

        {
            auto *node = firstNode->first_node("uuid");
            if (node)
            {
                auto *value = node->value();
                if (value)
                {
                    auto id = admfMaterial->getId();
                    id->setString(value);
                }
            }
        }

        {
            auto *node = firstNode->first_node("name");
            if (node)
            {
                auto *value = node->value();
                if (value)
                {
                    auto materialName = admfMaterial->getName();
                    materialName->setString(value);
                }
            }
        }

        {
            auto *node = firstNode->first_node("visualization.group");
            if (node)
            {
                auto *colorway_group = node->first_node("COLORWAY.group");
                if (colorway_group)
                {
                    auto *color_map_group = colorway_group->first_node("color_map.group");
                    if (color_map_group)
                    {
                        auto *file_image_map = color_map_group->first_node("file.image_map");
                        if (file_image_map)
                        {
                            auto *value = file_image_map->value();
                            if (value)
                                xTexMap.diffuse = value;
                        }
                    }

                    auto *normal_map_group = colorway_group->first_node("normal_map.group");
                    if (normal_map_group)
                    {
                        auto *file_image_map = normal_map_group->first_node("file.image_map");
                        if (file_image_map)
                        {
                            auto *value = file_image_map->value();
                            if (value)
                                xTexMap.normal = value;
                        }

                        auto *bumpstrength_text = normal_map_group->first_node("BumpStrength.text");
                        if (bumpstrength_text)
                        {
                            auto *value = bumpstrength_text->value();
                            if (value)
                            {
                                double normalValue;
                                try
                                {
                                    normalValue = std::stoi(value);
                                }
                                catch (...)
                                {
                                    normalValue = 1.;
                                }
                                //materialLayer->getBasic()->getNormal()->setValue(normalValue);
                            }
                        }
                    }
                }
                auto *xtex_group = node->first_node("xTex.group");
                if (xtex_group)
                {
                    auto *alpha_image_map = xtex_group->first_node("alpha.image_map");
                    if (alpha_image_map)
                    {
                        auto *value = alpha_image_map->value();
                        if (value)
                            xTexMap.alpha = value;
                    }

                    if (!xTexMap.diffuse.empty())
                    {
                        auto *color_image_map = xtex_group->first_node("color.image_map");
                        if (color_image_map)
                        {
                            auto *value = color_image_map->value();
                            if (value)
                                xTexMap.diffuse = value;
                        }
                    }

                    auto *displacement_image_map = xtex_group->first_node("displacement.image_map");
                    if (displacement_image_map)
                    {
                        auto *value = displacement_image_map->value();
                        if (value)
                            xTexMap.displacement = value;
                    }

                    if (!xTexMap.normal.empty())
                    {
                        auto *normal_image_map = xtex_group->first_node("normal.image_map");
                        if (normal_image_map)
                        {
                            auto *value = normal_image_map->value();
                            if (value)
                                xTexMap.normal = value;
                        }
                    }
                }
            }
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool _xtexToAdmf(const char *filename_, const char *admfFilePath_, int threadCount, int pngCompressLevel)
{
    if (!admfFilePath_ || !filename_ || threadCount <= 0)
        return false;
    /*
     std::string tempDirPath = replaceAll(admfFilePath_ , "\\" , "/");
     if (tempDirPath.empty())
     return false;
     
     if (tempDirPath[tempDirPath.length()- 1 ] != '/')
     tempDirPath.append("/");
     tempDirPath.append("temp");
     
     
     
     std::error_code errorCode;
     std::experimental::filesystem::remove(tempDirPath, errorCode);
     */

    ZipArchive::Ptr zipArchive = ZipFile::Open(filename_);
    if (!zipArchive)
        return false;

    //parseXML
    std::istream *stream = nullptr;
    for (int i = 0; i < zipArchive->GetEntriesCount(); i++)
    {

        auto entry = zipArchive->GetEntry(i);
        auto &name = entry->GetName();

        if (name.find("_MTL.") != std::string::npos)
        {
            g_isMetalnessPipeline = true;
            break;
        }
    }

    try
    {

        admf::ADMF admf = admf::createADMF();

        XTexMap xTexMap;
        if (!_parseXML(admf, zipArchive, filename_, xTexMap))
            return false;
        
        bool parseMaterialSuccess = _parseU3m(admf, zipArchive, filename_, xTexMap);
        if (!parseMaterialSuccess)
        {
            //parseMaterialSuccess = _parseXtex(admf, zipArchive, filename_, xTexMap);
            if (!parseMaterialSuccess)
                return false;
        }

        admf::ADMF_RESULT result = admf->saveToFile(admfFilePath_);
        return result == admf::ADMF_SUCCESS;
    }
    catch (...)
    {
        return false;
    }

    //std::string xmlContent(std::istreambuf_iterator<const char>(xmlDataStream), {});

    return true;
}
