(function()
{
    let EngineUtil = 
    {
        getworldTranslation : function(source) 
        {
            let target = {};
            for (let key in source)
            {
                target[key] = source[key];
            }
            
            return target;
        },
    };
    mgs.assign('EngineUtil', EngineUtil);
}());

