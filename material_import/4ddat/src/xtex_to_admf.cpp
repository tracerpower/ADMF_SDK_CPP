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
#include <experimental/filesystem>
#include "ZipFile.h"

#include "rapidxml_iterators.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"

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


struct XTexMap{
    std::string diffuse;
    std::string normal;
    std::string alpha;
    std::string roughness;
    std::string displacement;
};

bool _parseU3m(const admf::ADMF& admf, const ZipArchive::Ptr& zipArchive, const std::string& filename, XTexMap& xTexMap){
    
    auto count = zipArchive->GetEntriesCount();
    
    //parseXML
    std::istream *u3mDataStream = nullptr;
    for (int i = 0; i < count; i++)
    {
        
        auto entry = zipArchive->GetEntry(i);
        auto &name = entry->GetName();
        if (hasEnding(name, ".u3m"))
        {
            u3mDataStream = entry->GetDecompressionStream();
            break;
        }
    }
    
    if (u3mDataStream == nullptr)
        return false;
    
    
    return true;
}

bool _parseXML(const admf::ADMF& admf, const ZipArchive::Ptr& zipArchive, const std::string& filename, XTexMap& xTexMap){
    
    auto count = zipArchive->GetEntriesCount();
   
    //parseXML
    std::istream *xmlDataStream = nullptr;
    for (int i = 0; i < count; i++)
    {
        
        auto entry = zipArchive->GetEntry(i);
        auto &name = entry->GetName();
        if (hasEnding(name, "_Description.xml"))
        {
            xmlDataStream = entry->GetDecompressionStream();
            break;
        }
    }
    
    if (xmlDataStream == nullptr)
        return false;
    
    
    
    admf::Material admfMaterial = admf->getMaterial();
    
    std::string xmlContent(std::istreambuf_iterator<char>(*xmlDataStream), {});
    
    const auto p1 = std::chrono::system_clock::now();
    admf::ADMF_DATE timeStamp = (admf::ADMF_DATE)std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
    admfMaterial->setCreatedTime(timeStamp);
    admfMaterial->setModifiedTime(timeStamp);
    
    auto materialLayer = admfMaterial->getLayerArray()->append();
    
    auto metadata = admfMaterial->getMetaData();
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
    metadata->getType()->setString("xtex");
    
    
    //parse XML
    rapidxml::xml_document<> doc;
    doc.parse<0>((char *)(xmlContent.c_str()));
    
    rapidxml::xml_node<> *firstNode = doc.first_node();
    
    auto *name = firstNode->name();
    assert(strcmp(name, "swatch") == 0);
    
    metadata->getType()->setString("xtex");
    
    {
        auto *node = firstNode->first_node("uuid");
        if (node)
        {
            auto *value = node->value();
            if (value){
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
            if (value){
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
                    auto* file_image_map = color_map_group->first_node("file.image_map");
                    if (file_image_map){
                        auto *value = file_image_map->value();
                        if (value)
                            xTexMap.diffuse = value;
                    }
                    
                }
                
                auto *normal_map_group = colorway_group->first_node("normal_map.group");
                if (normal_map_group)
                {
                    auto* file_image_map = normal_map_group->first_node("file.image_map");
                    if (file_image_map){
                        auto *value = file_image_map->value();
                        if (value)
                            xTexMap.normal = value;
                    }
                    
                    auto* bumpstrength_text = normal_map_group->first_node("BumpStrength.text");
                    if (bumpstrength_text){
                        auto *value = bumpstrength_text->value();
                        if (value){
                            double normalValue;
                            try {
                                normalValue = std::stoi(value);
                            }
                            catch (...){
                                normalValue = 1.;
                            }
                            materialLayer->getBasic()->getNormal()->setValue(normalValue);
                        }
                    }
                }
            }
            auto *xtex_group = node->first_node("xTex.group");
            if (xtex_group)
            {
                auto* alpha_image_map = xtex_group->first_node("alpha.image_map");
                if (alpha_image_map)
                {
                    auto *value = alpha_image_map->value();
                    if (value)
                        xTexMap.alpha = value;
                }
                
                if (!xTexMap.diffuse.empty()){
                    auto* color_image_map = xtex_group->first_node("color.image_map");
                    if (color_image_map)
                    {
                        auto *value = color_image_map->value();
                        if (value)
                            xTexMap.diffuse = value;
                    }
                }
                
                auto* displacement_image_map = xtex_group->first_node("displacement.image_map");
                if (displacement_image_map)
                {
                    auto *value = displacement_image_map->value();
                    if (value)
                        xTexMap.displacement = value;
                }
                
                if (!xTexMap.normal.empty()){
                    auto* normal_image_map = xtex_group->first_node("normal.image_map");
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
    
    try
    {
        
        
        admf::ADMF admf = admf::createADMF();
        
        XTexMap xTexMap;
        if (!_parseXML(admf, zipArchive, filename_, xTexMap))
            return false;
        if (!_parseU3m(admf, zipArchive, filename_, xTexMap))
            return false;
     
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


