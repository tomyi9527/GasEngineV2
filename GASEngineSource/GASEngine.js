window.GASEngine =
{
    _RendererRevision_: '1',
    _EngineRevision_: '1'
};

function _loadGASEngineSourceFiles_(sourceRootDirectory)
{  
    var sourceFiles = 
    [
        'GASEngineSource/Base/Consts.js',
        'GASEngineSource/Base/Math.js',
        'GASEngineSource/Base/Foundation.js',
        'GASEngineSource/Base/Utilities.js',
        'GASEngineSource/Engine/Events.js',
        'GASEngineSource/Engine/FileSystem.js',
        'GASEngineSource/Engine/WebGLDevice.js',
        'GASEngineSource/Engine/WebGLRenderStates.js',
        'GASEngineSource/Engine/ShaderSourceProcessor.js',
        'GASEngineSource/Engine/WebGLShaderManager.js',
        'GASEngineSource/Engine/WebGLTextureManager.js',
        'GASEngineSource/Engine/WebGLBufferManager.js',
        'GASEngineSource/Renderer/RenderPipeline.js',
        'GASEngineSource/Renderer/PBRPipeline.js',
        'GASEngineSource/Engine/Mesh.js',
        'GASEngineSource/Engine/KeyframeAnimation.js',
        'GASEngineSource/Engine/Materials.js',
        'GASEngineSource/Engine/Texture.js',
        'GASEngineSource/Engine/MaterialMap.js',
        'GASEngineSource/Engine/Components.js',
        'GASEngineSource/Engine/Entity.js',
        'GASEngineSource/Engine/Factories.js',
        'GASEngineSource/Engine/TextureFactory.js',
        'GASEngineSource/Engine/MaterialFactory.js',        
        'GASEngineSource/Engine/Resources.js',
        'GASEngineSource/Engine/UniqueIDGenerator.js',

        'GASEngineSource/Engine/GAS1MeshDecoder.js',
        'GASEngineSource/Engine/GAS1MaterialDecoder.js',
        'GASEngineSource/Engine/GAS1KeyframeAnimationDecoder.js',        
        'GASEngineSource/Engine/GAS1Loader.js',

        'GASEngineSource/Engine/GAS2MeshDecoder.js',
        'GASEngineSource/Engine/GAS2MaterialDecoder.js',
        'GASEngineSource/Engine/GAS2KeyframeAnimationDecoder.js',
        'GASEngineSource/Engine/GAS2Loader.js',
        
        'GASEngineSource/Engine/GAS2IncrementalSaver.js',
        'GASEngineSource/Engine/Scene.js',
        
        'GASEngineSource/Engine/Input.js',
        'GASEngineSource/Engine/LightComponents.js',
        'GASEngineSource/Engine/HotspotManager.js',
        'GASEngineSource/Engine/SkeletonManager.js',
        'GASEngineSource/Engine/CameraManager.js',
        'GASEngineSource/Engine/Helper.js',
       
        'GASEngineSource/Engine/TGADecoder.js',
        'GASEngineSource/Engine/DDSDecoder.js',
        'GASEngineSource/Engine/PVRDecoder.js',

        'GASEngineSource/Engine/CameraControllerA/DelayInterpolator.js',
        'GASEngineSource/Engine/CameraControllerA/OrbitManipulatorStandardMouseKeyboardController.js',
        'GASEngineSource/Engine/CameraControllerA/OrbitManipulatorDeviceOrientationController.js',
        'GASEngineSource/Engine/CameraControllerA/OrbitManipulatorHammerController.js',
        'GASEngineSource/Engine/CameraControllerA/FirstPersonManipulatorStandardMouseKeyboardController.js',
        'GASEngineSource/Engine/CameraControllerA/AutoPilotManipulatorStandardMouseKeyboardController.js',
        'GASEngineSource/Engine/CameraControllerA/Manipulator.js',
        'GASEngineSource/Engine/CameraControllerA/OrbitManipulator.js',
        'GASEngineSource/Engine/CameraControllerA/FirstPersonManipulator.js',
        'GASEngineSource/Engine/CameraControllerA/AutoPilotManipulator.js',
        'GASEngineSource/Engine/CameraControllerA/SwitchManipulator.js',
        'GASEngineSource/Engine/CameraControllerA/StandardMouseKeyboard.js'
    ];

    for(var i = 0; i< sourceFiles.length; i++)
    {
        var url = sourceRootDirectory + sourceFiles[i];
        var newscript = document.createElement('script');
        newscript.setAttribute('type','text/javascript');
        newscript.setAttribute('src', url);
        newscript.async = false;
        document.body.appendChild(newscript);
    }    
}