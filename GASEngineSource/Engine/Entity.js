//Author: tomyi
//Date: 2017-06-19

//Entity
//DO NOT NEW THIS OBJECT DIRECTLY!
GASEngine.Entity = function()
{
    this.guid = GASEngine.generateUUID();    
    this._uniqueID = -1;
    this.name = 'Entity';
    this.mutable = false;

    this.parent = null;
    this.children = [];

    this.components = new Map();
    this.matrixLocal = new GASEngine.Matrix4();
    this.matrixWorld = new GASEngine.Matrix4();

    this.translation = new GASEngine.Vector3(0, 0, 0);
    this._rotation_ = new GASEngine.Euler(0, 0, 0);
    this._quaternion_ = new GASEngine.Quaternion();
    this.scale = new GASEngine.Vector3(1, 1, 1);

    this.MB_PROPS = null;
    this.MAX_PROPS = null;
    this._enable_ = true;

    this.bbox = new GASEngine.AABB();

    this._scene = null;
    
    this.type = undefined; //helper, camera, light and others, now just set helper type!
}

GASEngine.Entity.MAX_UNIQUE_ID = -1;

GASEngine.Entity.prototype =
{
    constructor: GASEngine.Entity,

    get uniqueID()
    {
        return this._uniqueID;
    },

    set uniqueID(val)
    {
        if(GASEngine.Entity.MAX_UNIQUE_ID < val)
        {
            GASEngine.Entity.MAX_UNIQUE_ID = val;
        }

        this._uniqueID = val;
    },

    clone: function()
    {
        var newEntity = GASEngine.EntityFactory.Instance.create();

        newEntity.uniqueID = this.uniqueID;
        newEntity.name = this.name;
        newEntity.parent = this.parent;

        newEntity.matrixLocal.copy(this.matrixLocal);
        newEntity.matrixWorld.copy(this.matrixWorld);

        newEntity.translation.copy(this.translation);
        newEntity._rotation_.copy(this._rotation_);
        newEntity._quaternion_.copy(this._quaternion_);
        newEntity.scale.copy(this.scale);

        // newEntity.type = this.type;
        // newEntity.translation.copy(this.translation);
        // newEntity._rotation_.copy(this._rotation_);
        // newEntity.scale.copy(this.scale);
        return newEntity;
    },

    destroy: function()
    {
        this.setScene_r(null);
        this.parent = null;

        for (var c of this.components.values()) 
        {
            GASEngine.ComponentFactory.Instance.destroy(c);
        }

        this.components = null;

        for(var i = 0; i < this.children.length; ++i)
        {
            var child = this.children[i];
            GASEngine.EntityFactory.Instance.destroy(child);
        }

        this.children.length = 0;
    },

    get rotation()
    {
        return this._rotation_;
    },

    get quaternion()
    {
        return this._quaternion_;
    },

    set rotation(value)
    {
        this._rotation_.copy(value);
        this._quaternion_.setFromEuler(value, false);
    },

    set quaternion(value)
    {
        this._quaternion_.copy(value);
        this._rotation_.setFromQuaternion(value, undefined, false);
    },

    get enable()
    {
        return this._enable_;
    },

    set enable(value)
    {
        this._enable_ = value;
    },

    getLocalTranslation: (function()
    {
        var localTranslation = new GASEngine.Vector3();
        return function()
        {
            localTranslation.copy(this.translation);
            return localTranslation;
        };
    }()),

    getLocalRotation: (function()
    {
        var localRotation = new GASEngine.Euler();
        return function()
        {
            localRotation.copy(this._rotation_);
            return localRotation;
        }
    }()),

    getLocalQuaternion: (function()
    {
        var localQuaternion = new GASEngine.Quaternion();
        return function()
        {
            localQuaternion.copy(this._quaternion_);
            return localQuaternion;
        }
    }()),

    getLocalScale: (function()
    {
        var localScale = new GASEngine.Vector3();
        return function()
        {
            localScale.copy(this.scale);
            return localScale;
        }
    }()),


    getWorldMatrix: function()
    {
        return this.matrixWorld;
    },

    getWorldTranslation: (function()
    {
        var worldTranslation = new GASEngine.Vector3();

        return function()
        {
            worldTranslation.copy(this.translation);

            if (this.parent)
            {
                worldTranslation.applyMatrix4(this.parent.matrixWorld);
            }

            return worldTranslation;
        };
    }()),

    getWorldRotation: (function()
    {
        var worldRotation = new GASEngine.Euler();

        return function()
        {
            worldRotation.setFromQuaternion(this.getWorldQuaternion());
            return worldRotation;
        };
    }()),

    getWorldQuaternion: (function()
    {
        var worldQuaternion = new GASEngine.Quaternion();
        var mulRot = new GASEngine.Quaternion();
        var parentLocalRot = new GASEngine.Quaternion();

        return function()
        {
            worldQuaternion.setFromEuler(this._rotation_);

            var parent = this.parent;
            while(parent)
            {
                parentLocalRot.setFromEuler(parent._rotation_);
                GASEngine.Utilities.scaleMulQuat(parent.scale, worldQuaternion, worldQuaternion);
                mulRot.copy(parentLocalRot).multiply(worldQuaternion);
                worldQuaternion.copy(mulRot);

                parent = parent.parent;
            }

            return worldQuaternion;
        };
    }()),

    getWorldScale: (function()
    {
        return function()
        {
            return this.matrixWorld.getScale();
        };
    }()),

    setLocalTranslation: function(translation)
    {
        this.translation.copy(translation);
    },

    setLocalRotation: function(value)
    {
        this._rotation_.copy(value);
        this._quaternion_.setFromEuler(value);
    },

    setLocalQuaternion: function(value)
    {
        this._quaternion_.copy(value);
        this._rotation_.setFromQuaternion(value, 'XYZ');
    },

    setLocalScale: function(scale)
    {
        this.scale.copy(scale);
    },

    setLocalMatrix: function()
    {
        var localTranslation = new GASEngine.Vector3();
        var localQuaternion = new GASEngine.Quaternion();
        var localScale = new GASEngine.Vector3();

        return function(matrix) {
            matrix.decompose(localTranslation, localQuaternion, localScale);
            this.setLocalTranslation(localTranslation);
            this.setLocalQuaternion(localQuaternion);
            this.setLocalScale(localScale);
        }
    }(),

    worldPositionToLocal: function()
    {
        return function(worldPositoin, outLocalPosition)
        {
            outLocalPosition.copy(worldPositoin);

            var parent = this.parent;
            if (parent)
            {
                GASEngine.Utilities.InverseTransformPosition(parent, outLocalPosition);
            }
        };
    }(),

    setWorldTranslation:(function()
    {
        return function(worldTranslation)
        {
            this.worldPositionToLocal(worldTranslation, this.translation);
        };
    })(),

    worldRotationToLocal: function()
    {
        var quat = new GASEngine.Quaternion();
        return function(worldRotation, outLocalRotation)
        {
            quat.setFromEuler(worldRotation);

            var parent = this.parent;
            if (parent)
            {
                GASEngine.Utilities.InverseTransformRotation(parent, quat);
            }

            outLocalRotation.setFromQuaternion(quat);
        };
    }(),

    setWorldRotation: (function()
    {
        return function(worldRotation)
        {
            this.worldRotationToLocal(worldRotation, this._rotation_);
        };
    })(),

    setWorldScale: (function()
    {
        var tmpVec = new GASEngine.Vector3();
        var epsilon = 0.0001;

        return function(scale)
        {
            tmpVec.copy(scale);
            if(this.parent)
            {
                var parentScale = this.parent.matrixWorld.getScale();

                var x = Math.abs(parentScale.x) < epsilon ? tmpVec.x : tmpVec.x / parentScale.x;
                var y = Math.abs(parentScale.y) < epsilon ? tmpVec.y : tmpVec.y / parentScale.y;
                var z = Math.abs(parentScale.z) < epsilon ? tmpVec.z : tmpVec.z / parentScale.z;
                tmpVec.set(x, y, z);
            }
            this.scale.copy(tmpVec);
        };
    })(),
    
    getScene: function()
    {
        return this._scene;
    },

    setScene_r: function(scene)
    {
        this._scene = scene;

        for(var component of this.components)
        {
            component.scene = this._scene;
        }

        for(var i = 0; i < this.children.length; i++)
        {
            this.children[i].setScene_r(scene);
        }
    },

    addComponent: function(component)
    {
        if(component.parentEntity !== null)
        {
            console.error('GASEngine.Entity.removeComponent: cannot add a component which already belong to another entity!');
            return false;
        }

        var typeName = component.typeName;
        this.components.set(typeName, component);
        component.parentEntity = this;
        component.scene = this._scene;

        return true;
    },

    removeComponent: function(typeName)
    {
        if(this.components.has(typeName))
        {
            var component = this.components.get(typeName);

            if(component.parentEntity !== this)
            {
                console.error('GASEngine.Entity.removeComponent: cannot remove a component that do not belong to the entity!');
                return false;
            }

            component.parentEntity = null;
            component.scene = null;
            this.components.delete(typeName);

            return true;
        }
        else
        {
            console.error('Attempt to remove a non-existent component type!');
            return false;
        }
    },

    getComponent:function(typeName)
    {
        var value = this.components.get(typeName);
        if(value === undefined)
        {
            return null;
        }
        else
        {
            return value;
        }
    },

    getMeshList_r: function(meshList)
    {
        const meshFilterComponent = this.getComponent('meshFilter');
        if(meshFilterComponent) {
            const mesh = meshFilterComponent.getMesh();
            if(mesh) {
                meshList.push({parentEntity: this, mesh});
            }
        }

        for(let i = 0, len = this.children.length; i < len; i++)
        {
            let child = this.children[i];
            child.getMeshList_r(meshList);
        }
    },

    getMaterialList_r: function(materialList)
    {
        const meshRendererComponent = this.getComponent('meshRenderer');
        if(meshRendererComponent) {
            const materials = meshRendererComponent.getMaterials();
            if(materials.length > 0) {
                materialList.push({parentEntity: this, materials});
            }
        }

        for(let i = 0, len = this.children.length; i < len; i++)
        {
            let child = this.children[i];
            child.getMaterialList_r(materialList);
        }
    },

    getChildCount: function()
    {
        return this.children.length;
    },

    getChildAt: function(index)
    {
        if(index < 0 || index >= this.children.length)
            return null;

        return this.children[index];
    },

    findChildByName: function(name)
    {
        for(var i = 0; i < this.children.length; ++i)
        {
            var child = this.children[i];
            if(child.name === name)
            {
                return child;
            }            
        }
        return null;
    },

    findChildEntityByName_r: function(name)
    {
        var stack = [];
        stack.length = 256;
        stack[0] = this;
        stack._top = 1;

        while(stack._top > 0)
        {
            var e = stack[stack._top - 1];
            stack._top -= 1;

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

                    if(stack._top === stack.length)
                    {
                        stack.length *= 2;
                    }

                    stack[stack._top] = child;
                    stack._top++;
                }
            }
        }
        
        stack = [];
        return null;
    },

    findChildByID: function(id)
    {
        for(var i = 0; i < this.children.length; ++i)
        {
            var child = this.children[i];
            if(child.uniqueID === id)
            {
                return child;
            }
        }
        return null;
    },

    findObjectByID_r: function(id)
    {
        var stack = [];
        stack.length = 256;
        stack[0] = this;
        stack._top = 1;

        while(stack._top > 0)
        {
            var e = stack[stack._top - 1];
            stack._top -= 1;

            for (var c of e.components.values()) 
            {
                if(c.uniqueID === id)
                {
                    stack = [];
                    return c; //This is a trick for component search!
                }
            }

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

                    if(stack._top === stack.length)
                    {
                        stack.length *= 2;
                    }

                    stack[stack._top] = child;
                    stack._top++;
                }
            }
        }
        
        stack = [];
        return null;
    },

    findChild: function(entity)
    {
        for(var i = 0; i < this.children.length; ++i)
        {
            var child = this.children[i];
            if(child === entity)
            {
                return i;
            }
        }

        return -1;
    },

    addChild: function(entity, next)
    {
        var index = this.findChild(entity);
        if(index === -1)
        {
            entity.parent = this;
            entity.setScene_r(this._scene);

            if(next)
            {
                var nextIndex = this.findChild(next);
                if(nextIndex === -1)
                {
                    console.error('GASEngine.Entity.addChild: the next is not in the child list.');
                    return;
                }
                this.children.splice(nextIndex, 0, entity);
            }
            else 
            {
                this.children.push(entity);
            }
        }
        else
        {
            console.error('GASEngine.Entity.addChild: the entity has been in the child list.');
        }
    },

    removeChild: function(entity)
    {
        var index = this.findChild(entity);
        if(index > -1)
        {
            this.children.splice(index, 1);
            entity.setScene_r(null);

            entity.parent = null;
        }
        else
        {
            console.error('GASEngine.Entity.addChild: can not remove a entity which is not in the child list.');
        }
    },

    setParent: function()
    {
        var worldTranslation = new GASEngine.Vector3();
        var worldRotation = new GASEngine.Euler();
        var worldScale = new GASEngine.Vector3();
        var epsilon = 0.0001;

        return function(parent, next, isKeepWorld)
        {
            worldTranslation.copy(this.getWorldTranslation());
            worldRotation.copy(this.getWorldRotation());
            worldScale.copy(this.getWorldScale());

            if(this.parent !== null)
            {
                this.parent.removeChild(this);
            }

            parent.addChild(this, next);
    
            if(isKeepWorld)
            {
                this.setWorldTranslation(worldTranslation);
                this.setWorldRotation(worldRotation);

                worldScale.x = Math.abs(worldScale.x) < epsilon ? this.scale.x : worldScale.x;
                worldScale.y = Math.abs(worldScale.y) < epsilon ? this.scale.y : worldScale.y;
                worldScale.z = Math.abs(worldScale.z) < epsilon ? this.scale.z : worldScale.z;
                this.setWorldScale(worldScale);
            }
        }
    }(),

    updateLocalMatrix: function()
    {
        if (this.MAX_PROPS !== null&& this.MAX_PROPS !== undefined) 
        {
            // here we do not apply offset matrix before recursively set the children's matrix
            this.matrixLocal.composeMaxMatrixQuaternion(this.MAX_PROPS, this.translation, this._quaternion_, this.scale, false);  
        }
        else if(this.MB_PROPS !== null && this.MB_PROPS !== undefined)
        {
            this.matrixLocal.composeMBMatrixQuaternion(this.MB_PROPS, this.translation, this._quaternion_, this.scale);
            //this.matrixLocal.composeMBMatrix(this.MB_PROPS, this.translation, this._rotation_, this.scale);
        }
        else
        {
            this.matrixLocal.composeQuaternion(this.translation, this._quaternion_, this.scale);
            //this.matrixLocal.compose(this.translation, this._rotation_, this.scale);
        }
    },

    updateMaxMatrix: function() {
        // here we apply offset matrix after recursively set the children's matrix
        if (this.MAX_PROPS !== null&& this.MAX_PROPS !== undefined) 
        {
            // here we do not apply offset matrix before recursively set the children's matrix
            this.matrixLocal.multiply(this.MAX_PROPS.__PREMULT_0);

            // and update after set offset matrix
            if(this._enable_ === true)
            {
                if(this.parent === null)
                {
                    this.matrixWorld.copy(this.matrixLocal);
                }
                else
                {
                    this.matrixWorld.multiplyMatrices(this.parent.matrixWorld, this.matrixLocal);
                }
            }
        }
    },

    updateWorldMatrix_r: (function()
    {
        var box = new GASEngine.Box();
        var position = new GASEngine.Vector3();
        var rotation = new GASEngine.Euler();
        var scale = new GASEngine.Vector3();

        return function()
        {
            if(this.MB_PROPS !== null)
            {
                if((this.MB_PROPS.VisibilityInheritance === true && this.parent.MB_PROPS !== null))
                {
                    this.MB_PROPS.Visibility = this.parent.MB_PROPS.Visibility;
                    this._enable_ = this.parent.MB_PROPS.Visibility;
                }
            }

            if(this._enable_ === true)
            {
                this.updateLocalMatrix();
            }

            var animator = this.getComponent('animator');
            if(animator !== null)
            {
                var activeAnimationClip = animator.getActiveAnimationClip();
                if(activeAnimationClip !== null)
                {
                    this._scene._addActiveAnimationClip(activeAnimationClip);
                }
                this._scene._addAnimator(animator);
            }

            var camera = this.getComponent('camera');
            if(camera !== null)
            {
                this._scene._addCamera(camera);
            }

            var environmetalLight = this.getComponent('environmentalLight');
            if(environmetalLight !== null)
            {
                this._scene._setEnvironmentalLight(environmetalLight);
            }

            var punctualLight = this.getComponent('punctualLight');
            if(punctualLight !== null)
            {
                this._scene._addPunctualLight(punctualLight);
            }


            var directionalLight = this.getComponent('directionalLight');
            if(directionalLight !== null)
            {
                this._scene._addDirectionalLight(directionalLight);
            }

            var pointLight = this.getComponent('pointLight');
            if(pointLight !== null)
            {
                this._scene._addPointLight(pointLight);
            }

            var spotLight = this.getComponent('spotLight');
            if(spotLight !== null)
            {
                this._scene._addSpotLight(spotLight);
            }
        
            if(this._enable_ === true)
            {
                if(this.parent === null)
                {
                    this.matrixWorld.copy(this.matrixLocal);
                }
                else
                {
                    this.matrixWorld.multiplyMatrices(this.parent.matrixWorld, this.matrixLocal);
                }
            }

            for(var i = 0; i < this.children.length; i++)
            {
                this.children[i].updateWorldMatrix_r();
            }

            // update max offset matrix
            this.updateMaxMatrix();

            // update bbox
            var worldTranslation = this.getWorldTranslation();
            
            var meshFilter = this.getComponent('meshFilter');
            if(meshFilter !== null && meshFilter.bbox !== null)
            {
                box.fromAABB(meshFilter.bbox);
                box.transform(this.matrixWorld);
                this.bbox.setBox(box);
            }
            else if(this.bbox !== null)
            {
                this.bbox.reset(worldTranslation);
            }

            for(var i = 0; i < this.children.length; i++)
            {
                this.bbox.merge(this.children[i].bbox);
            }            
        };
    })(),

    update: function()
    {
        this.updateWorldMatrix_r();
    },

    /*临时使用，为了解决entity新建和加载的数据格式不一致的问题,2019/6/28*/
    isBboxWorld: function()
    {
        let newType = ['helper', 'box', 'sphere'];
        if(newType.indexOf(this.type) === -1)
        {
            return false;
        }
        return true;
    },

    isPositionWorld: function()
    {
        return !this.isBboxWorld();
    },
    /*end*/

    getTrianglesCount: function ()
    {
        var triangles = 0;
        var meshFilter = this.getComponent('meshFilter');
        if (meshFilter)
        {
            var mesh = meshFilter.getMesh();
            if(mesh) {
                triangles += mesh.getTrianglesCount();
            }
        }

        for (var child of this.children)
        {
            triangles += child.getTrianglesCount();
        }
        return triangles;
    },

    traverse: function ( callback ) {
		callback( this );
		var children = this.children;
		for ( var i = 0, l = children.length; i < l; i ++ ) {
			children[ i ].traverse( callback );
		}
	},
}
