# 编译说明
## （依赖）osmesa说明

如果不需要软件渲染方式可以不进行此部分。

off-screen rendering 需要安装 osmesa 依赖，此处给出编译20.1.8时的过程，将来如果有更新版本可以自行尝试升级。

**linux** 上建议直接编译:

``` sh
# scl gcc-8
yum install devtoolset-8-gcc devtoolset-8-gcc-c++
source /opt/rh/devtoolset-8/enable 
...
# llvm
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.1/llvm-10.0.1.src.tar.xz
tar -xf llvm-10.0.1.src.tar.xz
cd llvm-10.0.1.src
mkdir build
cd build
cmake .. -DLLVM_ENABLE_RTTI=ON -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
make install
...
# 安装 meson (首先要有 python3 环境)
pip3 install meson ninja mako
...
# 升级 bison (可能需要先删除旧版)
# yum remove bison 
wget http://ftp.gnu.org/gnu/bison/bison-3.7.2.tar.xz
tar -xf bison-3.7.2.tar.xz
cd bison-3.7.2
./configure
make install
# 下载并编译 mesa
wget https://archive.mesa3d.org/mesa-20.1.8.tar.xz
tar -xf mesa-20.1.8.tar.xz
cd mesa-20.1.8
# 如果需要release build 添加 -Dbuildtype=release
meson build_llvm -Dosmesa=gallium -Dgallium-drivers=swrast,swr -Ddri-drivers=[] -Dvulkan-drivers=[] -Dprefix=/usr -Dplatforms=surfaceless,drm -Dglx=disabled
cd build_llvm
# 安装 mesa
ninja install
```

**windows** 则直接使用编译好的dll:

* 相关项目地址: [**github: mesa-dist-win**](https://github.com/pal1000/mesa-dist-win/releases)  以及 [prebuilt-binary 下载链接](https://github.com/pal1000/mesa-dist-win/releases/download/20.1.8/mesa3d-20.1.8-release-msvc.7z)

* glfw使用动态dll加载，无需编译时链接。复制dll到运行目录位置: 
    * mesa3d-20.1.8-release-msvc\x64\osmesa-gallium\OSMesa.dll
    * mesa3d-20.1.8-release-msvc\x64\libglapi.dll

## （依赖）FreeImage 说明

**linux** 上编译后安装:
```
wget http://downloads.sourceforge.net/freeimage/FreeImage3180.zip
unzip FreeImage3180.zip && cd FreeImage
make -j8
sudo make install
```

**windows** 则直接使用编译好的库:
安装包地址 [下载](http://downloads.sourceforge.net/freeimage/FreeImage3180Win32Win64.zip)
解压后放在 3rd 内即可。

## 代码说明

|目录位置|说明|
|-|-|
| src/           | GasEngine_OSR核心代码|
| src/data_types | 不直接依赖于ecs的数据结构。|
| src/ecs        | 一些ec相关的数据结构，及部分数据加载器。|
| src/opengl     | 希望把opengl相关的部分都放在这里|
| src/utils      | 一些不属于上面的代码|
| samples/       | 一些使用例子，希望供使用参考 |
| test/          | 一些测试，不用关注 |

## 代码编译

```
# 首先准备好一个支持c++17的编译器
mkdir build && cd build
# 如果需要offscreen
cmake .. -DHEADLESS_RENDER=ON
# 如果使用普通opengl窗口模式
cmake .. -DHEADLESS_RENDER=OFF
# 编译
make -j8

```

产物：

```
app/App.ScreenShot        本地截图程序
app/App.Viewer            本地查看程序
app/App.ArthubScreenShot  在线截图程序
app/App.ArthubViewer      在线查看程序
```

## 第三方库

* glad
* glfw
* glm
* osmesa
* rapidjson
* FreeImage
* gflags
* log4cplus
* cpp-httplib + openssl + zlib

TODO(beanpliu): 补充说明

## contributing

TODO(beanpliu): 补充说明
