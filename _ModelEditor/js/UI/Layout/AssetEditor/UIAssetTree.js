(function ()
{
    let UIAssetTree = function(options) 
    {
        mgs.UIPanel.call(this, options);

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
            }
        );
        for (let i = 0;i < 3;i ++)
        {
            let subTree = new mgs.UITree({text:'subTree' + i, rootTree:tree});
            tree.appendTree(subTree);

            let subTree2 = new mgs.UITree({text:'subTree2' + i, rootTree:tree});
            subTree.appendTree(subTree2);

            // let subTree3 = new mgs.UITree2({text:'subTree3' + i, rootTree:tree});
            // subTree.appendTree(subTree3);
        }
        this.content.append(tree);
        this.content.getRoot().addEventListener('mouseup', function(evt)
        {
            tree._selectTrees([]);
        }, false);
    };
    mgs.classInherit(UIAssetTree, mgs.UIPanel);
}());