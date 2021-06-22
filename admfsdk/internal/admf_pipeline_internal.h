//
//  admf_custom_internal.hpp
//  admfsdk
//
//  Created by yushihang on 2021/3/22.
//

#ifndef admf_custom_internal_hpp
#define admf_custom_internal_hpp

#include "admf_internal.h"
#include <memory>
namespace admf_internal {

    class BaseColorData_internal;
    class Texture_internal;
    class ColorRGB_internal;

    class BaseColor_internal : public admf::BaseColor_, public Base_internal
    {
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(BaseColor_internal);
    public:
        virtual admf::BaseColorData getData() override;
        virtual admf::Texture getTexture() override;
        virtual admf::TEX_TYPE getTextureType() override {return admf::TEX_TYPE_BASE;};
    private:
        std::shared_ptr<BaseColorData_internal> data_;
        std::shared_ptr<Texture_internal> texture_;
    };

    class Specular_internal: public admf::Specular_, public Base_internal
    {
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(Specular_internal);
    public:
        virtual admf::Texture getTexture() override;
        virtual admf::ColorRGB getColor() override;
        virtual admf::TEX_TYPE getTextureType() override {return admf::TEX_TYPE_SPECULAR;};
    private:
        std::shared_ptr<Texture_internal> texture_;
        std::shared_ptr<ColorRGB_internal> color_;

    };

    
#ifdef ADMF_EDIT
#define SetValue_Declaration()  virtual void setValue(admf::ADMF_FLOAT value) override {value_ = value;};
#else
#define SetValue_Declaration()
#endif
    

#define TextureAndValueContainer_Internal_Declaration(classname, basename, texType) \
    class classname: public admf::basename, public Base_internal   \
    { \
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(classname);    \
    public: \
        virtual admf::Texture getTexture() override; \
        virtual admf::ADMF_FLOAT getValue() override; \
        virtual admf::TEX_TYPE getTextureType() override {return admf::texType;};    \
    private:   \
        std::shared_ptr<Texture_internal> texture_; \
        admf::ADMF_FLOAT value_ = 0;\
    public: \
        SetValue_Declaration() \
    };


    TextureAndValueContainer_Internal_Declaration(Normal_internal, Normal_, TEX_TYPE_NORMAL)
    TextureAndValueContainer_Internal_Declaration(Alpha_internal, Alpha_, TEX_TYPE_ALPHA)
    TextureAndValueContainer_Internal_Declaration(Metalness_internal, Metalness_, TEX_TYPE_METALNESS)
    TextureAndValueContainer_Internal_Declaration(Roughness_internal, Roughness_, TEX_TYPE_ROUGHNESS)
    TextureAndValueContainer_Internal_Declaration(Glossiness_internal, Glossiness_, TEX_TYPE_GLOSSINESS)


}
#endif /* admf_custom_internal_hpp */