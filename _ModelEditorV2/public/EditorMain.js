(function()
{
    function loadScript(body, url)
    {   
        var newscript = document.createElement('script');
        newscript.setAttribute('type','text/javascript');
        newscript.setAttribute('src', url);
        newscript.async = false;
        body.appendChild(newscript);
    };

    function loadScriptList(baseUrl, scriptList)
    {
        var body = document.getElementsByTagName('body')[0];
        for(var i = 0; i< scriptList.length; i++)
        {
            var url = baseUrl + scriptList[i];
            loadScript(body, url);
        }
    };

    // -------------------------------------------- Engine --------------------------------------------
    _loadGASEngineSourceFiles_('../../');
  
    // -------------------------------------------- Runtime --------------------------------------------
    loadScriptList('',         
    [
        // -- core -- //
        'Runtime/Core/Core.js',
        'Runtime/Core/Log.js',
        'Runtime/Core/Util.js',
        'Runtime/Core/Math.js',
        'Runtime/Core/Object.js',
        'Runtime/Core/Events.js',
    
        // -- Framework -- //
        'Runtime/Framework/EnginePatch.js',
        'Runtime/Framework/Application.js',
        'Runtime/Framework/AssetManager.js',
        'Runtime/WebglCommon.js',
    ]);

    // -------------------------------------------- Editor --------------------------------------------
    loadScriptList('',         
    [
        // --  Util -- //
        'Editor/Util/Const.js',
        'Editor/Util/ManagerContainer.js',

        // --  Base UI -- //
        'Editor/BaseUI/UIBase.js',
        'Editor/BaseUI/UILayout.js',
        'Editor/BaseUI/UITree.js',
        'Editor/BaseUI/UIPanel.js',
        'Editor/BaseUI/UICanvas.js',
        'Editor/BaseUI/UITextField.js',
        'Editor/BaseUI/UIButton.js',
        'Editor/BaseUI/UIStateButton.js',
        'Editor/BaseUI/UISlider.js',
        'Editor/BaseUI/UICheckBox.js',
        'Editor/BaseUI/UIPropertyItem.js',
        'Editor/BaseUI/UIVector.js',
        'Editor/BaseUI/UIOverlay.js',
        'Editor/BaseUI/UIList.js',
        'Editor/BaseUI/UIMenuList.js',
        'Editor/BaseUI/UIGridView.js',

        // --  Viewport -- //
        'Editor/Viewport/EditorApplication.js',
        'Editor/Viewport/ViewportManager.js',
        'Editor/Viewport/Viewport.js',
        'Editor/Viewport/ViewportGizmo.js',
        'Editor/Viewport/ViewportCamera.js',

        'Editor/Viewport/ViewportMode/ViewportMode_Base.js',
        'Editor/Viewport/ViewportMode/ViewportMode_Navigate.js',
        'Editor/Viewport/ViewportMode/ViewportMode_Select.js',
        'Editor/Viewport/ViewportMode/ViewportMode_AxisOp.js',
        'Editor/Viewport/ViewportMode/ViewportMode_Translate.js',
        'Editor/Viewport/ViewportMode/ViewportMode_Rotate.js',
        'Editor/Viewport/ViewportMode/ViewportMode_Scale.js',
        'Editor/Viewport/ViewportMode/ViewportModeController.js',
        
        // -- Command -- //
        'Editor/Command/CommandHistory.js',
        'Editor/Command/SelectManager.js',
        'Editor/Command/CommandManager.js',

        // -- Delegate -- //
        'Editor/Delegate/DelegateObjectBase.js',
        'Editor/Delegate/DelegateObjectManagerBase.js',
        'Editor/Delegate/DelegateMap.js',
        'Editor/Delegate/DelegateManager.js',

        'Editor/Delegate/Scene/DelegateEntity.js',
        'Editor/Delegate/Scene/DelegateComponents.js',
        'Editor/Delegate/Scene/DelegateEntityManager.js',

        'Editor/Delegate/Test/DelegateTest.js',
        'Editor/Delegate/Test/DelegateTestManager.js',

        // -- Editor UI -- //
        'Editor/EditorUI/UIAttributeGrid.js',

        // -- Controller -- //
        'Editor/EditorController/ControllerBase.js',
        'Editor/EditorController/AssetPanelController.js',
        'Editor/EditorController/AttributePanelController.js',
        'Editor/EditorController/ScenePanelController.js',
        'Editor/EditorController/EditorController.js',

        // -- Lauch -- //
        'Editor/Editor.js',
        'Editor/Lauch.js',
    ]);
}());