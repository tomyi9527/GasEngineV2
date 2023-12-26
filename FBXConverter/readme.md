# 如何编译

我们提供了两种方式，cmake 路线和 非cmake路线。

但在此之前，对于 LINUX 系统，我们需要将依赖库进行安装。

## FBXSDK 在 LINUX 上的安装

参考 [安装说明](LinuxInstall.txt)

例如：

```shell
chmod ugo+x fbx20200_1_fbxsdk_linux

# 例如安装到 /usr/local 下
fbx20200_1_fbxsdk_linux /usr/local
```

## LINUX 上 gcc-8 (或更高) 编译器安装

可参考如下说明 [scl源使用说明](http://km.oa.com/group/799/articles/show/299371?kmref=search&from_page=1&no=1)

对tlinux2及更高版本来说
```shell
yum install tlinux-release-scl -y

yum search devtoolset-8-gcc-c++
# yum search devtoolset-9-gcc-c++ # 或更高版本

scl devtoolset-8 bash # 会新开一个配置好编译器环境变量的终端
# source /opt/rh/devtoolset-8/enable # 如果不想新开一个终端的话
```

## 使用cmake配置并编译

```
# 用于存储编译产物的目录
mkdir build && cd build

# 配置src目录
cmake ${PATH_TO_CMAKEFILE}

# 开始编译
make -j

```

要注意的是cmake的产物目前生成到了 ${PATH_TO_CMAKEFILE}/../../build 内。

windows同理可用 cmake-gui 配置。

## 使用非cmake配置并编译

linux: 使用makefile，但有些第三方库安装位置可能会不太一样，有可能需要手动修改。

windows: 使用 FBXConverter.sln。