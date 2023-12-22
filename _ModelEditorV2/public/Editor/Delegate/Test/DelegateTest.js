(function ()
{
    // ------------------------------ test source object ------------------------------ //
    let TestSourceObjectNest = function() 
    {
        mgs.Object.call(this);

        this.a = 11;
        this.b = 'ef';
        this.c = new GASEngine.Vector3(2, 1, 0);
    };
    mgs.classInherit(TestSourceObjectNest, mgs.Object);

    let TestSourceObject = function() 
    {
        mgs.Object.call(this);

        this.id = GASEngine.generateUUID();
        this.name = 'testObjectName';
        this.a = 10;
        this.b = 'abc';
        this.c = new GASEngine.Vector3(0, 1, 2);
        this.testNest = null; // new TestSourceObjectNest();
    };
    mgs.classInherit(TestSourceObject, mgs.Object);

    // ------------------------------ test delegate object ------------------------------ //
    let DelegateTestNest = function(params) 
    {
        mgs.DelegateObjectBase.call(this, params);

        this.addProperty
        (
            {
                path: 'a',
                editParam:
                {
                    type: 'Input',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'b',
                editParam:
                {
                    type: 'Input',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'c',
                editParam:
                {
                    type: 'Vector3',
                }
            }
        );
    };
    mgs.classInherit(DelegateTestNest, mgs.DelegateObjectBase);

    let DelegateTest = function(params) 
    {
        mgs.DelegateObjectBase.call(this, params);

        this.addProperty
        (
            {
                path: 'name',
                editParam:
                {
                    type: 'Label',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'a',
                editParam:
                {
                    type: 'Input',
                    gridOption: 
                    {
                        textOptions:
                        {
                            numberOnly: true,
                        },
                    }, 
                }
            }
        );

        this.addProperty
        (
            {
                path: 'b',
                editParam:
                {
                    type: 'Input',
                }
            }
        );

        this.addProperty
        (
            {
                path: 'testNest',
                editParam:
                {
                    type: 'Panel',
                    nestDelegateClass: mgs.DelegateTestNest,
                    hideCondition: {value: null},
                }
            }
        );
        
        this.addProperty
        (
            {
                path: 'c',
                editParam:
                {
                    type: 'Vector3',
                }
            }
        );


        
    };
    mgs.classInherit(DelegateTest, mgs.DelegateObjectBase);

    DelegateTest.prototype.onDestroy = function()
    {
        
    };
}());