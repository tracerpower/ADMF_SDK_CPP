//
//  admf_custom_internal.cpp
//  admfsdk
//
//  Created by yushihang on 2021/3/22.
//

#include "admf_internal_header.h"
#include "bson.h"
using namespace admf_internal;

std::string Custom_internal::getJson()
{
    return jsonString_;
}

void Custom_internal::load(bson_iter_t *iter) //save
{
    if (iter == nullptr)
        return;

    if (BSON_ITER_HOLDS_DOCUMENT(iter))
    {
        bson_t b;
        uint32_t len;
        const uint8_t *data;
        bson_iter_document(iter, &len, &data);

        if (bson_init_static(&b, data, len))
        {
            char *json = bson_as_relaxed_extended_json(&b, NULL);
            jsonString_ = json;
        }
    }
}

void Custom_internal::initMissed()
{
}
#ifdef ADMF_EDIT
void Custom_internal::save(bson_t *doc)
{
}
#endif
