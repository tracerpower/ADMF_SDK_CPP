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
    class BaseColorChangeColorData_internal;

    class BaseColor_internal : public admf::BaseColor_, public Base_internal
    {
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(BaseColor_internal);
    public:
        virtual admf::BaseColorData getData() override;
        virtual admf::Texture getTexture() override;
        virtual admf::TEX_TYPE getTextureType() override {return admf::TEX_TYPE_BASE;};
        //virtual admf::BaseColorChangeColorData getChangeColorData() override;
    private:
        std::shared_ptr<BaseColorData_internal> data_;
        std::shared_ptr<Texture_internal> texture_;
        //std::shared_ptr<BaseColorChangeColorData_internal> changeColor_;
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
    
    
    class Emissive_internal: public admf::Emissive_, public Base_internal
    {
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(Emissive_internal);
    public:
        virtual admf::Texture getTexture() override;
        virtual admf::TEX_TYPE getTextureType() override {return admf::TEX_TYPE_EMISSIVE;};
        virtual admf::ColorRGB getColor() override;
        virtual admf::ADMF_FLOAT getValue() override {return value_;};
#ifdef ADMF_EDIT
        virtual void setValue(admf::ADMF_FLOAT value) override {value_ = value;}
#endif
        
    private:
        std::shared_ptr<Texture_internal> texture_;
        std::shared_ptr<ColorRGB_internal> color_;
        admf::ADMF_FLOAT value_ = 0.0;
    };
    
    
    
    class AmbientOcclusion_internal: public admf::AmbientOcclusion_, public Base_internal
    {
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(AmbientOcclusion_internal);
    public:
        virtual admf::Texture getTexture() override;
        virtual admf::TEX_TYPE getTextureType() override {return admf::TEX_TYPE_AO;};
        
    private:
        std::shared_ptr<Texture_internal> texture_;

    };
    
    
    /// Height
    class Height_internal: public admf::Height_, public Base_internal
    {
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(Height_internal);

        
    public:
        virtual admf::Texture getTexture() override;
        virtual admf::TEX_TYPE getTextureType() override {return admf::TEX_TYPE_HEIGHT;};
        virtual admf::ADMF_FLOAT getValue() override {return value_;};
        virtual admf::ADMF_FLOAT getLevel() override  {return level_;};

#ifdef ADMF_EDIT
    public:
        virtual void setValue(admf::ADMF_FLOAT value) override {value_ = value;};
        virtual void setLevel(admf::ADMF_FLOAT level) override {level_ = level;};
#endif
        
    private:
        std::shared_ptr<Texture_internal> texture_;
        admf::ADMF_FLOAT value_ = 1.0;
        admf::ADMF_FLOAT level_ = 1.0;
    };


    
#ifdef ADMF_EDIT
#define SetValue_Declaration()  virtual void setValue(admf::ADMF_FLOAT value) override {value_ = value;};
#else
#define SetValue_Declaration()
#endif
    

#define TextureAndValueContainer_Internal_Declaration(classname, basename, texType, defaultValue) \
    class classname: public admf::basename, public Base_internal   \
    { \
        ADMF_INTERNAL_CLASS_CONSTRUCTOR(classname);    \
    public: \
        virtual admf::Texture getTexture() override; \
        virtual admf::ADMF_FLOAT getValue() override; \
        virtual admf::TEX_TYPE getTextureType() override {return admf::texType;};    \
    private:   \
        std::shared_ptr<Texture_internal> texture_; \
        admf::ADMF_FLOAT value_ = defaultValue;\
    public: \
        SetValue_Declaration() \
    };


    TextureAndValueContainer_Internal_Declaration(Normal_internal, Normal_, TEX_TYPE_NORMAL, 1.0f)
    TextureAndValueContainer_Internal_Declaration(Alpha_internal, Alpha_, TEX_TYPE_ALPHA, 1.0f)
    TextureAndValueContainer_Internal_Declaration(Metalness_internal, Metalness_, TEX_TYPE_METALNESS, 1.0f)
    TextureAndValueContainer_Internal_Declaration(Roughness_internal, Roughness_, TEX_TYPE_ROUGHNESS, 1.0f)
    TextureAndValueContainer_Internal_Declaration(Glossiness_internal, Glossiness_, TEX_TYPE_GLOSSINESS, 1.0f)
    TextureAndValueContainer_Internal_Declaration(Anisotropy_internal, Anisotropy_, TEX_TYPE_ANISOTROPY, 0.0f)
    TextureAndValueContainer_Internal_Declaration(AnisotropyRotation_internal, AnisotropyRotation_, TEX_TYPE_ANISOTROPY_ROTATION, 0.0f)



}
#endif /* admf_custom_internal_hpp */
