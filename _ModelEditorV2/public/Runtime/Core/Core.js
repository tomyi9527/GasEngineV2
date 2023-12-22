
/**
 * @module mgs
 * @description root namespace for minigamestudio runtime
 * @version 1.0.1 
 */

(function(global)
{
    let mgs =
    {
        systemObject : Object
    };

    mgs.assign = function(name, value, namespace)
    {
        // if (mgs[name])
        // {
        //     mgs.Log.error('mgs root namespace props name:' + '"' + name + '"' + ' is eixsted, try mgs.assign other name!');
        // }
    
        // mgs[name] = value;

        namespace = namespace ? namespace : mgs;
        if (namespace[name])
        {
            var errorMsg = 'namespace props name:';
            errorMsg += '"' + name + '"';
            errorMsg += ' is eixsted, try mgs.assign other name!';
            console.error(errorMsg);
        }
    
        namespace[name] = value;
    };

    try
    {
        mgs.now = (!window.performance || !window.performance.now || !window.performance.timing) ? Date.now : function () 
        {
            return window.performance.now();
        };
    }
    catch(e)
    {
        console.log('---------------- server side engine core! ----------------');
    }


    if (typeof exports === 'object' && typeof module === 'object')
    {
        module.exports = mgs;
    }
    else
    {
        if (global)
        {
            global.mgs = mgs;
        }
    }
    
})(this || window);