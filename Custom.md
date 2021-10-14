# Custom

代码示例

```cpp
admf::ADMF_RESULT result;
admf::ADMF::ADMFJsons admfJsons;
auto admf = admf::ADMF::loadFromFile(admfFilePath, result, &admfJsons);

auto custom = admf->getCustom();
auto& valueMap = custom->getValueMap();
auto& version = valueMap["4dstc.version"];
//时谛内部自定义字段 用于对应ADMF/4ddat在RHI里的shader版本


auto& version = valueMap["4dstc.changeColor"];
//时谛内部自定义字段 用于保存diffuse贴图里的改色信息

```

Custom 格式 <font color="gray">(各厂商可自行加入字段, key/value 只支持字符串)</font>

```jsonc
{
  "4dstc.changeColor": "{\n\t\"version\": 2,\n\t\"hasDiffuseMap\": true,\n\t\"bottomS\": 0,\n\t\"bottomV\": 20,\n\t\"meanS\": 0,\n\t\"meanV\": 143.426239013671875,\n\t\"kS\": 1,\n\t\"kV\": 0.84313726425170898438\n}\n",
  //时谛内部自定义字段 用于保存diffuse贴图里的改色信息

  "4dstc.version": "1.9.0"
  //时谛内部自定义字段 用于对应ADMF/4ddat在RHI里的shader版本
}
```
