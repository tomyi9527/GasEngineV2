# mesh文件格式解析

## overview

版本为7可以包含多个object，其他版本仅能包含1个。

### **file**

(start of file)|-
-|-
object_1| 第 1 个object
object_2| 第 2 个object
...|
object_n| 第 n 个object
(end of file)|-

可通过object的header获取长度。

### **object**

(start of object)|-
-|-
header| object的header，包含object基础信息和需要进一步加载的内容提示。
stringTable| 字符串存储。
section| 具体信息（类型、大小、偏移等）
data| 数据区
(end of object)|-

## object.header 存储方式

### **示例**

首先来看一个文件开头的例子。

```
(little-endian)
  Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
00000000: 4E 45 4F 2A CC 98 00 00 78 56 34 12 07 00 00 00    NEO*L...xV4.....
00000010: A0 00 00 00 A4 00 00 00 48 01 00 00 4D 45 53 48    ....$...H...MESH
00000020: 00 00 00 00 00 00 00 00 00 00 00 00 00 01 A3 BE    ..............#>
00000030: 1F 05 89 41 6C AF 95 3E 00 01 A3 3E 21 0A 8B 41    ...Al/.>..#>!..A
00000040: 3E 22 5E 3F A9 30 36 3F 91 0A 63 3E B4 76 7B 3F    >"^?)06?..c>4v{?
00000050: 22 54 29 3F 74 00 00 00 FF FF 7F 7F FF FF 7F 7F    "T)?t...........
00000060: 00 00 80 00 00 00 80 00 00 00 00 00 00 00 00 00    ................
00000070: 00 00 00 00                                        ....
```
[0x00, 0x74)为header区域

十进制为[0, 116)

### **属性列表**

起始位置为object的偏移量。

property| range_hex | range_dec | length | type | value_in_example | value_in_type | requirement | 描述
-|-|-|-|-|-|-|-|-
fileFlag | [0x00,0x04) | [0, 4) | 4 | char[4] | 4E 45 4F 2A | - | - | file_flag
objectSize | [0x04,0x08) | [4, 8) | 4 | uint32 | CC 98 00 00 | 39116 | version==7 <sup>#1</sup> | 当前header后的object的size为 39116
endianFlag | [0x08,0x0C) | [8, 12) | 4 | uint32 | 78 56 34 12 | 0x12345678 | - | 表示存储方式，此处存储固定值0x12345678，通过layout判断是否为little-endian。
version | [0x0C,0x10) | [12, 16) | 4 | uint32 | 07 00 00 00 | 7 | - | 文件版本为 v7
sectionOffset | [0x10,0x14) | [16, 20) | 4 | uint32 | A0 00 00 00 | 160 | - | section块开始的位置偏移（相对于此header起始位置）
parentUniqueID| [0x14,0x18) | [20, 24) | 4 | uint32 | A4 00 00 00 | 164 | - | 父对象的 uniqueId 为 164 (即0xA4)
meshUniqueID| [0x18,0x1C) | [24, 28) | 4 | uint32 | 48 01 00 00 | 328 | - | 当前对象的 uniqueId 为 328 (即0x148)
objectType| [0x1C,0x28) | [28, 40) | 12 | char[12] | 4D 45 53 48 00 ... | MESH | - | CAMERA MESH LIGHT 中一个字符串
nameIndex | [0x28,0x2C) | [40, 44) | 4 | uint32 | 00 00 00 00 | 0 | stringTable含有此index | 表示模型名为stringTable内第nameIndex个字符串
posBboxMin | [0x2C,0x38) | [44, 56) | 12 | float32[3] | 00 01 A3 BE 1F 05 89 41 6C AF 95 3E | [?, ?, ?] | - | 表示坐标包围盒的最小值点
posBboxMax | [0x38,0x44) | [56, 68) | 12 | float32[3] | 00 01 A3 3E 21 0A 8B 41 3E 22 5E 3F | [?, ?, ?] | - | 表示坐标包围盒的最大值点
uv0BboxMin | [0x44,0x4C) | [68, 76) | 8 | float32[2] | A9 30 36 3F 91 0A 63 3E | [?, ?] | - | 表示贴图坐标0包围盒的最小值点
uv0BboxMax | [0x4C,0x54) | [76, 84) | 8 | float32[2] | B4 76 7B 3F 22 54 29 3F | [?, ?] | - | 表示贴图坐标0包围盒的最大值点
stringTableOffset | [0x54,0x58) | [84, 88) | 4 | uint32 | 74 00 00 00 | 116 | - |stringTable块开始的位置偏移（相对于此header起始位置）
uv1BboxMin | [0x58,0x60) | [88, 96) | 8 | float32[2] | FF FF 7F 7F FF FF 7F 7F | [?, ?] | - | 表示贴图坐标1包围盒的最小值点
uv1BboxMax | [0x60,0x68) | [96, 104) | 8 | float32[2] | 00 00 80 00 00 00 80 00 | [?, ?] | - | 表示贴图坐标1包围盒的最大值点
md5 | [0x68,0x74) | [104, 116) | 12 | char[12] | - | - | - | 可留全零


<br>

<sup>#1</sup> *objectSize在version!=7时长度为文件长度，即认为只有一组object*

```c++
// FILE_FLAG
// const char* FILE_BEGINNING_FLAG = "NEO*";
// const char* FILE_END_FLAG = "*OEN";
struct BIN_HEADER
{
	char				FILE_FLAG[4];
	unsigned int		OBJECT_FILE_SIZE;
	unsigned int		ENDIAN_FLAG;
	unsigned int		VERSION;
	unsigned int		SECTION_TABLE_OFFSET;
	int					PARENTUNIQUEID;
	int					UNIQUEID;			//Offset 24bytes	
	char				OBJECTTYPE[12];		//CAMERA  MESH	LIGHT
	unsigned int		OBJECT_NAME_INDEX;
	float				POS_BBOX_MIN[3];
	float				POS_BBOX_MAX[3];
	float				UV0_BBOX_MIN[2];
	float				UV0_BBOX_MAX[2];
	unsigned int		STRING_TABLE_OFFSET;
	float				UV1_BBOX_MIN[2];
	float				UV1_BBOX_MAX[2];
	char				MD5[12];
};
```

## object.stringTable 存储方式

### **stringTable**
似乎最后一个总是为长为0的字符串。
(start of stringTable)|type|bytes|-
-|-|-|-
count| uint32 | 4 | stringTable中的字符串数量
stringItem_1 | stringItem | - | 第1个字符串
stringItem_2 | stringItem | - | 第2个字符串
... |
stringItem_k | stringItem | - | 第k个字符串
(end of stringTable)|

<br>

### **stringItem**
(start of stringItem)|type|bytes|-
-|-|-|-
size| uint32 | 4 | string数据的字符数，不含'\0'，等同strlen()结果。
data| char[size+1]| size + 1 | string数据区，含结尾的'\0'
(end of stringItem)|


## object.section 存储方式

### **section**
(start of section)| type | bytes | -
-|-|-|-
count| uint32 | 4 | section数量
sectionItem_1 | sectionItem | 28 | 第1个sectionItem
sectionItem_2 | sectionItem | 28 | 第2个sectionItem
... |
sectionItem_k | sectionItem | 28 | 第k个sectionItem
(end of section)|

<br>

### **sectionItem**
(start of sectionItem)|type|bytes|-
-|-|-|-
sectionType | uint32 | 4 | section_enum (0-18)
dataAttribute | uint32 | 4 | kMESH_ATTRIBUTE_LOOSE or encoding_type
sectionOffset | uint32 | 4 | memory start offset in file
sectionLength | uint32 | 4 | memory length for this section
AttributeCount | uint32 | 4 | 共有多少个attribute
elementCount | uint32 | 4 | 每个attribute会有多少个element
elementType | uint32 | 4 | 元素类型 FLOAT, UINT, STRUCT, UBYTE  (0-3)
(end of sectionItem)|

## object.data

此处存储内容由section内的参数引用即可。
section_offset处是一个固定长16字节的字符串，用于表明此section数据代表的含义。
如 POSITION NORMAL0 等


(start of section)| type | bytes | -
-|-|-|-
type | char[16] | 16 | section数据类型
data|byte[?]|?|格式和长度取决于相应section记录内的dataAttribute、xxxCount、xxxType等属性。
(end of section)|

16 + elementCount * attributeCount * sizeof(elementType) 应与 sectionLength 相等




