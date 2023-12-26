# Getting Started
## **配置测试环境**
* git clone git@github.com:tomyi9527/GasEngineV2.git
* git checkout develop
* Install nginx and map the project directory in nginx.conf as below:
  
```nginx  
server 
{
    listen       80;
    server_name  localhost;
    
    location /gas2 
    {
    	gzip_static on;
    	proxy_buffering off;
    	alias "YourPath/GASEngineV2";
    }
}
```

* Run the sample in your web browser (chrome recommended):
http://localhost/gas2/_ModelViewer/index.html
http://localhost/gas2/_ModelEditor/editor.html?url=http://localhost/gas2/assets/airship/airship.fbx.converted/airship.fbx

* Debug the code and learn how to load a model.
* Use build/FBXConverter.exe to convert your own fbx models by simply dragging the model onto the EXE image. Advanced usage of FBXConverter.exe can be shown with -h commandline flag.

## **编译引擎JS源文件打包工具**
* compiler为GASEngine源码编译打包工具，用于打包GASEngineSource内所有源文件形成单一js文件, 并可指定文件输出路径及输出文件名。如未显式指定，默认保存路径为GASEngineV2/build/GASEnging.js。compiler还在继续开发中，未来可能会继续增加新的功能，如代码混淆优化等，为GASEngine更好的使用体验提供支持。
* comiler重新编译：
    在Linux命令行运行make指令，将使用compiler/makefile对compiler源码进行编译，输出可执行文件compiler。
```bash
make
```
* compiler执行方法：
```bash
./compiler "$YourPath/GasEngine_20200810_5.js"
```

## **编译Linux版FBX模型转换器**
* 安装FBX SDK。找到./GASEngineV2/FBXConverter/fbx20200_1_fbxsdk_linux文件，执行命令：
```bash
chmod ugo+x fbx20200_1_fbxsdk_linux
```
为fbx20200_1_fbxsdk_linux添加执行属性。执行命令：
```bash
./fbx20200_1_fbxsdk_linux /usr
```
安装FBX SDK。相关头文件会被安装到/usr/include目录，库文件被安装到/usr/lib/gcc4目录下。
* 安装uuid库。如果Linux系统默认没有安装uuid库，需要执行如下命令安装(CentOS)：
```bash
yum install libuuid libuuid-devel
```
* 进入./GASEngineV2/FBXConverter/FBXConverter目录下，执行make命令:
```bash
make
```
可执行文件默认输出Release版本，位于./GASEngineV2/build/FBXConverter_release

若要使用debug模式进行编译可执行，build变量为其他值时仍位release模式。
```bash
make build=debug
```


## **编译和使用OpenGL版GASEngine**
简介：
> 基于OpenGL的跨平台Gas2模型展示引擎，支持无显卡软件渲染模式(osmesa)。
>
> 跨平台通过cmake配置，可在linux、windows、macos上运行。

详情查看 [GASEngineSource下的readme](GASEngineSource_OpenGL/readme.md)