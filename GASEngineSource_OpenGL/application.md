# 可执行文件说明

对可执行文件使用 -help 参数以查看参数说明。

## 运行环境

**linux**
需要将以下加入LD_LIBRARY_PATH路径
* libiconv.so
* libFreeImage.so
* libosmesa.so (如果是HEADLESS_RENDER)
* llvm相关库 (如果是HEADLESS_RENDER)

**windows**
需要将以下dll放入查找路径
* FreeImage.dll
* OSMesa.dll (如果是HEADLESS_RENDER)
* libglapi.dll (如果是HEADLESS_RENDER)

## app/*

**app/App.ArthubViewer**
ArthubClient目前仅支持token方式。
可以输入本地或在线位置。
```shell
./App.ArthubViewer -input UNIVERSAL_RESOURCE_LOCATION -token TOKEN_STRING
# 例如
./App.ArthubViewer -input @@AssetHub_Atc/39730022995 -token 333151e00019
./App.ArthubViewer -input D:\render_test\GASEngineV2\assets\fbx\Bamboo_Stage3_B\gas2\Bamboo_Stage3_B.FBX.structure.json
```

**app/App.ArthubScreenShot**
ArthubClient目前仅支持token方式。
可以输入本地或在线位置。
```shell
./App.ArthubScreenShot -input UNIVERSAL_RESOURCE_LOCATION -token TOKEN_STRING -output_method METHOD
# 例如
./App.ArthubScreenShot -input @@AssetHub_Atc/39730022995 -token 333151e00019 -output_method 1
./App.ArthubScreenShot -input D:\render_test\GASEngineV2\assets\fbx\Bamboo_Stage3_B\gas2\Bamboo_Stage3_B.FBX.structure.json -output_method 0 -output test_dir/
```

**app/App.ScreenShot**
```shell
./App.Viewer -input MODEL_STRUCTURE_JSON_PATH -output OUTPUT_DIR\
# 例如
./App.Viewer -input D:\render_test\GASEngineV2\assets\fbx\Bamboo_Stage3_B\gas2\Bamboo_Stage3_B.FBX.structure.json -output=Bamboo_Stage3_B\
./App.Viewer -input /data/projects/GASEngineV2/assets/fbx/Bamboo_Stage3_B/gas2/Bamboo_Stage3_B.FBX.structure.json -output=Bamboo_Stage3_B\
```

**app/App.Viewer**
```shell
./App.Viewer -input MODEL_STRUCTURE_JSON_PATH
# 例如
./App.Viewer -input D:\render_test\GASEngineV2\assets\fbx\Bamboo_Stage3_B\gas2\Bamboo_Stage3_B.FBX.structure.json
./App.Viewer -input /data/projects/GASEngineV2/assets/fbx/Bamboo_Stage3_B/gas2/Bamboo_Stage3_B.FBX.structure.json
```
