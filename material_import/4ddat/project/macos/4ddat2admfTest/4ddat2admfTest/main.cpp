//
//  main.cpp
//  4ddat2admfTest
//
//  Created by yushihang on 2021/3/26.
//

#include "4ddat_to_admf.hpp"
#include "exportadmf.h"
#include <string>

int main(int argc, const char * argv[]) {
    if (argc < 5)
    {
        const char* exeName = "admf_handler";
        printf("usage: %s <input file type> <input file path> <output admf path> <output dir>\n\n", exeName);
        printf("input file type:\n");
        printf("    0 : admf (output admf path will be ignored)\n");
        printf("    1 : 4ddat\n\n");
        printf("examples:\n");
        printf("    admf : %s 0 test.admf \"\" /temp/output\n", exeName);
        printf("    4ddat : %s 1 test.4ddat output.admf /temp/output\n\n", exeName);

        return 0;
    }
    

    
    int type = std::stoi(argv[1]);
    const char* inputPath = argv[2];
    const char* outputAdmf = argv[3];
    const char* extractAdmfDir = argv[4];
    printf("type = %d\n", type);
    printf("inputPath = %s\n", inputPath);
    printf("outputAdmf = %s\n", outputAdmf);
    printf("extractAdmfDir = %s\n", extractAdmfDir);
    
    
    switch (type) {
    case 0:
        extractAdmf(inputPath, extractAdmfDir);
        break;
        
    case 1:
        _4ddatToAdmf(inputPath, outputAdmf);
        extractAdmf(outputAdmf, extractAdmfDir);
        break;
        

        
        
    default:
        break;
    }

    return 0;
}
