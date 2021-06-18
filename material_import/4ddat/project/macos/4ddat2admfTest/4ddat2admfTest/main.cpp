//
//  main.cpp
//  4ddat2admfTest
//
//  Created by yushihang on 2021/3/26.
//

#include "4ddat_to_admf.hpp"
#include <string>

int main(int argc, const char * argv[]) {
    // insert code here...
    
    std::string fourDdatCompressedFilePath = "/Volumes/Data/plugin/4ddat/orange.4Ddat";
    //fourDdatCompressedFilePath = "/Users/yushihang/Documents/plugin/4ddat/baseWhite.4Ddat";
    std::string destFolderPath = "/Users/yushihang/Documents/未命名文件夹/temp/4ddat/1.admf";
    bool success = _4ddatToAdmf(fourDdatCompressedFilePath.c_str(), destFolderPath.c_str());
    extractAdmf(destFolderPath.c_str(), "/Users/yushihang/Documents/未命名文件夹/temp/4ddat/export");
    return 0;
}
