//
//  admf_custom.h
//  admfsdk
//
//  Created by yushihang on 2021/3/20.
//

#ifndef ADMF_INCLUDED
#error "DO NOT include the header directly, use #include \"admf_cpp.h\" instead"
#endif

#ifndef admf_custom_h
#define admf_custom_h
#include <string>
namespace admf
{
    /// 用户自定义数据
    class Custom_
    {
        ///    获取Json数据
        ///    @return    返回字符串数据结构
        virtual std::string getJson() = 0;
#ifdef ADMF_EDIT
    public:
        ///    设置Json数据
        ///    @param json Json数据
        virtual void setJson(const char *json) = 0;
#endif
    };

}
#endif /* admf_custom_h */
