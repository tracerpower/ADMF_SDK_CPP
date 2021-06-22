//
//  main.cpp
//  4ddat2admfTest
//
//  Created by yushihang on 2021/3/26.
//

#include "4ddat_to_admf.hpp"
#include <string>

int main(int argc, const char * argv[]) {
    
    if (argc < 4)
    {
        printf("usage: %s <input 4ddat path> <output admf path> <output dir>\n", argv[0]);
        return 0;
    }
    std::string fourDdatCompressedFilePath = argv[1];//R"(D:\temp\20-0073TPM.4Ddat)";
    std::string destFolderPath = argv[2]; // R"(D:\temp\1111\1.admf)";
    bool success = _4ddatToAdmf(fourDdatCompressedFilePath.c_str(), destFolderPath.c_str());
    if (success)
    {
        extractAdmf(destFolderPath.c_str(), argv[3]);//R"(D:\temp\1111\1)");
    }
    return 0;
}
