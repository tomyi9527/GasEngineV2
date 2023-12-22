//Author: tomyi
//Date: 2017-06-19

//Scene
GASEngine.Scene = function()
{
    this.mSceneWorldRadius = 0.0;
    this.mSceneWorldCenter = new GASEngine.Vector3(0, 0, 0);

    this._activeAnimationClips = [];    

    this._cameraLastIndex = -1;
    this._cameras = [];

    this._punctualLightLastIndex = -1;
    this._punctualLights = [];

    this._environmentalLight = null;

    this._directionalLightLastIndex = -1;
    this._directionalLights = [];

    this._pointLightLastIndex = -1;
    this._pointLights = [];

    this._spotLightLastIndex = -1;
    this._spotLights = [];

    this._skyboxList_ = [];
    this._environmentalLightList_ = [];
    this._cameraEntityList_ = [];

    //by tomyi
    this._animatorLastIndex = -1;
    this._animators = [];
    //temp 
    this._animationGlobalPlayMode = 'Play';
    this._animationGlobalSpeedFactor = 1.0;
    this._animationGlobalLoopMode = 'Loop';
    this._animationGlobalClamp = false;

    this._currentAnimationClipIndex = -1;
    this._animationUpdateCallback = () =>{};
    this._animationStartCallback = () =>{};
    //<

    this._selectedObject = null;

    this.modelRoot = null;
    this.root = GASEngine.EntityFactory.Instance.create();
    this.root.name = 'SceneRoot';
    this.root.setScene_r(this);

    GASEngine.Scene.Instance = this;
}

GASEngine.Scene.prototype = {

    constructor: GASEngine.Scene,

    getModelRoot: function()
    {
        return this.modelRoot;
    },

    setModelRoot: function(entity)
    {
        this.modelRoot = entity;
    },

    getSkyboxList: function()
    {
        return this._skyboxList_;
    },
    
    appendSkybox: function(skyboxEntity)
    {
        this._skyboxList_.push(skyboxEntity);
    },

    getEnvironmentalLightList: function()
    {
        return this._environmentalLightList_;
    },

    appendEnvironmentalLight: function(lightEntity)
    {
        this._environmentalLightList_.push(lightEntity);
    },

    getCameraEntityList: function()
    {
        return this._cameraEntityList_;
    },

    appendCameraEntity: function(cameraEntity)
    {
        this._cameraEntityList_.push(cameraEntity);
    },

    getCameraCount: function()
    {
        return (this._cameraLastIndex + 1);
    },

    getBoundingBox: function()
    {
        return this.root.bbox;
    },

    _addCamera: function(camera)
    {
        ++this._cameraLastIndex;

        if(this._cameras[this._cameraLastIndex] !== undefined)
        {
            this._cameras[this._cameraLastIndex] = camera;
        }
        else
        {
            this._cameras.push(camera);
        }
    },

    getCameras: function()
    {
        return this._cameras;
    },

    // Animation functions
    getAnimators: function()
    {
        return this._animators;
    },

    //TODO: animation controller. move to sub class (ArtHubScene)
    getAllAnimationClipNames: function()
    {
        if(this._animatorLastIndex < 0)
        {
            return [];
        }
        
        if(this._animatorLastIndex > 0)
        {
            console.error('GASEngine.Scene.getAllAnimationClipNames: The scene can only have one active animator. If more than one exist, the first is in effect.');
        }
        //TODO:
        let effectiveAnimator = this._animators[0];
        let clipNames = effectiveAnimator.getAnimationClipNameList();
        //this._currentEffetiveAnimatorClipNameList = clipNames;
        return clipNames;
    },

    getAllAnimationClipInfo: function()
    {
        if(this._animatorLastIndex < 0)
        {
            return [];
        }
        
        if(this._animatorLastIndex > 0)
        {
            console.error('GASEngine.Scene.getAllAnimationClipNames: The scene can only have one active animator. If more than one exist, the first is in effect.');
        }
        //TODO:
        let effectiveAnimator = this._animators[0];
        let clips = effectiveAnimator.getAnimationClips();
        let clipInfo = [];
        clips.forEach(e => {
            let item = {
                id: e.id,
                name: e.name,
                CF: e.CF,
                EF: e.EF,
                FPS: e.FPS,
                SF: e.SF,
                clampedSF: e.clampedSF,
                clampedEF: e.clampedEF,
                enableClamp: e.enableClamp
            };
            clipInfo.push(item);
        })
        //this._currentEffetiveAnimatorClipNameList = clipNames;
        return clipInfo;
    },

    getCurrentAnimationClipIndex: function ()
    {
        if(this._animatorLastIndex < 0)
        {
            return [];
        }
        
        if(this._animatorLastIndex > 0)
        {
            console.error('GASEngine.Scene.getAllAnimationClipNames: The scene can only have one active animator. If more than one exist, the first is in effect.');
        }
        let effectiveAnimator = this._animators[0];
        let clipIndex = effectiveAnimator.getActiveAnimationClipIndex();
        return clipIndex;
    },

    setAnimationGlobalPlayMode: function(playMode) 
    {
        if(playMode === 'Play' || playMode === 'Pause' || playMode === 'Stop')
        {
            this._animationGlobalPlayMode = playMode;
        }
        else
        {
            console.error('GASEngine.Scene.setAnimationGlobalPlayMode: Wrong play mode is set!');
        }
    },

    getAnimationGlobalPlayMode: function()
    {
        return this._animationGlobalPlayMode;
    },

    setAnimationGlobalSpeedFactor: function(speedFactor) 
    {
        if(speedFactor < 0.1 || speedFactor > 5.0)
        {
            console.error('GASEngine.Scene.setAnimationGlobalSpeedFactor: animation speed set is out of range!');
        }
        this._animationGlobalSpeedFactor = Math.clamp(speedFactor, 0.1, 5.0);
    },

    getAnimationGlobalSpeedFactor: function()
    {
        return this._animationGlobalSpeedFactor;
    },

    setAnimationGlobalLoopMode: function(loopMode)
    {
        if(loopMode === 'Loop' || loopMode === 'Shuffle' || loopMode === 'Repeat' || loopMode === 'Once')
        {
            this._animationGlobalLoopMode = loopMode;
        }
        else
        {
            console.error('GASEngine.Scene.setAnimationGlobalLoopMode: Wrong loop mode is set!');
        }
    },

    getAnimationGlobalLoopMode: function()
    {
        return this._animationGlobalLoopMode;
    },
    
    setAnimationGlobalClamp: function(flag)
    {
        this._animationGlobalClamp = flag;
    },

    getActiveAnimationProgress: function()
    {
        if(this._animatorLastIndex < 0)
        {
            return 0.0;
        }
        //TODO:
        let effectiveAnimator = this._animators[0];
        let activeAnimationClip = effectiveAnimator.getActiveAnimationClip();
        if(activeAnimationClip)
        {
            let progress = activeAnimationClip.progress;
            return progress;
        }
        else
        {
            return 0.0;
        }        
    },

    setActiveAnimationProgress: function(progress)
    {
        if(this._animatorLastIndex < 0)
        {
            return;
        }
        //TODO:
        let effectiveAnimator = this._animators[0];
        let activeAnimationClip = effectiveAnimator.getActiveAnimationClip();
        if(activeAnimationClip)
        {
            activeAnimationClip.progress = Math.clamp(progress, 0.0, 1.0);
        }
    },
    
    changeActiveAnimaitonClip: function(clipId)
    {
        if(this._animatorLastIndex < 0)
        {
            return;
        }
        let effectiveAnimator = this._animators[0];
        effectiveAnimator.play(clipId);
    },

    changeActiveAnimaitonClipByIndex: function(clipIndex)
    {
        if(this._animatorLastIndex < 0)
        {
            return;
        }
        let effectiveAnimator = this._animators[0];
        effectiveAnimator.playByIndex(clipIndex);
    },

    setAnimationUpdateCallback: function(func)
    {
        this._animationUpdateCallback = func;
    },

    setAnimationStartCallback: function(func)
    {
        this._animationStartCallback = func;
    },
    
    //< 

    _addPunctualLight: function (punctualLight)
    {
        ++this._punctualLightLastIndex;

        if(this._punctualLights[this._punctualLightLastIndex] !== undefined)
        {
            this._punctualLights[this._punctualLightLastIndex] = punctualLight;
        }
        else
        {
            this._punctualLights.push(punctualLight);
        }
    },

    _addDirectionalLight: function (directionalLight)
    {
        ++this._directionalLightLastIndex;

        if(this._directionalLights[this._directionalLightLastIndex] !== undefined)
        {
            this._directionalLights[this._directionalLightLastIndex] = directionalLight;
        }
        else
        {
            this._directionalLights.push(directionalLight);
        }
    },

    _addPointLight: function (pointLight)
    {
        ++this._pointLightLastIndex;

        if(this._pointLights[this._pointLightLastIndex] !== undefined)
        {
            this._pointLights[this._pointLightLastIndex] = pointLight;
        }
        else
        {
            this._pointLights.push(pointLight);
        }
    },

    _addSpotLight: function (spotLight)
    {
        ++this._spotLightLastIndex;

        if(this._spotLights[this._spotLightLastIndex] !== undefined)
        {
            this._spotLights[this._spotLightLastIndex] = spotLight;
        }
        else
        {
            this._spotLights.push(spotLight);
        }
    },

    _addAnimator: function (animator)
    {
        ++this._animatorLastIndex;

        if(this._animators[this._animatorLastIndex] !== undefined)
        {
            this._animators[this._animatorLastIndex] = animator;
        }
        else
        {
            this._animators.push(animator);
        }
    },

    _setEnvironmentalLight: function (environmentalLight)
    {
        this._environmentalLight = environmentalLight;
    },

    _addActiveAnimationClip: function(clip)
    {
        this._activeAnimationClips.push(clip);
    },

    _removeActiveAnimationClip: function(clip)
    {
        var found = false;
        for(var i = 0; i < this._activeAnimationClips.length; ++i)
        {
            if(this._activeAnimationClips[i] === clip)
            {
                this._activeAnimationClips.splice(i, 1);
                found = true;
                break;
            }
        }

        if(!found)
        {
            console.error('GASEngine.Scene._removeActiveAnimationClip: cannot remove a animation that is not playing!');
        }
    },    

    destroy: function()
    {
        this._activeAnimationClips.length = 0;

        this._cameraLastIndex = -1;
        this._cameras.length = 0;
    
        this._punctualLightLastIndex = -1;
        this._punctualLights.length = 0;
    
        this._environmentalLight = null;
    
        this._directionalLightLastIndex = -1;
        this._directionalLights.length = 0;
    
        this._pointLightLastIndex = -1;
        this._pointLights.length = 0;
    
        this._spotLightLastIndex = -1;
        this._spotLights.length = 0;
    
        this._skyboxList_.length = 0;
        this._environmentalLightList_.length = 0;
        this._cameraEntityList_.length = 0;

        this._animatorLastIndex = -1;
        this._animators.length = 0;

        this.root = null;
        //TODO: 需要clear hierarchy
    },

    appendEntityOnRoot: function(entity)
    {
        // entity.setScene_r(this);

        this.root.addChild(entity);
    },

    removeEntityOnRoot: function(entity)
    {
        this.root.removeChild(entity);
    },

    findObjectByPath: function(path)
    {
        var pathElements = path.split('/');

        var stack = [];
        for(var i = 0; i < this.root.getChildCount() ; ++i)
        {
            var child = this.root.getChildAt(i);
            stack.push(child);
        }

        if(pathElements.length === 1)
        {
            //only search entity name
            while(stack.length !== 0)
            {
                var e = stack.shift();
                if(e.name === pathElements[0])
                {
                    stack = [];
                    return e;
                }
                else
                {
                    for(var i = 0; i < e.getChildCount() ; ++i)
                    {
                        var child = e.getChildAt(i);
                        stack.push(child);
                    }
                }
            }

            stack = [];
            return null;
        }
        if(pathElements[0] === '' && pathElements.length > 1)
        {
            //search from the root
            var parent = this.root;
            for(var i = 1; i < pathElements.length; ++i)
            {
                parent = parent.findChildByName(pathElements[i]);
                if(parent === null)
                {
                    break;
                }
            }

            return parent;
        }
        else if(pathElements[0] !== '' && pathElements.length > 1)
        {
            //search releative path
            console.error('GASEngine.Scene.findObjectByPath: wrong searching path format!');
            return null;
        }
        else
        {
            console.error('GASEngine.Scene.findObjectByPath: wrong searching path format!');
            return null;
        }
    },

    findObjectByID: function(id, fromEntity)
    {
        if(id < 0)
        {
            return null;
        }

        if(fromEntity === undefined || fromEntity === null)
        {
            fromEntity = this.root;
        }
        var stack = [];
        stack.push(fromEntity);

        while(stack.length !== 0)
        {
            var e = stack.shift();
            if(e.uniqueID === id)
            {
                stack = [];
                return e;
            }
            else
            {
                for(var i = 0; i < e.getChildCount() ; ++i)
                {
                    var child = e.getChildAt(i);
                    stack.push(child);
                }
            }
        }

        stack = [];
        return null;
    },

    findObjectByName: function(name, fromEntity)
    {
        if(name.length === 0)
        {
            return null;
        }

        if(fromEntity === undefined || fromEntity === null)
        {
            fromEntity = this.root;
        }
        var stack = [];
        stack.push(fromEntity);

        while(stack.length !== 0)
        {
            var e = stack.shift();
            if(e.name === name)
            {
                stack = [];
                return e;
            }
            else
            {
                for(var i = 0; i < e.getChildCount() ; ++i)
                {
                    var child = e.getChildAt(i);
                    stack.push(child);
                }
            }
        }

        stack = [];
        return null;
    },

    findComponents: function(typeName)
    {
        var stack = [];
        stack.length = 256;
        stack[0] = this.root;
        stack._top = 1;

        var results = [];

        while(stack._top > 0)
        {
            var e = stack[stack._top - 1];
            stack._top -= 1;

            for (var c of e.components.values())
            {
                if(c.typeName === typeName)
                {
                    results.push(c);
                }
            }

            for(var i = 0; i < e.getChildCount() ; ++i)
            {
                var child = e.getChildAt(i);

                if(stack._top === stack.length)
                {
                    stack.length *= 2;
                }

                stack[stack._top] = child;
                stack._top++;
            }
        }

        stack = [];
        return results;
    },

    cull: (function()
    {
        var ndc = new GASEngine.Vector3();;

        return function()
        {
            if(this._cameraLastIndex < 0)
            {
                return null;
            }

            for(var i = 0; i <= this._cameraLastIndex; ++i)
            {
                this._cameras[i].clearRenderableList();
            }

            var stack = [];
            stack.length = 256;
            stack[0] = this.root;
            stack._top = 1;

            while(stack._top > 0)
            {
                var e = stack[stack._top - 1];
                stack._top -= 1;

                //if(e.MB_PROPS !== null && !e.MB_PROPS.Visibility)
                if(e.enable === false)
                {
                    continue;
                }
                //check bounding
                // this.frustum(e);
                var meshFilter = e.getComponent('meshFilter');
                if(meshFilter !== null)
                {
                    var mesh = meshFilter.getMesh();
                    //mesh can be undefined
                    // if(mesh !== null)
                    if(mesh)
                    {
                        //Calculate ndc depth for sorting
                        var matrixWorld = e.getWorldMatrix();
                        ndc.setFromMatrixPosition(matrixWorld);

                        var viewProjectionMatrix = this._cameras[0].getViewProjectionMatrix();
                        ndc.applyProjection(viewProjectionMatrix);

                        //Get materials
                        var meshRenderer = e.getComponent('meshRenderer');
                        var entityType = e.type;
                        mesh.entityType = entityType;
                        var materials = null;
                        if(meshRenderer !== null)
                        {
                            materials = meshRenderer.getMaterials()
                        }

                        if (materials && materials.length > 0) {
                            this._cameras[0].appendMesh
                            (
                                mesh,
                                materials[0],
                                this._environmentalLight,
                                this._punctualLights,
                                this._directionalLights,
                                this._pointLights,
                                this._spotLights,
                                matrixWorld,
                                ndc.z,
                                entityType
                            );
                        }

                        var subMeshes = mesh.getStream('subMesh');
                        if(!subMeshes) subMeshes = [];
                        for(var submeshIndex = 0; submeshIndex < subMeshes.length; ++submeshIndex)
                        {
                            //TODO: 增加对subMesh Stream中，materialIndex属性的支持，可以直接指定材质索引
                            var materialIndex = subMeshes[submeshIndex].materialIndex;
                            if(materialIndex === undefined) 
                            {
                                materialIndex = submeshIndex;
                            }

                            var material = (materials[materialIndex] === undefined ? null : materials[materialIndex]);
                            if(material instanceof GASEngine.HotspotMaterial)
                            {
                                this._cameras[0].addHotspotItem(mesh, material, submeshIndex);
                            }
                            else
                            {
                                if(material !== null)
                                {
                                    if(material instanceof GASEngine.CompoundMaterial)
                                    {
                                        material = material.getActiveMaterial();
                                    }                                
                                }

                                if(material !== null && material.visible === true)
                                {
                                    this._cameras[0].appendRenderables
                                    (
                                        mesh,
                                        material,
                                        submeshIndex,
                                        this._environmentalLight,
                                        this._punctualLights,
                                        this._directionalLights,
                                        this._pointLights,
                                        this._spotLights,
                                        matrixWorld,                                
                                        ndc.z,
                                        entityType
                                    );
                                }
                                //<
                            }
                        }
                        //< for loop end
                    }
                }

                // #ifdef _DEBUG
                //if(mesh !== null && materials !== null)
                //{
                //    var subMeshes = mesh.getStream('subMesh');
                //    if(subMeshes.length !== materials.length)
                //    {
                //        console.warn('GASEngine.Scene.cull: some sections lack material. Default fallback one would be adopted.');
                //    }
                //}
                // #endif            

                for(var i = 0; i < e.getChildCount() ; ++i)
                {
                    var child = e.getChildAt(i);

                    if(stack._top === stack.length)
                    {
                        stack.length *= 2;
                    }

                    stack[stack._top] = child;
                    stack._top++;
                }
            }

            return this._cameras;
        }
    })(),

    update: function(delta)
    {
        if(this._animatorLastIndex >= 0)
        {
            // TODO:
            var animator = this._animators[0];
            var clip = animator.getActiveAnimationClip();            
            if(clip)
            {
                clip.onAnimationStartCallback = this._animationStartCallback;
                clip.onAnimationUpdateCallback = this._animationUpdateCallback;
            }
        }

        //update animation
        for(var i = 0; i < this._activeAnimationClips.length; ++i)
        {
            var clip = this._activeAnimationClips[i];            
            clip.update(delta);
        }

        let clipIndex, clipCount;
        if(this._animatorLastIndex >= 0)
        {
            // TODO:
            var animator = this._animators[0];
            var clip = animator.getActiveAnimationClip();            
            if(clip)
            {
                // Speed
                clip.speed = this._animationGlobalSpeedFactor;

                // PlayMode
                if(this._animationGlobalPlayMode === 'Play')
                {
                    clip.enable = true;
                }
                else if(this._animationGlobalPlayMode === 'Pause')
                {
                    clip.enable = false;
                }
                else if(this._animationGlobalPlayMode === 'Stop')
                {
                    clip.enable = false;
                    clip.progress = 0.0;
                }
            
                // LoopMode
                if(this._animationGlobalLoopMode === 'Loop')
                {
                    if(clip.clipEnded) // Animation Over
                    {
                        clipIndex = animator.getAnimationClipIndex(clip.id);
                        clipCount = animator.getAnimationClipCount();
                        clipIndex += 1;
                        clipIndex = clipIndex >= clipCount ? 0 : clipIndex;
                        animator.playByIndex(clipIndex);
                    }
                }
                else if(this._animationGlobalLoopMode === 'Repeat')
                {
                    if(clip.clipEnded) // Animation Over
                    {
                        clip.progress = 0.0;
                    }
                }
                else if(this._animationGlobalLoopMode === 'Shuffle')
                {    
                    if(clip.clipEnded) // Animation Over
                    {
                        clip.progress = 0.0;
                        clipCount = animator.getAnimationClipCount();
                        clipIndex = Math.floor(Math.random() * clipCount);
                        clipIndex = clipIndex >= clipCount ? 0 : clipIndex;
                        animator.playByIndex(clipIndex);
                    }                
                }
                else if(this._animationGlobalLoopMode === 'Once')
                {
                    
                }

                // enable clamp
                clip.enableClamp = this._animationGlobalClamp;
            }
        }

        this._cameraLastIndex = -1;
        this._punctualLightLastIndex = -1;
        this._directionalLightLastIndex = -1;
        this._pointLightLastIndex = -1;
        this._spotLightLastIndex = -1;
        this._animatorLastIndex = -1;

        this._activeAnimationClips.length = 0;
        //update hierarchy
        this.root.update();
        
        //update camera matrix
        for(var i = 0; i <= this._cameraLastIndex; ++i)
        {
            this._cameras[i]._updateViewMatrix();
            this._cameras[i]._updateProjectionMatrix();
            this._cameras[i]._updateViewProjectionMatrix();
        }
        GASEngine.SkeletonManager.Instance.update();
    },

    pickObject: function(x, y)
    {
        if(this.getCameraCount() > 0 && this.root) 
        {
            var raycaster = new GASEngine.Raycaster();
            raycaster.setFromCamera(new GASEngine.Vector2(x, y), this._cameras[0]);
            var intersects = raycaster.intersectObject(this.root);

            if(intersects.length > 0)
            {
                this._selectedObject = intersects[0].object;
                return intersects[0];
            }
        }

        return undefined;
    },
    
    pickPosition_V2: function(x, y)
    {
        if(this.getCameraCount() > 0 && this.root)
        {
            var raycaster = new GASEngine.Raycaster();
            raycaster.setFromCamera(new GASEngine.Vector2(x, y), this._cameras[0]);

            var intersects = raycaster.intersectObjects(this.root.children);
            if(intersects.length > 0)
            {
                var interObject = intersects[0].object;
                var interPos = intersects[0].point;
                var face = intersects[0].face;
                var faceIndex = intersects[0].faceIndex;
                var skinned = undefined;
                var morphed = undefined;

                var meshFilterComponent = interObject.getComponent('meshFilter');
                var mesh = meshFilterComponent.getMesh();
                if (mesh.isMorphed())
                {
                    morphed = true;
                }
                else 
                {
                    morphed = false;
                }

                if(mesh.isSkinned())
                {
                    skinned = true;
                }
                else
                {
                    skinned = false;
                }

                if(morphed || skinned)
                {
                    var vertexElementCount = mesh.getVertexElementCount();
                    var position = mesh.getStream('skinnedPosition');
                    
                    var vertexIndexAX = face.a * vertexElementCount + 0;
                    var vertexIndexBX = face.b * vertexElementCount + 0;
                    var vertexIndexCX = face.c * vertexElementCount + 0;
                    var triA = new GASEngine.Vector3();
                    var triB = new GASEngine.Vector3();
                    var triC = new GASEngine.Vector3();
                    triA.fromArray(position, vertexIndexAX);
                    triB.fromArray(position, vertexIndexBX);
                    triC.fromArray(position, vertexIndexCX);            

                    var barycoord = GASEngine.Triangle.barycoordFromPoint(interPos, triA, triB, triC);                    

                    //var __skinnedPosition = interPos.clone();

                    var result = { 'skinned': skinned, 'morphed': morphed, 'object': interObject, 'point': interPos, 'face': face, 'barycoord': barycoord };

                    return result;
                    //var testP = new GASEngine.Vector3(0, 0, 0);
                    //testP.addScaledVector(triA, barycoord.x).addScaledVector(triB, barycoord.y).addScaledVector(triC, barycoord.z);
                }
                else
                {
                    //var __skinnedPosition = interPos.clone();
                    // interObject.updateMatrixWorld();
                    var invWorldMatrix = new GASEngine.Matrix4();
                    invWorldMatrix.getInverse(interObject.matrixWorld);
                    interPos.applyMatrix4(invWorldMatrix);

                    var result = { 'skinned': skinned, 'morphed': morphed, 'object': interObject, 'point': interPos, 'face': face };

                    return result;
                }
            }
        }

        return undefined;
    },

    //TODO:saralu
    //视锥体剔除
    //判断spot和pointLight是否影响
    //skinningMesh boundingBox变化
    frustum: function(entity)
    {
        var rootBbox = this.getBoundingBox();

        var bbox = entity.bbox;
    },

    setShadingMode: function(value) {
        this.root.traverse(function(entity) {
            if(entity.type === 'helper')
                return;

            var meshFilter = entity.getComponent('meshFilter');
            if(meshFilter) {
                var mesh = meshFilter.getMesh();
                if (mesh)
                {
                    mesh.isWireframe = (value === 'wireframe');
                }
            }

            var meshRenderer = entity.getComponent('meshRenderer');
            if(meshRenderer) {
                var materials = meshRenderer.getMaterials();
                for(var material of materials) {
                    if(material instanceof GASEngine.CompoundMaterial) {
                        material.setActiveMaterial(value);
                    }
                }
            }
        });
    },

    // enableWireframeOverlay: function (value) 
    // {
    //     this.root.traverse(function (entity) 
    //     {
    //         if (entity.type === 'helper')
    //             return;

    //         var meshFilter = entity.getComponent('meshFilter');
    //         if (meshFilter) {
    //             var mesh = meshFilter.getMesh();
    //             mesh.isWireframeOverlay = value;
    //         }
    //     });
    // },

    // enableUVLayout: function (value) {
    //     this.root.traverse(function (entity) {
    //         if (entity.type === 'helper') return;
    //         var meshFilter = entity.getComponent('meshFilter');
    //         if (meshFilter) {
    //             var mesh = meshFilter.getMesh();
    //             mesh.isshowUVLayout = value;
    //         }
    //     });
    // }
};