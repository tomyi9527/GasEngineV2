# ChangeLog

## 待完成
* [本次不支持] Light支持（如spotLight, pointLight, PunctualLight, directionalLight等）。
* [本次不支持] shader的preprocess处理。
* [本次不支持] Scene的FindXXXX系列函数的实现。
* [待检验（本次不需支持）] uniform的Struct或arrayStruct的效果是否正确。
* [待检验（本次不需支持）] Env为panorama时的specular似乎不正常。

## 1.4.1
* [新增] 并行gif截图。
* [新增] 支持动画内容，bone显示，bbox显示，quat相关函数正确性检查。

## 1.4.0
* [新增] texture非pot长宽图像的imresize支持
* [新增] DDS(DXTTexture)读取已直接使用freeimage走普通图片的读取流程。
* [修改] component若文件拉取失败或解析失败不要直接退出。
* [新增] 非FloatTexture的bone_matrices

## 0.0.1alpha
* [新增] GasEngineV2 的初版移植程序。
* [新增] 模型 skinning 的加载。
* [新增] blinn-phong, dielectric, electric, matcap material 已完成。
* [新增] 浏览本地gas2内容，本地截图
* [新增] 浏览arthub的在线gas2内容，在线截图(使用token)

# contribution guide

合并到master的MergeRequest都要填写ChangeLog的变化。

示例格式如下：

```
## v0.0.1beta        (如果是release或测试版本)
* [新增] 支持加载 animator component 
* [修改] 修复material参数中enable不能正常生效的问题
* [删除] 模型不再支持 gas1 格式的输入

## 未发布            (如果是分支内开发功能，暂时不起标题，在下次release时再移动到最新条目下)
* [新增] 支持加载 animator component 
* [待完成] 模型尚未支持 dielectric material 的加载
```

未发布的内容内，新增修改删除为已完成待发布内容，
待完成可以写估点时的任务，表示此次距离发布还需完成的功能。