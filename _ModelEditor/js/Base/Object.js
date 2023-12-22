(function () 
{
    mgs.getClassByName = function(name)
    {
        let theClass = mgs[name];
        if (!theClass)
        {
            return;
        }

        return theClass.prototype ? theClass.prototype._class_ : null;
    };

    inherits = function (Self, Super) 
    {
        console.log(Self.name);
        var Temp = function () {};
        var Func = function (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) 
        {
            Super.call(this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
            Self.call(this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
            // this.constructor = Self;
        };
        Func._super = Super.prototype;
        Temp.prototype = Super.prototype;
        Func.prototype = new Temp();

        return Func;
    };


    mgs.assign('classInherit', function(sub, supper)
    {
        let subName = sub.name;
        mgs.assign(subName, sub);
        
        ///////////////////// RTTI /////////////////////////
        if (supper)
        {
            sub.prototype = Object.create(supper.prototype);
            sub.prototype._superClass_ = supper;
        };

        sub.prototype._class_ = sub;
        sub.prototype.getClass = function()
        {
            return sub.prototype._class_;
        };

        sub.prototype.getClassName = function()
        {
            return subName; // this._class_.name;
        };

        sub.prototype.getSuperClass = function()
        {
            return sub.prototype._superClass_;
        };

        sub.prototype.isInstanceOf = function(inSuperClass)
        {
            let superClass = this.getClass();
            while(superClass)
            {
                if (superClass === inSuperClass)
                {
                    return true;
                }

                superClass = superClass.prototype._superClass_;
            }
            return false;
        };

        sub.isSubClassOf = function(inSuperClass)
        {
            return this.prototype._superClass_ === inSuperClass;
        };

        sub.prototype.superCall = function(functionName)
        {
            let superClass = this.getSuperClass();
            if (superClass)
            {
                let func = superClass.prototype[functionName];
                if (func)
                {
                    var args = Array.from(arguments);
                    return func.apply(this, args.slice(1));
                }
            }
        };

        sub.superCall = function(functionName, obj)
        {
            let superClass = sub.prototype._superClass_;
            if (superClass)
            {
                let func = superClass.prototype[functionName];
                if (func)
                {
                    var args = Array.from(arguments);
                    return func.apply(obj, args.slice(2));
                }
            }
        };

        if (supper)
        {
            if (!supper.__subs)
            {
                supper.__subs = {};
            }
            
            supper.__subs[subName] = sub;
        }

        return sub;
    });
}());

///////////////////// base object /////////////////////////
(function ()
{
    let Object = function()
    {
        this.uuid = GASEngine.generateUUID();
    };
    mgs.classInherit(Object, null);
}());

