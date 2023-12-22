(function ()
{
    /*************************************** editor events ***************************************/
    let definePathString = function(constTable, pathString)
    {
        let spector = '.';
        let pathes = pathString.split(spector);
        let parent = constTable;
        let currentPathString = '';

        for (let i = 0;i < pathes.length;i ++)
        {
            let path = pathes[i];
            let child = parent[path];

            let midChar = (currentPathString === '') ? '' : spector;
            currentPathString += midChar;
            currentPathString += path;

            if (i == pathes.length - 1)
            {
                if (child)
                {
                    mgs.Log.error('constTable namespace props name:' + '"' + currentPathString + '"' + ' is eixsted, try set other name!');
                }
                else
                {
                    parent[path] = pathString;
                }
            }
            else
            {
                if (!child)
                {
                    child = {};
                    parent[path] = child;
                }
            }

            parent = child;
        }
    }

    let EVENTS = {};
    mgsEditor.assign('EVENTS', EVENTS);

    // init&load events
    // definePathString(EVENTS, 'onDOMContentLoaded');
    definePathString(EVENTS, 'onSceneLoaded');

    // definePathString(EVENTS, 'onAssetAdded');

    // definePathString(EVENTS, 'onAssetSelectedEnd');
    
    // definePathString(EVENTS, 'onAssetDeleted');
    // definePathString(EVENTS, 'onAssetDeleteEnd');

    // // hot keys
    // definePathString(EVENTS, 'hotkey.register');
    // definePathString(EVENTS, 'hotkey.unregister');
    // definePathString(EVENTS, 'hotkey.shift');
    // definePathString(EVENTS, 'hotkey.ctrl');
    // definePathString(EVENTS, 'hotkey.alt');
    // definePathString(EVENTS, 'hotkey.updateModifierKeys');

    // // layout
    // definePathString(EVENTS, 'editorLayout.assetTree');
    // definePathString(EVENTS, 'editorLayout.assetGrid');
    // definePathString(EVENTS, 'editorLayout.attributePanel');
    // definePathString(EVENTS, 'editorLayout.toolBar');
    // definePathString(EVENTS, 'editorLayout.viewport');
    // definePathString(EVENTS, 'editorLayout.viewTree');

    // definePathString(EVENTS, 'editorLayout.propertyPanelRoot');

    // tool bar events
    // definePathString(EVENTS, 'toolBar.onMenuClick');
    // definePathString(EVENTS, 'toolBar.onFocusClick');

    //ViewportManager
    definePathString(EVENTS, 'toolBar.onTransformSelect');
    definePathString(EVENTS, 'toolBar.onCheckboxChanged');
    definePathString(EVENTS, 'toolBar.getTransformSelected');
    definePathString(EVENTS, 'toolBar.navigateUniformStepValue');
    definePathString(EVENTS, 'toolBar.onSaveBtnClick');

    definePathString(EVENTS, 'viewport.entityClicked');
    definePathString(EVENTS, 'viewport.transformChanged');

    //sceneTreeManager
    definePathString(EVENTS, 'sceneTree.onDropHeader');
    definePathString(EVENTS, 'sceneTree.onDropInsertArea');
    definePathString(EVENTS, 'sceneTree.entitySelected');
    definePathString(EVENTS, 'sceneTree.entityDeleted');
    definePathString(EVENTS, 'sceneTree.entityCreated');
    definePathString(EVENTS, 'sceneTree.entityCopy');
    definePathString(EVENTS, 'sceneTree.entityPaste');

    //attributePanel
    definePathString(EVENTS, 'attribute.changed');
    definePathString(EVENTS, 'attribute.deleted');
    definePathString(EVENTS, 'attribute.created');

    // // history
    // definePathString(EVENTS, 'historyManager');

    // // selector
    // definePathString(EVENTS, 'selector.set');
    // definePathString(EVENTS, 'selector.onSet');
    // definePathString(EVENTS, 'selector.undo');
    
    // // assets
    // definePathString(EVENTS, 'assets.loaded');
    // definePathString(EVENTS, 'assets.assetManager');

}());