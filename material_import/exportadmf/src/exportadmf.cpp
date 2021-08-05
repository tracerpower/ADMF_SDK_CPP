//
//  exportadmf.c
//  exportadmf
//
//  Created by yushihang on 2021/7/1.
//

#include <stdio.h>
#include <string>
#include <iostream>


#include <sstream>
#include <fstream>
#if (defined _WIN32) || (defined __APPLE__)
#include <filesystem>
#else
#include <iomanip> 
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#endif
#include "FreeImage/FreeImage.h"
#include "changecolor/changecolor.h"
#include "admf_cpp.h"
#include "admf_internal_header.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    void exportChangeColor(const std::string& path, const CHANGE_COLOR::Result& result)
    {
        try {
            std::ofstream out(path);
            out << "{" << std::endl;
            out << "\t\"hasDiffuseMap\": true," << std::endl;
            out << "\t\"bottomS\": " << std::setprecision(20) << result.bottomS << "," << std::endl;
            out << "\t\"bottomV\": " << std::setprecision(20) << result.bottomV << "," << std::endl;
            out << "\t\"meanS\": " << std::setprecision(20) << result.meanS << "," << std::endl;
            out << "\t\"meanV\": " << std::setprecision(20) << result.meanV << "," << std::endl;
            out << "\t\"kS\": " << std::setprecision(20) << result.kS << "," << std::endl;
            out << "\t\"kV\": " << std::setprecision(20) << result.kV  << std::endl;
            out << "}" << std::endl;
            out.close();
            
            
            
            
        }
        catch (...) {
            
        }
    }
    
    bool ExportImageDataToFile(const unsigned char* buffer, const std::string& destPath, int width, int height, int channel, const int elementSize) {
        FREE_IMAGE_TYPE imageType = FIT_UNKNOWN;
        
        switch (channel)
        {
        case 1:
            imageType = FIT_BITMAP;
            break;
        case 2:
            imageType = FIT_BITMAP;
            break;
        case 3:
            if (elementSize == 1) {
                imageType = FIT_BITMAP;
            }
            else if (elementSize == 2) {
                imageType = FIT_RGB16;
            }
            else if (elementSize == 4) {
                imageType = FIT_RGBF;
            }
            break;
        case 4:
            if (elementSize == 1) {
                imageType = FIT_BITMAP;
            }
            else if (elementSize == 2) {
                imageType = FIT_RGBA16;
            }
            else if (elementSize == 4) {
                imageType = FIT_RGBAF;
            }
            break;
        default:
            break;
        }
        
        FIBITMAP* bitmap = FreeImage_ConvertFromRawBitsEx(true, (BYTE*)buffer, imageType,
                                                          width, height, width * channel * elementSize,
                                                          channel * elementSize * 8, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                                                          true);
        
        if (!bitmap) {
            return false;
        }
        
        //FreeImage_FlipHorizontal(bitmap);
        FreeImage_FlipVertical(bitmap);
        auto info = FreeImage_GetInfo(bitmap);
        bool bSuccess = FreeImage_Save(FIF_PNG, bitmap, destPath.c_str(), PNG_DEFAULT);
        FreeImage_Unload(bitmap);
        return true;
    }
    
    bool hasSuffix(std::string const &fullString, std::string const &suffix) {
        if (fullString.length() >= suffix.length()) {
            return (0 == fullString.compare (fullString.length() - suffix.length(), suffix.length(), suffix));
        } else {
            return false;
        }
    }
    
    void extractLayer(const std::string& pathName,  const admf::MaterialLayer& layer, const std::string& layerIndex)
    {
        if (!layer->isEnabled())
            return;
        
        auto basic = layer->getBasic();
        
        std::vector<admf::Texture> textureVector;
        
        auto vectorLambda = [&textureVector](const admf::Texture& texture) {
            if (texture)
            {
                auto binary = texture->getBinaryData();
                if (binary)
                {
                    auto len = binary->getDataLength();
                    if (len > 0)
                    {
                        textureVector.push_back(texture);
                        return true;
                    }
                }
                
            }
            return false;
        };
        auto needExportDiffuse = vectorLambda(basic->getBaseColor()->getTexture());
        //这句必须在最前
        vectorLambda(basic->getAlpha()->getTexture());
        vectorLambda(basic->getNormal()->getTexture());
        vectorLambda(basic->getSpecular()->getTexture());
        vectorLambda(basic->getMetalness()->getTexture());
        vectorLambda(basic->getRoughness()->getTexture());
        vectorLambda(basic->getGlossiness()->getTexture());
        
        for (auto& texture : textureVector)
        {
            auto binaryData = texture->getBinaryData();
            if (!binaryData)
                continue;
            
            admf_internal::BinaryData_internal* binaryData_internal = dynamic_cast<admf_internal::BinaryData_internal*>(binaryData.get());
            
            auto name = binaryData_internal->getRawName();
            char* nameBuff = new char[name->getLength()+1];
            name->getString(nameBuff, name->getLength()+1);
            auto texturePath = pathName + "/" + nameBuff;
            
            
            admf_internal::Texture_internal* texture_internal = dynamic_cast<admf_internal::Texture_internal*>(texture.get());
            auto textureBinaryType = texture_internal->getTypeByBinaryData();
            auto ext = texture_internal->getExtensionByTextureFileType(textureBinaryType);
            
            assert(hasSuffix(texturePath, ext));
            
            
            auto dataLen = binaryData->getDataLength();
            unsigned char* dataBuff = (unsigned char*)malloc(dataLen);
            binaryData->getData(dataBuff, dataLen);
            
            if (needExportDiffuse)
            {
                CHANGE_COLOR::Result result = CHANGE_COLOR::changeColor(dataBuff, texture->getWidth(), texture->getHeight(), texture->getChannels());
                needExportDiffuse = false;
                exportChangeColor(pathName + "/changeColor" + layerIndex + ".json", result);
            }
            
            if (textureBinaryType != admf::TextureFileType::RAW)
            {
                auto myfile = std::fstream(texturePath, std::ios::out | std::ios::binary);
                myfile.write((char*)dataBuff, dataLen);
                myfile.close();
            }
            else
            {
                ExportImageDataToFile((unsigned char*)dataBuff, texturePath, texture->getWidth(), texture->getHeight(), texture->getChannels(), texture->getElementSize());
            }
 
            free(dataBuff);
            dataBuff = nullptr;
            //binaryData->exportToFile(texturePath.c_str());
            delete[] nameBuff;
        }
    }
    
    
    
    bool extractAdmf(const char* admfFilePath, const char* dir_)
    {
        printf("\nexport %s to dir %s\n", admfFilePath, dir_);
        if (admfFilePath == nullptr || dir_ == nullptr || strlen(admfFilePath) == 0 || strlen(dir_) == 0)
            return false;
        std::string dir(dir_);
#if (defined __APPLE__) || (defined _WIN32)
#ifdef __APPLE__
        namespace fs = std::__fs::filesystem;
#else
        namespace fs = std::filesystem;
#endif
        if (!fs::exists(dir))
        {
            printf("dir %s not exist\n", dir_);
            return false;
        }
        
#else
        struct stat st;
        if(stat(dir_,&st) == 0)
        {
            if(((st.st_mode) & S_IFMT) != S_IFDIR)
                return false;
        }
        else
            return false;
#endif
        FreeImage_Initialise();
        if (dir[dir.length()-1] == '\\' || dir[dir.length()-1] == '/')
            dir.pop_back();
        
        admf::ADMF_RESULT result;
        admf_internal::ADMF_internal::ADMFJsons admfJsons;
        auto admf = admf_internal::ADMF_internal::loadFromFile(admfFilePath, result, &admfJsons);
        if (!admf)
        {
            printf("admf %s open failed\n", admfFilePath);
            return false;
        }
        
        
        {
            std::ofstream jsonFile(dir + "/material.json");
            jsonFile << admfJsons.material;
        }
        {
            std::ofstream jsonFile(dir + "/custom.json");
            jsonFile << admfJsons.custom;
        }
        {
            std::ofstream jsonFile(dir + "/geometry.json");
            jsonFile << admfJsons.geometry;
        }
        {
            std::ofstream jsonFile(dir + "/physics.json");
            jsonFile << admfJsons.physics;
        }
        std::string layersPath = dir;
        /*
         std::string layersPath = dir + "/textures";
         #ifdef _WIN32
         _mkdir(layersPath.c_str());
         #else
         mkdir(layersPath.c_str(), 0777);
         #endif
         */
        auto material = admf->getMaterial();
        auto layerArray = material->getLayerArray();
        auto layersCount = layerArray->size();
        for (int i = 0; i < layersCount; i++) {
            auto layer = layerArray->get(i);
            extractLayer(layersPath, layer, i==0 ? "":std::to_string(i).c_str());
        }
        
        auto sideLayer = material->getSideLayer();
        extractLayer(layersPath,  sideLayer, "Side");
        
        //admf->getMaterial()->getMetaData()->getSource()->exportToFile((dir + "/orig.4ddat").c_str());
        FreeImage_DeInitialise();
        printf("\nexport %s to dir %s finished\n", admfFilePath, dir_);
        return true;
    }
    
#ifdef __cplusplus
}
#endif
