(function ()
{
    let UIEditorLayout = function (options) 
    {
        mgs.UILayout.call(this, options);
        
        options = options ? options : {};


        ///////////////menulist////////////////////////////
        let menuHeight = '0px';
        let globalMenu = new mgs.UIMenuList
        (
            {
                id: 'globalMenu',
                text: '菜单',
                styleOptions: 
                {
                    height: menuHeight,
                    width: '100%',
                    position: 'fixed'
                },
                onAppend:function(item)
                {
                    if (item.info.icon)
                    {
                        let icon = document.createElement('div');
                        icon.classList.add('UIAssetMenuItem-icon');
                        icon.innerHTML = item.info.icon;
                        item.appendElement(icon);
                    }

                    let title = document.createElement('div');
                    title.classList.add('UIAssetMenuItem-title');
                    title.innerHTML = item.info.title;
                    item.appendElement(title);
                },
            }
        );

        let menuListInfo =
        [
            {
                info:
                {
                    title: mgs.LANG.getString('copy'),
                    type: 'copy'
                }
            },
            {
                info:
                {
                    title: mgs.LANG.getString('paste'),
                    type: 'paste'
                }
            }
        ];
        globalMenu.setMenuList(menuListInfo);
        globalMenu.show(true);
        // this.append(globalMenu);

        /////////////////////////////////////// create childs ///////////////////////////////////////
        let leftPart = new mgs.UILayout
        (
            {
                id:'leftPart',
                styleOptions:
                {
                    'flex-flow':'column nowrap',
                    'margin-top': menuHeight
                }
            }
        );
        this.append(leftPart);

        // UISceneEditor
        this.sceneEditor = new mgs.UISceneEditor
        (
            {
                id:'sceneEditor',
                text:'场景编辑器',
                childStyleOptions:
                {
                    content:
                    {
                        margin: '0px',
                        display: 'flex',
                        'flex-flow':'row nowrap',
                    },
                    header:
                    {
                        height : '0px',
                    },
                }
            }
        );
        leftPart.append(this.sceneEditor);

        // UIAssetPanel
        this.assetPanel = new mgs.UIAssetPanel
        (

            {
                id:'assetPanel',
                text:'资源',
                styleOptions:
                {
                    height:'30%', // height:'313px',
                    'border-top-width':'1px',
                },
                handlerOption:
                {
                    direction:'top',
                    min:100,
                    // max:500,
                },
                childStyleOptions:
                {
                    content:
                    {
                        margin: '0px',
                        display: 'flex',
                        'flex-flow':'row nowrap',
                    },
                    header:
                    {
                        height:'25px',
                    },
                },
                foldOption:
                {
                    horizontal:true,
                }
            }
        );
        leftPart.append(this.assetPanel);
        ////////////////////////////leftPart end//////////////////////

        // UIAttributePanel
        this.attributePanel = new mgs.UIAttributePanel
        (
            {
                id:'attributePanel',
                text:'属性',
                styleOptions:
                {
                    width: '30%', // width:'330px',
                    'border-left-width':'1px',
                    'margin-top': menuHeight
                },

                childStyleOptions:
                {
                    content:
                    {
                        padding:'5px 5px 15px 5px',
                        margin:'0px',
                        color: 'white',
                        'overflow-x': 'hidden'
                    },
                },

                handlerOption:
                {
                    direction:'left',
                    min:100,
                    // max:500,
                },
                foldOption:
                {
                    vertial:true,
                    buttonDirection:'Bottom',
                },
            }
        );
        this.append(this.attributePanel);
    };
    mgs.classInherit(UIEditorLayout, mgs.UILayout);

    UIEditorLayout.prototype.getAttributePanel = function()
    {
        return this.attributePanel;
    }

    UIEditorLayout.prototype.getAssetPanel = function()
    {
        return this.assetPanel;
    }

    UIEditorLayout.prototype.getSceneTree = function()
    {
        return this.sceneEditor.getSceneTree();
    }

    UIEditorLayout.prototype.getSceneMenu = function()
    {
        return this.sceneEditor.getSceneMenu();
    }

    UIEditorLayout.prototype.getToolBar = function()
    {
        return this.sceneEditor.getToolBar();
    }

    UIEditorLayout.prototype.getFpsElement = function()
    {
        return this.sceneEditor.getFpsElement();
    }

    UIEditorLayout.prototype.getCanvas3d = function()
    {
        return this.sceneEditor.getCanvas3d();
    }

    UIEditorLayout.prototype.getCanvasPanel = function()
    {
        return this.sceneEditor.getCanvasPanel();
    }
    
}());