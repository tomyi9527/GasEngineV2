//Author: saralu
//Date: 2019-5-27
//Notes: root index

function parseURL()
{
    var query = (function()
    {
        var qs = window.location.search.substring(1).split('&');
        var result = {};
        for(var i = 0; i < qs.length; ++i)
        {
            var part = qs[i];
            var pair = part.split('=');
            var key = decodeURIComponent(pair[0]);
            var value = decodeURIComponent(pair[1]);

            if(key in result)
            {
                if(Array.isArray(result[key]))
                {
                    result[key].push(value);
                }
                else
                {
                    result[key] = [result[key], value];
                }
            }
            else
            {
                result[key] = value;
            }
        }

        return result;
    })();

    var modelPath = query.url;
    if (!modelPath)
        return;

    var i = modelPath.lastIndexOf('/');
    if (i < 0)
    {
        return;
    }

    var modelRoot = modelPath.substring(0, i + 1);
    var modelName = modelPath.substr(i + 1);
    var assetID = query.asset_id;
    var k = window.location.href.indexOf('new_engine_v');
    var engineRoot = window.location.href.substr(0, k);

    return {
        'engineRoot'    : engineRoot,
        'assetID'       : assetID,
        'modelName'     : modelName,
        'modelRoot'     : modelRoot,
        'parameters'    : query
    };
};

function onTick()
{
    requestAnimationFrame(onTick);

    SceneEditor.Editor.Instance.tick();
};

function onResize(event)
{
    SceneEditor.Editor.Instance.onWindowResize();
};

function onClick(event)
{
    SceneEditor.Editor.Instance.onClicked();
};

function onMouseMove(event)
{
    SceneEditor.Editor.Instance.onMouseMove(event.clientX, event.clientY);
};

function onKeyDown(event)
{
    SceneEditor.Editor.Instance.onKeyDown(event);
}

window.onload = function()
{
    var parameters = parseURL() || {};

    new SceneEditor.Editor();

    SceneEditor.Editor.Instance.init();

    // SceneEditor.Editor.Instance.loadSimpleScene();
    
    SceneEditor.Editor.Instance.loadScene(parameters.modelRoot, parameters.modelName);

    //SceneEditor.Editor.Instance.loadGASScene(parameters.modelRoot, parameters.modelName);
    
    onTick();

    // window.addEventListener('resize', onResize, false);
    window.addEventListener('click', onClick, false);
    window.addEventListener('mousemove', onMouseMove, false);
    window.addEventListener('contextmenu', event => event.preventDefault());
    window.addEventListener('keydown', onKeyDown, false);
}

