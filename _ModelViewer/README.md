# Getting Started
1. cd _ArtHubModelEditor_V2
2. tnpm install

3. node bin/www<br>
或者: 在VSCode中打开项目，设置launch.json如下：

{
    // Use IntelliSense to learn about possible attributes.<br>
    // Hover to view descriptions of existing attributes.<br>
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387<br>
    "version": "0.2.0",<br>
    "configurations": [<br>
        {<br>
            "type": "node",<br>
            "request": "launch",<br>
            "name": "Launch Program",<br>
            "program": "${workspaceFolder}\\_ArtHubModelEditor_V2\\bin\\www"<br>
        }<br>
    ]<br>
}<br>


运行后，则自动开启Web Server在3321，根目录为GasEngine_V2的目录。

```
* Run the sample in your web browser (chrome recommended):
http://localhost:3321/_ArthubModelViewer/index.html