//
//  admf_custom_internal.cpp
//  admfsdk
//
//  Created by yushihang on 2021/3/22.
//

#include "admf_internal_header.h"
#include "bson.h"
#ifndef __APPLE__
#include <assert.h>
#endif
using namespace admf_internal;
using namespace admf;

void BaseColorData_internal::load(bson_iter_t *iter) //save
{

    if (iter == nullptr)
        return;

    if (!BSON_ITER_HOLDS_DOCUMENT(iter))
        return;

    bson_iter_t child;
    if (!bson_iter_recurse(iter, &child))
        return;

    std::string typeKey = getNewKey("type");
    std::string solidKey = getNewKey("solid");
    std::string multiKey = getNewKey("multi");

    while (bson_iter_next(&child))
    {
        std::string keyName = bson_iter_key(&child);
        assert(bson_iter_value(&child) != nullptr);
        //printf("Found element key: \"%s\"\n", keyName.c_str());
        if (keyName == typeKey)
        {
            type_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
        else if (keyName == solidKey)
        {
            solid_ = std::make_shared<BaseColorDataSolid_internal>(admfIndex_, &child);
        }
        else if (keyName == multiKey)
        {
            multi_ = std::make_shared<BaseColorDataMulti_internal>(admfIndex_, &child);
        }
    }
}

void BaseColorData_internal::initMissed()
{
    if (!type_)
        type_ = std::make_shared<String_internal>(admfIndex_);
    if (!solid_)
        solid_ = std::make_shared<BaseColorDataSolid_internal>(admfIndex_);
    if (!multi_)
        multi_ = std::make_shared<BaseColorDataMulti_internal>(admfIndex_);
}
#ifdef ADMF_EDIT
void BaseColorData_internal::save(bson_t* doc)
{
	std::string typeKey = getNewKey("type");
	std::string solidKey = getNewKey("solid");
	std::string multiKey = getNewKey("multi");

	ADMF_BSON_APPEND_STRING(doc, typeKey, type_);
	ADMF_BSON_APPEND_DOCUMENT(doc, solidKey, solid_);
	ADMF_BSON_APPEND_DOCUMENT(doc, multiKey, multi_);
}
#endif

String BaseColorData_internal::getType()
{
    return String(type_);
}

BaseColorDataSolid BaseColorData_internal::getSolid()
{
    return BaseColorDataSolid(solid_);
}
BaseColorDataMulti BaseColorData_internal::getMulti()
{
    return BaseColorDataMulti(multi_);
}

void BaseColorDataSolid_internal::load(bson_iter_t *iter) //save
{

    blockArray_ = std::make_shared<Array_internal<admf::BaseColorDataSolidBlock>>([this]() {
        return std::make_shared<BaseColorDataSolidBlock_internal>(admfIndex_, nullptr);
    });

    if (iter == nullptr)
        return;

    if (!BSON_ITER_HOLDS_DOCUMENT(iter))
        return;

    bson_iter_t child;
    if (!bson_iter_recurse(iter, &child))
        return;

    std::string colorSpaceKey = getNewKey("colorSpace");
    std::string valueKey = getNewKey("value");
    std::string inputBlackKey = getNewKey("inputBlack");
    std::string inputWhiteKey = getNewKey("inputWhite");
    std::string gammaKey = getNewKey("gamma");
    std::string outputBlackKey = getNewKey("outputBlack");
    std::string outputWhiteKey = getNewKey("outputWhite");
    std::string blockKey = getNewKey("block");

    while (bson_iter_next(&child))
    {
        std::string keyName = bson_iter_key(&child);
        assert(bson_iter_value(&child) != nullptr);
        //printf("Found element key: \"%s\"\n", keyName.c_str());
        if (keyName == colorSpaceKey)
        {
            colorSpace_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
        else if (keyName == valueKey)
        {
            value_ = std::make_shared<ColorRGB_internal>(admfIndex_, &child);
        }
        else if (keyName == inputBlackKey)
        {
            inputBlack_ = (ADMF_UINT)bson_iter_as_int64(&child);
        }
        else if (keyName == inputWhiteKey)
        {
            inputWhite_ = (ADMF_UINT)bson_iter_as_int64(&child);
        }
        else if (keyName == gammaKey)
        {
            gamma_ = (ADMF_FLOAT)bson_iter_as_double(&child);
        }
        else if (keyName == outputBlackKey)
        {
            outputBlack_ = (ADMF_UINT)bson_iter_as_int64(&child);
        }
        else if (keyName == outputWhiteKey)
        {
            outputWhite_ = (ADMF_UINT)bson_iter_as_int64(&child);
        }
        else if (keyName == blockKey)
        {
            if (!BSON_ITER_HOLDS_ARRAY(&child))
                continue;
            bson_t b;
            uint32_t len;
            const uint8_t *data;
            bson_iter_array(&child, &len, &data);
            if (!bson_init_static(&b, data, len))
                continue;

            bson_iter_t iter_;
            if (!bson_iter_init(&iter_, &b))
                continue;

            while (bson_iter_next(&iter_))
            {
                auto block = std::make_shared<BaseColorDataSolidBlock_internal>(admfIndex_, &iter_);
                blockArray_->pushBack(block);
            }
        }
    }
}

void BaseColorDataSolid_internal::initMissed()
{
    if (!colorSpace_)
        colorSpace_ = std::make_shared<String_internal>(admfIndex_);
    if (!value_)
        value_ = std::make_shared<ColorRGB_internal>(admfIndex_);
}
#ifdef ADMF_EDIT
void BaseColorDataSolid_internal::save(bson_t *doc)
{
    std::string colorSpaceKey = getNewKey("colorSpace");
    std::string valueKey = getNewKey("value");
    std::string inputBlackKey = getNewKey("inputBlack");
    std::string inputWhiteKey = getNewKey("inputWhite");
    std::string gammaKey = getNewKey("gamma");
    std::string outputBlackKey = getNewKey("outputBlack");
    std::string outputWhiteKey = getNewKey("outputWhite");
    std::string blockKey = getNewKey("block");

    ADMF_BSON_APPEND_STRING(doc, colorSpaceKey, colorSpace_);
    ADMF_BSON_APPEND_DOCUMENT(doc, valueKey, value_);
    ADMF_BSON_APPEND_INT32(doc, inputBlackKey, inputBlack_);
    ADMF_BSON_APPEND_INT32(doc, inputWhiteKey, inputWhite_);
    ADMF_BSON_APPEND_DOUBLE(doc, gammaKey, gamma_);
    ADMF_BSON_APPEND_INT32(doc, outputBlackKey, outputBlack_);
    ADMF_BSON_APPEND_INT32(doc, outputWhiteKey, outputWhite_);
    ADMF_BSON_APPEND_ARRAY(doc, blockKey, blockArray_, BaseColorDataSolidBlock_internal);
}
#endif
String BaseColorDataSolid_internal::getColorSpace() //"srgb",
{
    return String(colorSpace_);
}

ColorRGB BaseColorDataSolid_internal::getValue() //
{
    return ColorRGB(value_);
}

ADMF_INT BaseColorDataSolid_internal::getInputBlack() //0,
{
    return inputBlack_;
}

ADMF_INT BaseColorDataSolid_internal::getInputWhite() //255,
{
    return inputWhite_;
}

ADMF_FLOAT BaseColorDataSolid_internal::getGamma() //1.0,
{
    return gamma_;
}

ADMF_INT BaseColorDataSolid_internal::getOutputBlack() //0,
{
    return outputBlack_;
}

ADMF_INT BaseColorDataSolid_internal::getOutputWhite() //0,
{
    return outputWhite_;
}

Array<BaseColorDataSolidBlock> BaseColorDataSolid_internal::getBlockArray()
{
    return blockArray_;
}

void BaseColorDataSolidBlock_internal::load(bson_iter_t *iter) //save
{
    if (iter == nullptr)
        return;

    if (!BSON_ITER_HOLDS_DOCUMENT(iter))
        return;

    bson_iter_t child;
    if (!bson_iter_recurse(iter, &child))
        return;

    std::string nameKey = getNewKey("name");
    std::string typeKey = getNewKey("type");
    std::string valueKey = getNewKey("value");

    while (bson_iter_next(&child))
    {
        std::string keyName = bson_iter_key(&child);
        assert(bson_iter_value(&child) != nullptr);
        //printf("Found element key: \"%s\"\n", keyName.c_str());
        if (keyName == nameKey)
        {
            name_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
        else if (keyName == typeKey)
        {
            type_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
        else if (keyName == valueKey)
        {
            value_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
    }
}

void BaseColorDataSolidBlock_internal::initMissed()
{
    if (!name_)
        name_ = std::make_shared<String_internal>(admfIndex_);
    if (!type_)
        type_ = std::make_shared<String_internal>(admfIndex_);
    if (!value_)
        value_ = std::make_shared<String_internal>(admfIndex_);
}
#ifdef ADMF_EDIT
void BaseColorDataSolidBlock_internal::save(bson_t *doc)
{
    std::string nameKey = getNewKey("name");
    std::string typeKey = getNewKey("type");
    std::string valueKey = getNewKey("value");

    ADMF_BSON_APPEND_STRING(doc, nameKey, name_);
    ADMF_BSON_APPEND_STRING(doc, typeKey, type_);
    ADMF_BSON_APPEND_STRING(doc, valueKey, value_);
}
#endif
String BaseColorDataSolidBlock_internal::getName() //"(166,202,240)",
{
    return name_;
}

String BaseColorDataSolidBlock_internal::getType() //"RGB",
{
    return type_;
}

String BaseColorDataSolidBlock_internal::getValue() //"166,202,240",
{
    return value_;
}

void BaseColorDataMulti_internal::load(bson_iter_t *iter) //save
{
    blockArray_ = std::make_shared<Array_internal<admf::BaseColorDataMultiBlock>>([this]() {
        return std::make_shared<BaseColorDataMultiBlock_internal>(admfIndex_, nullptr);
    });

    if (iter == nullptr)
        return;

    if (!BSON_ITER_HOLDS_DOCUMENT(iter))
        return;

    bson_iter_t child;
    if (!bson_iter_recurse(iter, &child))
        return;


    std::string blockKey = getNewKey("block");

    while (bson_iter_next(&child))
    {
        std::string keyName = bson_iter_key(&child);
        assert(bson_iter_value(&child) != nullptr);
        //printf("Found element key: \"%s\"\n", keyName.c_str());
        if (keyName == blockKey)
        {
            if (!BSON_ITER_HOLDS_ARRAY(&child))
                continue;
            bson_t b;
            uint32_t len;
            const uint8_t *data;
            bson_iter_array(&child, &len, &data);
            if (!bson_init_static(&b, data, len))
                continue;

            bson_iter_t iter_;
            if (!bson_iter_init(&iter_, &b))
                continue;

            while (bson_iter_next(&iter_))
            {
                auto block = std::make_shared<BaseColorDataMultiBlock_internal>(admfIndex_, &iter_);
                blockArray_->pushBack(block);
            }
        }
    }
}

void BaseColorDataMulti_internal::initMissed()
{


}
#ifdef ADMF_EDIT
void BaseColorDataMulti_internal::save(bson_t *doc)
{
}
#endif


Array<BaseColorDataMultiBlock> BaseColorDataMulti_internal::getBlockArray()
{
    return blockArray_;
}



void BaseColorDataMultiBlock_internal::load(bson_iter_t *iter) //save
{
    if (iter == nullptr)
        return;

    if (!BSON_ITER_HOLDS_DOCUMENT(iter))
        return;

    bson_iter_t child;
    if (!bson_iter_recurse(iter, &child))
        return;

    std::string nameKey = getNewKey("name");
    std::string colorSpaceKey = getNewKey("colorSpace");
    std::string dpiKey = getNewKey("dpi");
    std::string widthKey = getNewKey("width");
    std::string heightKey = getNewKey("height");
    std::string valueKey = getNewKey("value");

    while (bson_iter_next(&child))
    {
        std::string keyName = bson_iter_key(&child);
        assert(bson_iter_value(&child) != nullptr);
        //printf("Found element key: \"%s\"\n", keyName.c_str());
        if (keyName == nameKey)
        {
            name_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
        else if (keyName == dpiKey)
        {
            dpi_ = std::make_shared<Vec2_internal>(admfIndex_, &child);
        }
        else if (keyName == widthKey)
        {
            width_ = (ADMF_FLOAT)bson_iter_as_double(&child);
        }
        else if (keyName == heightKey)
        {
            height_ = (ADMF_FLOAT)bson_iter_as_double(&child);
        }
        else if (keyName == colorSpaceKey)
        {
            colorSpace_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
        else if (keyName == valueKey)
        {
            value_ = std::make_shared<String_internal>(admfIndex_, &child);
        }
    }
}

void BaseColorDataMultiBlock_internal::initMissed()
{
    if (!name_)
        name_ = std::make_shared<String_internal>(admfIndex_);
    if (!colorSpace_)
        colorSpace_ = std::make_shared<String_internal>(admfIndex_);
    if (!dpi_)
        dpi_ = std::make_shared<Vec2_internal>(admfIndex_);
    if (!value_)
        value_ = std::make_shared<String_internal>(admfIndex_);
}

#ifdef ADMF_EDIT
void BaseColorDataMultiBlock_internal::save(bson_t *doc)
{
    std::string nameKey = getNewKey("name");
    std::string colorSpaceKey = getNewKey("colorSpace");
    std::string dpiKey = getNewKey("dpi");
    std::string widthKey = getNewKey("width");
    std::string heightKey = getNewKey("height");
    std::string valueKey = getNewKey("value");

    ADMF_BSON_APPEND_STRING(doc, nameKey, name_);
    ADMF_BSON_APPEND_STRING(doc, colorSpaceKey, colorSpace_);
    ADMF_BSON_APPEND_DOCUMENT(doc, dpiKey, dpi_);
    ADMF_BSON_APPEND_DOUBLE(doc, widthKey, width_);
    ADMF_BSON_APPEND_DOUBLE(doc, heightKey, height_);
    ADMF_BSON_APPEND_STRING(doc, valueKey, value_);
}
#endif


String BaseColorDataMultiBlock_internal::getName()
{
    return name_;
}
String BaseColorDataMultiBlock_internal::getColorSpace()
{
    return colorSpace_;
}

Vec2 BaseColorDataMultiBlock_internal::getDpi()
{
    return dpi_;
}

ADMF_INT BaseColorDataMultiBlock_internal::getWidth()
{
    return width_;
}

ADMF_INT BaseColorDataMultiBlock_internal::getHeight()
{
    return height_;
}

String BaseColorDataMultiBlock_internal::getValue()
{
    return value_;
}

#ifdef ADMF_EDIT

void BaseColorDataMultiBlock_internal::setWidth(ADMF_INT width)
{
    width_ = width;
}
void BaseColorDataMultiBlock_internal::setHeight(ADMF_INT height)
{
    height_ = height;
}
#endif
