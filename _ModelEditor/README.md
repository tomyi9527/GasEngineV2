## ArtHubModelEditor

### css
editor使用的css文件

### js
构建基本的场景及UI的文件，与css文件夹下的class类名一一对应

### v2
editor的主要逻辑文件，包括
- Delegates： 对engine数据的封装
- EditorModes：五种编辑模式及对应的模式切换类及基类
- WebglCommon.js：绘制gizmo相关的代码
- HistoryList.js：redo和undo时需要使用到的历史记录文件

- Editor.js：初始化editor的基本参数及函数定义

### editor.html 和 index.js
editor的入口文件




