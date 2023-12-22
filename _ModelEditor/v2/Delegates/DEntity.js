//Author: saralu
//Date: 2019/5/31
//Delegate of entity

SceneEditor.DEntity = function(engineEntity)
{
    SceneEditor.DObjectBase.call(this);
    this.engineEntity = engineEntity;
    this.translation = new GASEngine.Vector3();
    this.rotation = new GASEngine.Euler();
    this.quaternion = new GASEngine.Quaternion();
    this.scale = new GASEngine.Vector3(1, 1, 1);


    //used for bind helper and object
    // this.helper = null;
    this.gizmo = null;
    this.picker = null;

    this.realEntity = this.engineEntity;
    
    this.setProperty();
}

SceneEditor.DEntity.prototype = Object.create(SceneEditor.DObjectBase.prototype);
SceneEditor.DEntity.prototype.constructor = SceneEditor.DEntity;

SceneEditor.DEntity.prototype.getPath = function()
{
    if(!this.engineEntity) return '';
    var path = [];
    var entity = this.engineEntity;
    while(entity.parent !== null)
    {
        path.push(entity.uniqueID);
        entity = entity.parent;
    }
    return path.reverse().join('/');
}

SceneEditor.DEntity.prototype.getRealEntityForMove = function(entity)
{
    if(!entity) return entity;
    var meshFilterComponent = entity.getComponent('meshFilter');
    if(!meshFilterComponent) return entity;
    var mesh = meshFilterComponent.getMesh();
    if(mesh.isSkinned())
    {
        while(entity !== null)
        {
            var animator = entity.getComponent('animator');
            if(animator !== null)
            {
                break;
            }
            else
            {
                entity = entity.parent;
            }
        }
        return entity;
    }    
    else 
    {
        return entity;
    }
};

SceneEditor.DEntity.prototype.setProperty = function()
{
    var propName = {
        id: 'Name',
        displayName: '名称',
        type: 'string',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineEntity)
            {
                return this.engineEntity.name;
            }
            else 
            {
                return '';
            }
        }.bind(this),
        set: function(name)
        {
            if(this.engineEntity)
            {
                this.engineEntity.name = name;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    var propUUID =
    {
        id: 'uniqueID',
        displayName: 'UUID',
        type: 'string',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineEntity)
            {
                return this.engineEntity.uniqueID;
            }
            else 
            {
                return 0;
            }
        }.bind(this),
        set: function(uniqueID)
        {
            if(this.engineEntity)
            {
                this.engineEntity.uniqueID = uniqueID;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    var propTranslation = 
    {
        id: 'translation',
        displayName: '位移',
        type: 'vector3',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            return this.getTranslation();
        }.bind(this),
        set: function(translation)
        {
            this.setTranslation(translation);
        }.bind(this)
    };

    var propRotation = 
    {
        id: 'rotation',
        displayName: '旋转',
        type: 'vector3',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            return this.getRotation();
        }.bind(this),
        set: function(rotation)
        {
            this.setRotation(rotation);
        }.bind(this)
    };

    var propScale = 
    {
        id: 'scale',
        displayName: '缩放',
        type: 'vector3',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            return this.getScale();
        }.bind(this),
        set: function(scale)
        {
            this.setScale(scale);
        }.bind(this)
    };

    // var propChildren = 
    // {
    //     id: 'children',
    //     displayName: 'Children',
    //     type: 'array',
    //     range: undefined,
    //     editorType: 'Input',
    //     viewType: 'Span',
    //     showMode: 'view',
    //     getElementCount: function()
    //     {
    //         if(this.engineEntity)
    //         {
    //             return this.engineEntity.children.length;
    //         }
    //         else 
    //         {
    //             return 0;
    //         }
    //     }.bind(this),
    //     get: function(index)
    //     {
    //         if(this.engineEntity)
    //         {
    //             if(index < 0 || index >= this.engineEntity.children.length) return false;
    //             var DEntity = new SceneEditor.DEntity(this.engineEntity.children[index]);
    //             return DEntity;
    //         }
    //         else
    //         {
    //             return false;
    //         }
    //     }.bind(this),
    //     set: function(index, entity)
    //     {
    //         if(this.engineEntity)
    //         {
    //             if(index < 0 || index >= this.engineEntity.children.length) return false;
    //             var historyType = 'children' + index;
    //             var historyItem = this.engineEntity.children[index];
    //             SceneEditor.HitoryList.Instance.addHistoryList({type: historyType, oldValue: historyItem, object: this.engineEntity});
    //             this.engineEntity.children[index] = entity;
    //         }
    //         else
    //         {
    //             return false;
    //         }
    //     }.bind(this)
    // };

    var propComponent = 
    {
        id: 'components',
        displayName: 'Component',
        type: 'map',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        getComponentTypes: function()
        {
            if(this.engineEntity)
            {
                var types = Array.from(this.engineEntity.components.keys());
                return types;
            }
            else
            {
                return false;
            }
        }.bind(this),
        getComponent: function(typeName)
        {
            if(this.engineEntity)
            {
                var component = this.engineEntity.getComponent(typeName);
                var dComponent = null;
                switch(typeName)
                {
                    case 'meshRenderer':
                        dComponent = new SceneEditor.DMeshRendererComponent(component);
                        break;
                    case 'meshFilter':
                        dComponent = new SceneEditor.DMeshFilterComponent(component);
                        break;
                    case 'camera':
                        dComponent = new SceneEditor.DCameraComponent(component);
                        break;
                }
                return dComponent;
            }
            else
            {
                return false;
            }
        }.bind(this),
        set: function(typeName, component)
        {
            if(this.engineEntity)
            {
                this.engineEntity.addComponent(component);
            }
            else
            {
                return false;
            }
        }.bind(this)

    }

    this.properties.push(propName);
    this.properties.push(propUUID);
    this.properties.push(propTranslation);
    this.properties.push(propRotation);
    this.properties.push(propScale);
    // this.properties.push(propChildren);
    this.properties.push(propComponent);
}




SceneEditor.DEntity.prototype.getWorldTranslation = function()
{
    var translation = new GASEngine.Vector3();
    return function()
    {
        if(this.realEntity)
        {
            // this.realEntity.matrixWorld.decompose( translation, this.quaternion, this.scale);
            translation.copy(this.realEntity.getWorldTranslation());
            return translation;
        }
        else 
        {
            translation.set(0, 0, 0);
            return translation;
        }
    };
}();

SceneEditor.DEntity.prototype.setWorldTranslation = function()
{
    return function(vec)
    {
        if(this.realEntity)
        {
            this.realEntity.setWorldTranslation(vec);
            this.realEntity.update();
            // console.log(this.realEntity.translation);
        }
        else
        {
            return false;
        }
    };
}();

SceneEditor.DEntity.prototype.worldPositionToLocal = function()
{
    return function(worldTranslation, localTranslation)
    {
        if(this.realEntity)
        {
            this.realEntity.worldPositionToLocal(worldTranslation, localTranslation);
        }
    };
}();

SceneEditor.DEntity.prototype.worldRotationToLocal = function()
{
    return function(worldRotation, localRotation)
    {
        if(this.realEntity)
        {
            this.realEntity.worldRotationToLocal(worldRotation, localRotation);
        }
    };
}();

SceneEditor.DEntity.prototype.getTranslation = function()
{
    var translation = new GASEngine.Vector3();
    return function()
    {
        if(this.realEntity)
        {
            // this.realEntity.matrixWorld.decompose( translation, this.quaternion, this.scale);
            translation.copy(this.realEntity.getLocalTranslation());
            return translation;
        }
        else 
        {
            translation.set(0, 0, 0);
            return translation;
        }
    };
}();

SceneEditor.DEntity.prototype.setTranslation = function()
{
    return function(vec)
    {
        if(this.realEntity)
        {
            // this.realEntity.translation.copy(vec);
            this.realEntity.setLocalTranslation(vec);
            this.realEntity.update();
            // console.log(this.realEntity.translation);
        }
        else
        {
            return false;
        }
    };
}();



SceneEditor.DEntity.prototype.getWorldRotation = function()
{
    var rotation = new GASEngine.Euler();
    return function()
    {
        if(this.realEntity)
        {
            // this.realEntity.matrixWorld.decompose(  this.translation, this.quaternion, this.scale);
            // rotation.setFromQuaternion(this.quaternion, 'XYZ');
            rotation.copy(this.realEntity.getWorldRotation());
            return rotation;
        }
        else 
        {
            rotation.set(0,0,0,'XYZ');
            return rotation;
        }
    }
}();

SceneEditor.DEntity.prototype.setWorldRotation = function()
{
    var rotation = new GASEngine.Euler();
    return function(euler)
    {
        if(this.realEntity)
        { 
            rotation.x = euler.x;
            rotation.y = euler.y;
            rotation.z = euler.z;
            rotation.order = euler.order;
            this.realEntity.setWorldRotation(rotation);
            this.realEntity.update();
        }
        else 
        {
            return false;
        }
    }
}();

SceneEditor.DEntity.prototype.getRotation = function()
{
    var rotation = new GASEngine.Euler();
    return function()
    {
        if(this.realEntity)
        {
            // this.realEntity.matrixWorld.decompose(  this.translation, this.quaternion, this.scale);
            // rotation.setFromQuaternion(this.quaternion, 'XYZ');
            // rotation.copy(this.realEntity.rotation);
            rotation.copy(this.realEntity.getLocalRotation());
            return rotation;
        }
        else 
        {
            rotation.set(0,0,0,'XYZ');
            return rotation;
        }
    }
}();

SceneEditor.DEntity.prototype.setRotation = function()
{
    var rotation = new GASEngine.Euler();
    return function(euler)
    {
        if(this.realEntity)
        {
            rotation.x = euler.x;
            rotation.y = euler.y;
            rotation.z = euler.z;
            // this.realEntity.rotation.copy(rotation);
            this.realEntity.setLocalRotation(rotation);
            this.realEntity.update();
        }
        else
        {
            return false;
        }
        
    };
}();

SceneEditor.DEntity.prototype.getWorldScale = function()
{
    var scale = new GASEngine.Vector3();
    return function()
    {
        if(this.realEntity)
        {
            // this.realEntity.matrixWorld.decompose( this.translation, this.quaternion, scale);
            scale.copy(this.realEntity.getWorldScale());
            return scale;
        }
        else 
        {
            scale.set(0, 0, 0);
            return scale;
        }
    };
}();

SceneEditor.DEntity.prototype.setWorldScale = function()
{
    return function(vec)
    {
        if(this.realEntity)
        {
            // this.realEntity.matrixWorld.decompose( this.translation, this.quaternion, scale);
            this.realEntity.setWorldScale(vec);
            this.realEntity.update();
        }
        else 
        {
            return false;
        }
    };
}();

SceneEditor.DEntity.prototype.getScale = function()
{
    var scale = new GASEngine.Vector3();
    return function()
    {
        if(this.realEntity)
        {
            // this.realEntity.matrixWorld.decompose( this.translation, this.quaternion, scale);
            scale.copy(this.realEntity.getLocalScale());
            return scale;
        }
        else 
        {
            scale.set(0, 0, 0);
            return scale;
        }
    };
}();

SceneEditor.DEntity.prototype.setScale = function()
{
    var scale = new GASEngine.Vector3();
    return function(vec)
    {
        // this.validate(vec);
        // GASEngine.Utilities.validateVector(vec);

        if(this.realEntity)
        {
            this.realEntity.setLocalScale(vec);
            this.realEntity.update();
        }
        else
        {
            return false;
        }
    };
}();
