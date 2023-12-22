(function ()
{
    let DelegateEntity = function(params) 
    {
        mgs.DelegateObjectBase.call(this, params);

        // -------------------------------------- undisplayed property --------------------------------- //
        // this.addProperty
        // (
        //     {
        //         path: 'parent',
        //         getter: function()
        //         {
        //             let entity = this.getSourceObject();
        //             return entity.parent;
        //         },
        //     }
        // );

        // -------------------------------------- displayed property --------------------------------- //
        this.addProperty
        (
            {
                path: 'id',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    return entity.uniqueID;
                },
                setter: function(value)
                {

                },
                editParam:
                {
                    type: 'Label',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'name',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    return entity.name;
                },
                setter: function(value)
                {
                    let entity = this.getSourceObject();
                    entity.name = value;
                },
                editParam:
                {
                    type: 'Input',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'position',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    let translation = entity.getLocalTranslation();
                    let value = [translation.x, translation.y, translation.z];
                    return value;
                },
                setter: function(value)
                {
                    let entity = this.getSourceObject();
                    entity.setLocalTranslation(new GASEngine.Vector3(value[0], value[1], value[2]));
                },
                editParam:
                {
                    type: 'Vector3',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'rotation',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    let rotation = entity.getLocalRotation(); 
                    let value = [rotation.x, rotation.y, rotation.z];
                    return value;
                },
                setter: function(value)
                {
                    let entity = this.getSourceObject();
                    entity.setLocalRotation(new GASEngine.Euler(value[0], value[1], value[2], 'XYZ'));
                },
                editParam:
                {
                    type: 'DegreeVector',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'scale',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    let scale = entity.getLocalScale();
                    let value = [scale.x, scale.y, scale.z];
                    return value;
                },
                setter: function(value)
                {
                    let entity = this.getSourceObject();
                    entity.setLocalScale(new GASEngine.Vector3(value[0], value[1], value[2]));
                },
                editParam:
                {
                    type: 'Vector3',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'components',
                editParam:
                {
                    type: 'Panel',
                    nestDelegateClass: mgs.DelegateComponents,
                }
            }
        );
    };
    mgs.classInherit(DelegateEntity, mgs.DelegateObjectBase);

    DelegateEntity.prototype.getChildDelegates = function()
    {
        let childDelegates = [];

        let entity = this.getSourceObject();  
        for (let i = 0;i < entity.children.length;i ++)
        {
            let child = this.getManager().get(entity.children[i].uniqueID);
            if (child)
            {
                childDelegates.push(child);
            }
        }

        return childDelegates;
    };

    DelegateEntity.prototype.getParentDelegate = function()
    { 
        let entity = this.getSourceObject();
        if (entity.parent)
        {
            return this.getManager().get(entity.parent.uniqueID);
        }

        return null;
    };
}());