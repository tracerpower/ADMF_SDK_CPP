//
//  admf_material.h
//  admfsdk
//
//  Created by yushihang on 2021/3/20.
//

#ifndef ADMF_INCLUDED
#error "DO NOT include the header directly, use #include \"admf_cpp.h\" instead"
#endif

#ifndef admf_pipeline_h
#define admf_pipeline_h

namespace admf
{

    /// 贴图集合类
    class TextureContainer
    {
    public:
        ///     获取贴图
        ///    @return    返回贴图
        virtual Texture getTexture() = 0;
        ///     获取贴图类型
        ///    @return    返回贴图类型
        virtual TEX_TYPE getTextureType() = 0;
    };

    /// 贴图集合和值集合类
    class TextureAndValueContainer : public TextureContainer
    {
    public:
        ///     获取贴图值
        ///    @return    返回贴图值
        virtual ADMF_FLOAT getValue() = 0;
#ifdef ADMF_EDIT
    public:
        ///     设置贴图值
        ///     @param value 贴图值
        virtual void setValue(ADMF_FLOAT value) = 0;
#endif
    };

    /// 基本颜色类
    class BaseColor_ : public TextureContainer
    {
    public:
        ///     获取基本颜色数据
        ///    @return    返回基本颜色数据
        virtual BaseColorData getData() = 0;
        virtual BaseColorChangeColorData getChangeColorData() = 0;

    };

    /// 法线贴图类
    class Normal_ : public TextureAndValueContainer
    {
    };
    /// Alpha贴图类
    class Alpha_ : public TextureAndValueContainer
    {
    };
    /// 金属贴图合类
    class Metalness_ : public TextureAndValueContainer
    {
    };
    /// 粗糙贴图类
    class Roughness_ : public TextureAndValueContainer
    {
    };
    /// 高光贴图类
    class Specular_ : public TextureContainer
    {
    public:
        ///     获取颜色
        ///    @return    返回颜色
        virtual ColorRGB getColor() = 0;
    };
    /// 光泽贴图类
    class Glossiness_ : public TextureAndValueContainer
    {
    };
    
    /// 各向异性
    class Anisotropy_ : public TextureAndValueContainer
    {
    };
    
    /// 各向异性
    class AnisotropyRotation_ : public TextureAndValueContainer
    {
    };
    
    /// 自发光
    class Emissive_ : public TextureAndValueContainer
    {
    };

}
#endif /* admf_material_h */
