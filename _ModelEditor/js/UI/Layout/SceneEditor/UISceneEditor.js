(function ()
{
    var style = getComputedStyle(document.body);
    let itemColor = style.getPropertyValue('--item-color');

    let UISceneEditor = function(options) 
    {
        mgs.UIPanel.call(this, options);
    
        ////////////////////////// scene editor layout //////////////////////////
        this.sceneTree = new mgs.UISceneTree
        (
            {
                id:'viewTree',
                text:'场景树',
                styleOptions:
                {
                    width: '25%', // width:'256px',
                    'border-right-width':'1px',
                },
                handlerOption:
                {
                    direction:'right',
                    min:100,
                    max:400,
                },
                foldOption:
                {
                    vertial:true,
                    buttonDirection:'Top',
                },
                childStyleOptions:
                {
                    content:
                    {
                        margin: '0px',
                        background: itemColor,
                    },
                },
            }
        );
        this.content.append(this.sceneTree);
  
        ///////////////////// scene menu ////////////////////
        this.sceneMenu = new mgs.UIMenuList
        (
            {
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
        this.content.append(this.sceneMenu);

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
            },
            // {
            //     info:
            //     {
            //         title: mgs.LANG.getString('delete'),
            //         type: 'delete'
            //     }
            // },
            {
                info:
                {
                    title: mgs.LANG.getString('create'),
                    icon:   mgs.ICONS.assetMenuListAdd,
                },
                menuListInfo:
                [
                    {
                        info:
                        {
                            title: '创建空对象',
                            type: 'create',
                            entityType: 'empty'
                        },
                    },
                    {
                        info:
                        {
                            title: '创建立方体',
                            type: 'create',
                            entityType: 'cube'
                        },
                    },
                    {
                        info:
                        {
                            title: '创建球',
                            type: 'create',
                            entityType: 'sphere'
                        },
                    },
                ],
            }
        ];
        
        this.sceneMenu.setMenuList(menuListInfo);

        // 菜单选取
        this.sceneMenu.onSelect = function(idx, item)
        {  
            if (item && item.info)
            {
                // item.info.handler();
                var type = item.info.type;
                if(type === 'copy')
                {
                    mgsEditor.emit(mgsEditor.EVENTS.sceneTree.entityCopy);
                }
                else if(type === 'paste')
                {
                    mgsEditor.emit(mgsEditor.EVENTS.sceneTree.entityPaste);
                }
                else if(type === 'delete')
                {

                }
                else if(type === 'create')
                {
                    mgsEditor.emit(mgsEditor.EVENTS.sceneTree.entityCreated, item.info.entityType);
                }   
            }
        };

        ////////////////////////// viewPort editor layout //////////////////////////
        this.viewportPanel = new mgs.UIViewportPanel(
        {
            id:'toolBarViewport',
            styleOptions:
            {
                'flex-flow':'column nowrap',
            },
            sceneEditor:this,
        });

        this.content.append(this.viewportPanel);
    };
    mgs.classInherit(UISceneEditor, mgs.UIPanel);

    UISceneEditor.prototype.getSceneTree = function()
    {
        return this.sceneTree;
    }

    UISceneEditor.prototype.getSceneMenu = function()
    {
        return this.sceneMenu;
    }

    UISceneEditor.prototype.getToolBar = function()
    {
        return this.viewportPanel.getToolBar();
    }

    UISceneEditor.prototype.getFpsElement = function()
    {
        return this.viewportPanel.getFpsElement();
    }

    UISceneEditor.prototype.getCanvas3d = function()
    {
        return this.viewportPanel.getCanvas3d();
    }

    UISceneEditor.prototype.getCanvasPanel = function()
    {
        return this.viewportPanel.getCanvasPanel();
    }
    
}());