//
//  admf_pipeline_internal.hpp
//  admfsdk
//
//  Created by yushihang on 2021/3/22.
//

#ifndef admf_pipeline_internal_hpp
#define admf_pipeline_internal_hpp

#include "admf_internal.h"
namespace admf_internal {
    class Custom_internal : public admf::Custom_, public Base_internal
    {
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(Custom_internal);
    public:
        virtual std::string getJson() override;
    private:
        std::string jsonString_;
        
#ifdef ADMF_EDIT
    public:
        virtual void setJson(const char* json) override {jsonString_ = json;};
#endif
    };
}
#endif /* admf_pipeline_internal_hpp */
