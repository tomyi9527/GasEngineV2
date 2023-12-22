
/**
 * @module mgs
 * @description root namespace for minigamestudio engine
 * @version 1.0.1 
 */

let mgs =
{
    systemObject : Object
};

(function(global)
{
    mgs.assign = function(name, value)
    {
        if (mgs[name])
        {
            mgs.Log.error('mgs root namespace props name:' + '"' + name + '"' + ' is eixsted, try mgs.assign other name!');
        }
    
        mgs[name] = value;
    }

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
})(this);