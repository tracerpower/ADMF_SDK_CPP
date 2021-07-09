//
//  4ddat_to_admf.cpp
//  4ddat_to_admf
//
//  Created by yushihang on 2021/3/26.
//


#include "4ddat_to_admf.hpp"

#include "MaterialSerializerBase.h"
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
#include <sstream>
#include <fstream>
#include <string>
#include "changecolor.h"
#include <iomanip>

const String TexChannel_Unknown        = "Unknown";
const String TexChannel_Color        = "uTexColor";
const String TexChannel_Normal        = "uTexNormal";
const String TexChannel_Specular    = "uTexSpecular";
const String TexChannel_Gloss        = "uTexGloss";
const String TexChannel_Rough        = "uTexRoughness";
const String TexChannel_Ao            = "uTexAO";
const String TexChannel_Height        = "uTexHeight";
const String TexChannel_Fresnel        = "uTexFresnel";
const String TexChannel_Metallic    = "uTexMetallic";
const String TexChannel_Alpha        = "uTexAlpha";
const String TexChannel_SpecularLobes = "uTexSpecularLobes";
const String TexChannel_Gaussian    = "uTexGaussian";

const String TexChannel_Diffuse        = "uTexDiffuse";
const String TexChannel_Bump        = "uTexBump";

int getTextureUsedTypeByName(std::string name)
{
    using namespace RenderCore;
    int usedType = 7;
    if (name == TexChannel_Color)
    {
        usedType = MAP_COLOR;
    }
    else if (name == TexChannel_Normal)
    {
        usedType = MAP_NORMAL;
    }
    else if (name == TexChannel_Specular)
    {
        usedType = MAP_SPECULAR;
    }
    else if (name == TexChannel_Gloss)
    {
        usedType = MAP_GLOSS;
    }
    else if (name == TexChannel_Rough)
    {
        usedType = MAP_ROUGHNESS;
    }
    else if (name == TexChannel_Ao)
    {
        usedType = MAP_AO;
    }
    else if (name == TexChannel_Height)
    {
        usedType = MAP_HEIGHT;
    }
    else if (name == TexChannel_Fresnel)
    {
        usedType = MAP_FRENEL;
    }
    else if (name == TexChannel_Metallic)
    {
        usedType = MAP_METALLIC;
    }
    else if (name == TexChannel_Alpha)
    {
        usedType = MAP_ALPHA;
    }
    else if (name == TexChannel_Gaussian)
    {
        usedType = MAP_GAUSSIAN;
    }
    else if (name == TexChannel_SpecularLobes)
    {
        usedType = MAP_LOBES;
    }
    return usedType;
}



bool checkIfSrgb(int matType)
{
    bool ifSrgb = false;
    switch (matType)
    {
        case RenderCore::MAP_COLOR:
            ifSrgb = true;
            break;
        case RenderCore::MAP_SPECULAR:
            ifSrgb = true;
            break;
        default:
            ifSrgb = false;
            break;
    }
    return ifSrgb;
}

std::string replaceAll(const char *pszSrc, const char *pszOld, const char *pszNew)
{
    std::string strContent, strTemp;
    strContent.assign( pszSrc );
    std::string::size_type nPos = 0;
    while( true )
    {
        nPos = strContent.find(pszOld, nPos);
        strTemp = strContent.substr(nPos+strlen(pszOld), strContent.length());
        if ( nPos == std::string::npos )
        {
            break;
        }
        strContent.replace(nPos,strContent.length(), pszNew );
        strContent.append(strTemp);
        nPos +=strlen(pszNew) - strlen(pszOld)+1; //防止重复替换 避免死循环
    }
    return strContent;
}

admf::ADMF_RESULT materialEntryInfoToAdmf(const std::string& filename, const MaterialEntryInfo& materialEntryInfo, S4DTextureDataVec* outTextureDatas, const std::string& admfFilePath)
{
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
    else{
        sourceFileName = filename.substr(pos+1);
    }
#endif
    
    admf::ADMF admf = admf::createADMF();
    auto &matInfo = materialEntryInfo.materialInfo;
    
    admf->getSchema()->setString("1.0");
    
    admf::Material admfMaterial = admf->getMaterial();
    
    const auto p1 = std::chrono::system_clock::now();
    admf::ADMF_DATE timeStamp = (admf::ADMF_DATE)std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
    admfMaterial->setCreatedTime(timeStamp);
    admfMaterial->setModifiedTime(timeStamp);
    
    auto materialName = admfMaterial->getName();
    materialName->setString(matInfo.name.c_str());
    
    auto metadata = admfMaterial->getMetaData();
    auto metadataSource = metadata->getSource();
    
    metadataSource->setName(sourceFileName.c_str());
    metadataSource->updateFromFile(filename.c_str(), false);
    metadata->getType()->setString("4ddat");
    
    auto id = admfMaterial->getId();
    std::string version = std::to_string(matInfo.versionMajor) + "." + std::to_string(matInfo.versionMinor) + "." + std::to_string(matInfo.versionPatch);
    id->setString(version.c_str());
    
    
    
    
    auto layerArray = admfMaterial->getLayerArray();
    auto layer = layerArray->append();
    layer->setEnabled(true);
    
    
    
    auto layerType = layer->getType();
    layerType->setString(matInfo.type.c_str());
    
    int len = layerType->getLength();
    char *layerType_ = new char[len+1];
    layerType->getString(layerType_, len);
    
    auto layerBasic = layer->getBasic();
    auto transform = layerBasic->getTransform();
    
    auto normal = layerBasic->getNormal();
    auto roughness = layerBasic->getRoughness();
    auto glossiness = layerBasic->getGlossiness();
    auto alpha = layerBasic->getAlpha();
    auto metalness = layerBasic->getMetalness();
    auto specular = layerBasic->getSpecular();
    
    auto baseColor = layerBasic->getBaseColor();
    auto baseColorData = baseColor->getData();
    auto baseColorSolid = baseColorData->getSolid();
    auto baseColorSolidBlock = baseColorSolid->getBlockArray();
    auto baseColorType = baseColorData->getType(); //改色类型
    
    auto spec = layer->getSpec();
    auto specRefraction = spec->getRefraction();

 
    auto* uv0Scale = matInfo.FindPropertyVarient("uv0Scale");
    float scaleX = -1;
    float scaleY = -1;
    if (uv0Scale && uv0Scale->type == RenderCore::MVarient::VEC2)
    {
        auto transformScale = transform->getScale();
        transformScale->setX(uv0Scale->vec2.x);
        transformScale->setY(uv0Scale->vec2.y);
        scaleX = uv0Scale->vec2.x;
        scaleY = uv0Scale->vec2.y;
    }
    
    auto* uv0Offset = matInfo.FindPropertyVarient("uv0Offset");
    if (uv0Offset && uv0Offset->type == RenderCore::MVarient::VEC2)
    {
        auto transformOffset = transform->getOffset();
        transformOffset->setX(uv0Offset->vec2.x);
        transformOffset->setY(uv0Offset->vec2.y);
    }
    
    auto* uv0Rotation = matInfo.FindPropertyVarient("uv0Rotation");
    if (uv0Rotation && uv0Rotation->type == RenderCore::MVarient::FLOAT)
    {
        transform->setRotation(uv0Rotation->f);
    }
    
    
    auto* kNs = matInfo.FindPropertyVarient("kNs"); // 法线强度
    if (kNs && kNs->type == RenderCore::MVarient::FLOAT)
    {
        normal->setValue(kNs->f);
    }
    
    roughness->setValue(0.f);
    glossiness->setValue(1.0f);
    
  
    auto* kRs = matInfo.FindPropertyVarient("kRs");
    if (kRs && kRs->type == RenderCore::MVarient::FLOAT)
    {
        float kRsValue = kRs->f;
        // 类型是Metal和Gilding时含义为roughness，其他类型含义为glossiness
        if (strcmp(layerType_, "Metal") == 0 || strcmp(layerType_, "Gilding") == 0)
        {
            roughness->setValue(kRsValue);
        }
        else
        {
            glossiness->setValue(kRsValue);
        }
    }
    

    
    
    auto* kHs = matInfo.FindPropertyVarient("kHs");
    if (kHs && kHs->type == RenderCore::MVarient::FLOAT)
    {
        //todo
        //凹凸贴图的偏移量
    }
    
    alpha->setValue(1.0);
    auto* kAlpha = matInfo.FindPropertyVarient("kAlpha");
    if (kAlpha && kAlpha->type == RenderCore::MVarient::FLOAT)
    {
        alpha->setValue(kAlpha->f);
    }
    
    auto* kSaturate = matInfo.FindPropertyVarient("kSaturate");
    if (kSaturate && kSaturate->type == RenderCore::MVarient::FLOAT)
    {
        // 旧版改色，无用
    }
    
    metalness->setValue(0);
    auto* kMetallic = matInfo.FindPropertyVarient("kMetallic");
    if (kMetallic && kMetallic->type == RenderCore::MVarient::FLOAT)
    {
        metalness->setValue(kMetallic->f);
    }
    
    auto* kLuminance = matInfo.FindPropertyVarient("kLuminance");
    if (kLuminance && kLuminance->type == RenderCore::MVarient::FLOAT)
    {
        // 旧版改色，无用
    }

    bool _needAdjustColor = false;
    auto* needAdjustColor = matInfo.FindPropertyVarient("needAdjustColor");
    if (needAdjustColor && needAdjustColor->type == RenderCore::MVarient::BOOL)
    {
        if (needAdjustColor->b)
        {
            _needAdjustColor = true;
            baseColorType->setString("solid");
        }
    }

    bool _hasDiffuseColor = false;
    int _diffuseColor[3] = {128, 128, 128};
    auto* diffuseColor = matInfo.FindPropertyVarient("diffuseColor");
    if (diffuseColor && diffuseColor->type == RenderCore::MVarient::VEC3)
    {
        auto colorSpace = baseColorSolid->getColorSpace();
        colorSpace->setString("srgb");
        
        auto value = baseColorSolid->getValue();
        value->setR(diffuseColor->vec3.x);
        value->setG(diffuseColor->vec3.y);
        value->setB(diffuseColor->vec3.z);
        
        _hasDiffuseColor = true;

        _diffuseColor[0] = diffuseColor->vec3.x * 255;
        _diffuseColor[1] = diffuseColor->vec3.y * 255;
        _diffuseColor[2] = diffuseColor->vec3.z * 255;
        
    }
    auto* specularColor = matInfo.FindPropertyVarient("specularColor");
    if (specularColor && specularColor->type == RenderCore::MVarient::VEC3)
    {
        auto color = specular->getColor();
        color->setR(specularColor->vec3.x);
        color->setG(specularColor->vec3.y);
        color->setB(specularColor->vec3.z);
    }
    auto* emissiveColor = matInfo.FindPropertyVarient("emissiveColor");
    if (emissiveColor && emissiveColor->type == RenderCore::MVarient::VEC3)
    {
        //TODO
    }
    
    layerBasic->setEmissive(0);
    auto* emissiveMultiplier = matInfo.FindPropertyVarient("emissiveMultiplier");
    if (emissiveMultiplier && emissiveMultiplier->type == RenderCore::MVarient::FLOAT)
    {
        layerBasic->setEmissive(emissiveMultiplier->f);
    }
    
    specRefraction->setGlossiness(1.0f);
    auto* refractionGlossiness = matInfo.FindPropertyVarient("refractionGlossiness");
    if (refractionGlossiness && refractionGlossiness->type == RenderCore::MVarient::FLOAT)
    {
        specRefraction->setGlossiness(refractionGlossiness->f);
    }
    auto* removeColor = matInfo.FindPropertyVarient("removeColor");
    if (removeColor && removeColor->type == RenderCore::MVarient::BOOL)
    {
        //TODO
    }
    
    auto* refractionColor = matInfo.FindPropertyVarient("refractionColor");
    if (refractionColor && refractionColor->type == RenderCore::MVarient::VEC3)
    {
        auto color = specRefraction->getColor();
        color->setR(refractionColor->vec3.x);
        color->setG(refractionColor->vec3.y);
        color->setB(refractionColor->vec3.z);
    }
    
    auto* transmittanceColor = matInfo.FindPropertyVarient("transmittanceColor");
    if (transmittanceColor && transmittanceColor->type == RenderCore::MVarient::VEC3)
    {
        // 半透，暂时不对齐
    }
    
    auto* transmission = matInfo.FindPropertyVarient("transmission");
    if (transmission && transmission->type == RenderCore::MVarient::FLOAT)
    {
        // 半透，暂时不对齐
    }
    
    int mapType = RenderCore::MAP_COLOR;
    bool ifHdr = false;
    bool ifSrgb = false;
    bool ifRemoveColor = false;
    std::string mapName = "";
    S4DTextureData* tempPtr = nullptr;
    String srcFileName = matInfo.fourDdatSourceFile;
    admf::Texture texture = nullptr;

    bool hasDiffuseTexture = false;
    std::vector<std::string> tempFiles;
    if (outTextureDatas )
    {
        for (int i = 0; i < outTextureDatas->size(); i++)
        {
            auto& imageData = outTextureDatas->at(i);
            mapName = imageData.head.texName;
            mapType = getTextureUsedTypeByName(mapName);
            /*
             constexpr int MAP_COLOR = 0;
             constexpr int MAP_AO    = 1;
             constexpr int MAP_ROUGHNESS = 2;
             constexpr int MAP_METALLIC    = 3;
             constexpr int MAP_NORMAL    = 4;
             constexpr int MAP_HEIGHT    = 5;
             constexpr int MAP_SPECULAR  = 6;
             constexpr int MAP_LOBES        = 7;
             constexpr int MAP_FRENEL    = 8;
             constexpr int MAP_GAUSSIAN    = 9;     // Gaussian texture maps;
             constexpr int MAP_GLOSS = 10;
             constexpr int MAP_ALPHA = 11;
             */

            if (mapType == RenderCore::MAP_COLOR){
                hasDiffuseTexture = true;

            }

#define CASE_TEXTURE_TYPE(_type_, _object_) case RenderCore::_type_: texture = _object_->getTexture(); break;
            switch (mapType){
                CASE_TEXTURE_TYPE(MAP_COLOR, baseColor);
                CASE_TEXTURE_TYPE(MAP_ROUGHNESS, roughness);
                CASE_TEXTURE_TYPE(MAP_METALLIC, metalness);
                CASE_TEXTURE_TYPE(MAP_NORMAL, normal);
                CASE_TEXTURE_TYPE(MAP_SPECULAR, specular);
                CASE_TEXTURE_TYPE(MAP_GLOSS, glossiness);
                CASE_TEXTURE_TYPE(MAP_ALPHA, alpha);
            default:
                assert(false);
                continue;
            }
            
            auto colorSpace = texture->getColorSpace();
            if (checkIfSrgb(mapType))
                colorSpace->setString("srgb");
            else
                colorSpace->setString("linear");
            
            int w = imageData.head.imageWidth;
            int h = imageData.head.imageHeight;
            int c = imageData.head.imageChannels;
            int elementsSize = imageData.head.elementSize;
            int size = w * h * c * elementsSize;

#if (defined _WIN32) || (defined __APPLE__)
            char tempBuff[L_tmpnam + 1] = {0};
            const char *tmpFileName = std::tmpnam(tempBuff);
#else
            char tmpFileName[PATH_MAX+1] = "/tmp/admf_XXXXXX";
            int fileHandle =  mkstemp(tmpFileName);
            if (fileHandle < 0)
                continue;
            close(fileHandle);
#endif
            assert(tmpFileName);
            if (tmpFileName == nullptr)
                continue;
            
            {
                std::ofstream outfile(tmpFileName, ios::out | ios::binary);
                outfile.write((const char*)imageData.imageData, size);
            }
            
            auto binary = texture->getBinaryData();
            binary->updateFromFile(tmpFileName, true);
            texture->setWidth(w);
            texture->setHeight(h);
            if (scaleX > 0)
                texture->setPhysicalWidth(1/ scaleX);
            if (scaleY > 0)
                texture->setPhysicalHeight(1/scaleY);
            texture->setChannels(c);
            texture->setElementSize(elementsSize);

            auto dpi = texture->getDpi();
            auto dpi_x = (w * 25.4) * scaleX;
            auto dpi_y = (h * 25.4) * scaleY;
            dpi->setX(dpi_x);
            dpi->setY(dpi_y);


        
            tempFiles.emplace_back(tmpFileName);
            
            
        }
    }

    //solid colorCards
    admf::BaseColorDataSolidBlock block = baseColorSolidBlock->append();
    block->setOriginal(true);
    baseColorData->getType()->setString("solid");
    baseColorData->setIndex(0);

    if (_hasDiffuseColor)
    {
        if (_needAdjustColor || !hasDiffuseTexture)
        {
            admf::BaseColorDataSolidBlock block = baseColorSolidBlock->append();
            std::stringstream os;
            os << _diffuseColor[0] << "," << _diffuseColor[1] << "," << _diffuseColor[2];
            
            std::stringstream os_;
            os_ << "(" << os.str() << ")";
            
            block->getName()->setString(os_.str().c_str());
            block->getType()->setString("RGB");
            block->getValue()->setString(os.str().c_str());
            baseColorData->setIndex(1);
        }
    }
    
    admf::ADMF_RESULT result = admf->saveToFile(admfFilePath.c_str());
    
    for (auto& path : tempFiles)
    {
        remove(path.c_str());
    }
    tempFiles.clear();
    delete[] layerType_;
    return result;
}


bool _4ddatToAdmf(const char* filename_, const char* admfFilePath_)
{
    std::string filename(filename_);
    std::string admfFilePath(admfFilePath_);
    
    I4DBaseSerializerPtr serializer = All4DdatSerializerInterface::tryGetSerializer(filename);
    if (serializer == nullptr)
    {
        printf("serializer == nullptr\n");
        return false;
    }
    
    if (!serializer->openFile(filename))
    {
        printf("%s open failed\n", filename_);
        return false;
    }
    
    std::unique_ptr<char,function<void(char*)>> _(new char,[serializer](char*p)->void{
        serializer->release();
        delete p;
    });
    
    auto outDatasHead = serializer->getFileHeaderData();
    if (outDatasHead == nullptr)
    {
        printf("outDatasHead not found\n");
        return false;
    }
    
    assert(outDatasHead->materialNum == 1);
    
    
    auto& matEntities = serializer->getAllMaterialsEntites();
    auto* outTextureDatas = serializer->getAllTextureDatas();
    for (int i = 0; i < outDatasHead->materialNum; i++)
    {
        
        MaterialEntryInfo& materialEntryInfo = matEntities[i];
        
        auto result = materialEntryInfoToAdmf(filename, materialEntryInfo, outTextureDatas, admfFilePath);
        printf("convert %s to %s success\n", filename.c_str(), admfFilePath.c_str());
        return result == admf::ADMF_SUCCESS;
        
    }
    printf("convert %s to %s fail\n", filename.c_str(), admfFilePath.c_str());
    return false;
}
