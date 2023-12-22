(function()
{
    /*************************************** editor string ***************************************/
    let language = 'cn';

    let STRINGS =
    {
        getString : function(key)
        {
            return this[key][language];
        },
    };

    STRINGS.buttonCopy = {cn:'复制', en:'Copy'};

    mgs.assign('STRINGS', STRINGS);

    /*************************************** editor icons ***************************************/
    let ICONS =
    {
    };

    // toolbar icons
    ICONS.menu = '&#57960';
    ICONS.moveBtn = '&#57617';
    ICONS.rotateBtn = '&#57619';
    ICONS.scaleBtn = '&#57618';
    ICONS.focusBtn = '&#57623';
    ICONS.closeBtn = '&#57650';
    ICONS.saveBtn = '&#57907';
    ICONS.playBtn = '&#57649';
    // ICONS.spaceBtn = '&#ea52';

    // tree
    ICONS.treeCollapse = '&#57728';
    ICONS.treeExpand = '&#57721';

    // viewtree icons
    ICONS.viewtreeDefault = '&#57734';

    // fold button
    ICONS.foldButtonLeft = '&#57688',
    ICONS.foldButtonRight = '&#57696',
    ICONS.foldButtonTop = '&#57687',
    ICONS.foldButtonBottom = '&#57689',

    // checkbox
    ICONS.checkbox = '&#57651',

    // menu list
    ICONS.menuListItemExtend = '&#57696',

    // asset menu list
    ICONS.assetMenuListAdd = '&#58129',
    ICONS.assetMenuListCopy = '&#57638',
    ICONS.assetMenuListDelete = '&#57636',
    ICONS.assetMenuListReplace = '&#57619',

    // asset
    ICONS.assetIconDefault = '&#58009',

    // property
    ICONS.propertyContainer_FolderButtonRight = '&#57728',
    ICONS.propertyContainer_FolderButtonDown = '&#57721',

    mgs.assign('ICONS', ICONS);

    /*************************************** editor lang ***************************************/
    let constLang = 'cn';
    let LANG =
    {
        getString : function(key)
        {
            return this[key][constLang];
        },
    };

    // asset menu list
    LANG.create = 
    {
        en: 'Create',
        cn: '创建资源',
    };

    LANG.copy = 
    {
        en: 'Copy',
        cn: '复制',
    };

    LANG.paste = 
    {
        en: 'Paste',
        cn: '粘贴',
    };

    LANG.delete = 
    {
        en: 'Delete',
        cn: '删除',
    };

    LANG.replace = 
    {
        en: 'Replace',
        cn: '替换',
    };

    mgs.assign('LANG', LANG);
    
    /*************************************** events ***************************************/
    let EVENTS =
    {
    };

    // toolbar icons
    EVENTS.onAssetLoaded = 'onAssetLoaded';
 
    // hot keys
    EVENTS.hotkey = {};
    EVENTS.hotkey.register = 'EVENTS.hotkey.register';
    EVENTS.hotkey.shift = 'EVENTS.hotkey.shift';
    EVENTS.hotkey.ctrl = 'EVENTS.hotkey.ctrl';
    EVENTS.hotkey.alt = 'EVENTS.hotkey.alt';
    EVENTS.hotkey.updateModifierKeys = 'EVENTS.hotkey.updateModifierKeys';
    EVENTS.hotkey.ctrlString = 'EVENTS.hotkey.ctrlString';

    mgs.assign('EVENTS', EVENTS);
}());

