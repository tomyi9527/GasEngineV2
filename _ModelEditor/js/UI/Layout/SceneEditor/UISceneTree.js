(function ()
{
    let UISceneTree = function(options) 
    {
        mgs.UIPanel.call(this, options);

        this.tree = new mgs.UITree
        (
            {
                styleOptions:{}, 
                childStyleOptions:
                {
                    header:{display:'none',}, 
                    insertArea:{display:'none',},
                    content:{'padding-left':'0px',},
                },
                canFocus: true,
            }
        );
     
        this.content.append(this.tree);
    };
    mgs.classInherit(UISceneTree, mgs.UIPanel);

    UISceneTree.prototype.bindData = function(dSceneRoot)
    {
        this.contentClear();
        if(!dSceneRoot) return;
        let tree = new mgs.UITree
        (
            {
                styleOptions:{}, 
                childStyleOptions:
                {
                    header:{display:'none',}, 
                    insertArea:{display:'none',},
                    content:{'padding-left':'0px',},
                },
                canFocus: true,
                text: dSceneRoot.name,
                entity: dSceneRoot
            }
        );

        for(var i = 0; i < dSceneRoot.children.length; i++)
        {
            this.initSceneTree(dSceneRoot.children[i], tree, tree);
        }
        this.tree = tree;
        this.content.append(this.tree);
    };

    UISceneTree.prototype.initSceneTree = function(entity, root, parent, next)
    {
        let subTree = new mgs.UITree({text: entity.name, rootTree:root, entity: entity});
        for(var i = 0; i< entity.children.length; i++)
        {
            this.initSceneTree(entity.children[i], root, subTree);
        }
        let nextTree = null;
        if(next)
        {
            let nextTrees = parent.getTreeByEntityID(next.uniqueID);
            nextTree = nextTrees.length > 0 ? nextTrees[0] : null;
        }
        parent.appendTree(subTree, nextTree); 
        return subTree;   
    };
    
    UISceneTree.prototype.getTree = function()
    {
        return this.tree;
    }

}());