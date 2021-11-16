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

#include <string.h>

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

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


bool _xtexToAdmf(const char* filename_, const char* admfFilePath_, int threadCount, int pngCompressLevel)
{
    if (!admfFilePath_ || !filename_ || threadCount <= 0 )
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
    
    auto count = zipArchive->GetEntriesCount();
    std::vector<ZipArchiveEntry::Ptr> entryVector(zipArchive->GetEntriesCount());
    
   
    for (int i=0; i< count; i++ ){
        
        auto entry = zipArchive->GetEntry(i);
        if (entry == nullptr)
            continue;
        
        entryVector.push_back(entry);
    
    }
    
    
    std::istream* xmlDataStream = nullptr;
    //find the first xml
    for (auto& entry : entryVector)
    {
        auto& name = entry->GetName();
        if (hasEnding(name, "_Description.xml"))
        {
            xmlDataStream = entry->GetDecompressionStream();
            break;
        }
    }
    
    
    if (xmlDataStream == nullptr)
        return false;
    
    //std::string xmlContent(std::istreambuf_iterator<const char>(xmlDataStream), {});
    
    
    return true;
}


