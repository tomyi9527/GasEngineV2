//Author: saralu
//Date: 2019/7/8
//sceneTree
//deal with the tree and menu
SceneEditor.SceneTreeManager = function(panel)
{
    SceneEditor.PanelManager.call(this, panel);
}

SceneEditor.SceneTreeManager.prototype = Object.create(SceneEditor.PanelManager.prototype);
SceneEditor.SceneTreeManager.prototype.constructor = SceneEditor.SceneTreeManager;

SceneEditor.SceneTreeManager.prototype.init = function()
{   
   this.initEventListener();
   this.initBinder();
   this.initCmdHandler();
}

SceneEditor.SceneTreeManager.prototype.initEventListener = function()
{
    //tree event
    let treeRoot = this.panel.getRoot();
    let sceneMenu = SceneEditor.Editor.Instance.sceneMenu;
    treeRoot.addEventListener('mousedown', function(evt)
    {
        if (evt.button === 2)
        {
            sceneMenu.open(evt.clientX, evt.clientY);
        }
        evt.stopPropagation();
    });

    let tree = this.panel.getTree();
    treeRoot.addEventListener('mouseup', function(evt)
    {
        tree._selectTrees([]);
        evt.stopPropagation();
    }, false);
    ////////////////////////////////

    let treePanel = this.panel;
    mgsEditor.on(mgsEditor.EVENTS.onSceneLoaded, function(dSceneRoot)
    {
        // self._attributeGridManager = new mgs.UIAttributeGridManager({panel: self});
        treePanel.bindData(dSceneRoot);
    });
}

SceneEditor.SceneTreeManager.prototype.initBinder = function()
{
    let treePanel = this.panel;
    this._sceneTreeBinder = new mgs.ActionBinder();
    this._sceneTreeBinder.on('onSelect', function(cmd)
    {
        let tree = treePanel.getTree();
        let entityID = cmd.n;
        if(!entityID) 
        {
            tree._selectTrees([]);    
            return;
        }
        tree._selectTreesByID(entityID);
    });


    this._sceneTreeBinder.on('onAction', function(cmd)
    {
        let tree = treePanel.getTree();
        if(cmd.t === 'set')
        {
            let entityID = cmd.n.child;
            let entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
            let parentID = cmd.n.parentNode;
            let parent = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(parentID);
            let nextIdx = cmd.n.index;
            let next = parent.getChildAt(nextIdx);

            let parentTrees = tree.getTreeByEntityID(parentID);
            let parentTree = parentTrees[0];
            treePanel.initSceneTree(entity, tree, parentTree, next);
        }
        else if(cmd.t === 'delete')
        {
            var obj = cmd.n;
            var pid = obj.parent;
            // var pid = parent.uniqueID;

            var cid = obj.child;
            // var cid = child.uniqueID;
            
            tree._removeTrees(pid, cid);
        }
        else if(cmd.t === 'insert')
        {
            var obj = cmd.n;
            if(obj.parent && obj.child)
            {
                var pid = obj.parent;
                // var pid = parent.uniqueID;
    
                var cid = obj.child;
                // var cid = child.uniqueID;
                tree._onDrop_Header(pid, cid);   
                
            }
            else if(obj.first && obj.next)
            {
                var fid = obj.first;
                // var fid = first.uniqueID;
                var nid = obj.next;
                // var nid = next.uniqueID;
                tree._onDrop_InsertArea(fid, nid);  

            }
        }
    });

    this._selectTreeHandler = SceneEditor.Editor.Instance.cmdManager.getHandler('s');
    this._selectTreeHandler.bind('e', this._sceneTreeBinder);

    this._delegateHandler = SceneEditor.Editor.Instance.cmdManager.getHandler('d');
    this._delegateHandler.bind('e', this._sceneTreeBinder);
}

SceneEditor.SceneTreeManager.prototype.initCmdHandler = function()
{
    //select
    mgsEditor.on(mgsEditor.EVENTS.sceneTree.entitySelected, function(trees)
    {
        let entity = trees[0].entity;
        let id = entity ? entity.uniqueID : null;
        var op = {
            h: 's',
            t: 'select',
            i: 'e',
            p: id
        }
        var cmd = new SceneEditor.Command(op);
        SceneEditor.Editor.Instance.cmdManager.processCmds([cmd]);
    });

    //delete
    mgsEditor.on(mgsEditor.EVENTS.sceneTree.entityDeleted, function(entity)
    {
        console.log(entity);
        // var idx = entity.parent.findChild(entity);
        // var obj = {entity: entity, index: idx};
        var id = entity ? entity.uniqueID : null;
        var op1 = {
            h: 'd',
            t: 'delete',
            i: 'e',
            p: id
        }
        var cmd1 = new SceneEditor.Command(op1);

        var op2 = {
            h: 's',
            t: 'select',
            i: 'e',
            p: null
        }
        var cmd2 = new SceneEditor.Command(op2);
        SceneEditor.Editor.Instance.cmdManager.processCmds([[cmd2, cmd1]]);
    });

    //create
    mgsEditor.on(mgsEditor.EVENTS.sceneTree.entityCreated, function(type)
    {
        let entityID = SceneEditor.Editor.Instance.createEntity(type);
        var op1 = {
            h: 'd',
            t: 'set',
            i: 'e',
            p: entityID
        }
        var cmd1 = new SceneEditor.Command(op1);

        var op2 = {
            h: 's',
            t: 'select',
            i: 'e',
            p: entityID
        }
        var cmd2 = new SceneEditor.Command(op2);
        SceneEditor.Editor.Instance.cmdManager.processCmds([[cmd1, cmd2]]);
    });

    //set
    mgsEditor.on(mgsEditor.EVENTS.sceneTree.onDropHeader, function(parent, child)
    {
        let pID = parent ? parent.uniqueID : null;
        let cID = child ? child.uniqueID : null;
        let p = {parent: pID, child: cID};
        var op = {
            h: 'd',
            t: 'insert',
            i: 'e',
            p: p
        }
        var cmd = new SceneEditor.Command(op);
        SceneEditor.Editor.Instance.cmdManager.processCmds([cmd]);
    });

    mgsEditor.on(mgsEditor.EVENTS.sceneTree.onDropInsertArea, function(first, next)
    {
        let fID = first ? first.uniqueID : null;
        let nID = next ? next.uniqueID : null;
        let p = {first: fID, next: nID};
        var op = {
            h: 'd',
            t: 'insert',
            i: 'e',
            p: p
        }
        var cmd = new SceneEditor.Command(op);
        SceneEditor.Editor.Instance.cmdManager.processCmds([cmd]);
    });

    let treePanel = this.panel;
    mgsEditor.on(mgsEditor.EVENTS.sceneTree.entityCopy, function()
    {
        let tree = treePanel.getTree();
        console.log('copy');
        let selectTrees = tree.getSelectTrees();
        let entities = [];
        for(let i = 0; i < selectTrees.length; i++)
        {
            let selectTree = selectTrees[i];
            let entity = selectTree.entity;
            entities.push(entity);
        }
        //for test, only support copy single entity!!!
        if(entities.length > 0)
        {
            var newEntity = entities[0].copy();
            console.log(newEntity);
        }
    });

    mgsEditor.on(mgsEditor.EVENTS.sceneTree.entityPaste, function()
    {
        console.log('paste');
    });

}