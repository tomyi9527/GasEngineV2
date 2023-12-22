(function ()
{
    let DelegateComponents = function(params) 
    {
        mgs.DelegateObjectBase.call(this, params);

        this.addProperty
        (
            {
                path: 'animator',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    return entity.getComponent('animator');
                },
                editParam:
                {
                    type: 'Panel',
                    nestDelegateClass: mgs.DelegateAnimator,
                    hideCondition: {values: [null, undefined]},
                }
            }
        );

        this.addProperty
        (
            {
                path: 'meshFilter',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    return entity.getComponent('meshFilter');
                },
                editParam:
                {
                    type: 'Panel',
                    nestDelegateClass: mgs.DelegateMeshFilter,
                    hideCondition: {values: [null, undefined]},
                }
            }
        );

        this.addProperty
        (
            {
                path: 'meshRenderer',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    return entity.getComponent('meshRenderer');
                },
                editParam:
                {
                    type: 'Panel',
                    nestDelegateClass: mgs.DelegateMeshRenderer,
                    hideCondition: {values: [null, undefined]},
                }
            }
        );
    };
    mgs.classInherit(DelegateComponents, mgs.DelegateObjectBase);

    let DelegateComponent = function(params)
    {
        mgs.DelegateObjectBase.call(this, params);
    };
    mgs.classInherit(DelegateComponent, mgs.DelegateObjectBase);

    let DelegateAnimator = function(params)
    {
        mgs.DelegateComponent.call(this, params);

        this.addProperty
        (
            {
                path: 'clipProgress',
                getter: function()
                {
                    let entity = this.getSourceObject();
                    let animator = entity.getComponent('animator');
                    let activeClip = animator.getActiveAnimationClip();
                    if (activeClip)
                    {
                        return activeClip.progress;
                    }
                    
                    return 0;
                },
                editParam:
                {
                    type: 'Slider',
                    needUpdate: true,
                }
            }
        );
    };
    mgs.classInherit(DelegateAnimator, mgs.DelegateComponent);

    let DelegateMeshFilter = function(params)
    {
        mgs.DelegateComponent.call(this, params);
    };
    mgs.classInherit(DelegateMeshFilter, mgs.DelegateComponent);

    let DelegateMeshRenderer = function(params)
    {
        mgs.DelegateComponent.call(this, params);

        // this.addProperty
        // (
        //     {
        //         path: 'testInt',
        //         getter: function()
        //         {
        //             let entity = this.getSourceObject();
        //             let meshRender = entity.getComponent('meshRenderer');
        //             return meshRender.testInt;
        //         },
        //         setter: function(value)
        //         {
        //             let entity = this.getSourceObject();
        //             let meshRender = entity.getComponent('meshRenderer');
        //             meshRender.testInt = value;
        //         },
        //         editParam:
        //         {
        //             type: 'Slider',
        //             gridOption: 
        //             {
        //                 textOptions:
        //                 {
        //                     numberOnly: true,
        //                 },
        //             }, 
        //             // needUpdate: true,
        //         }
        //     }
        // );
    };
    mgs.classInherit(DelegateMeshRenderer, mgs.DelegateComponent);
}());