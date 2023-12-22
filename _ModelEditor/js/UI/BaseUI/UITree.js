(function ()
{
    // UITree architecture
    // UITree->
    //     InsertArea(position:absolute)    // deal drag evts
    //     Header->                         // deal drag evts
    //         Foldbutton
    //         Icon
    //         Title
    //     Content->
    //         UITree1
    //         UITree2
    //         ...

    let UITree = function(options) 
    {
        options = options ? options : {};
        mgs.UIBase.call(this, options);  

        this.rootTree = options.rootTree;
        this.entity = options.entity || null;

        // create elements
        this.insertArea = new mgs.UIBase({id:'insertArea', styleClass:'UITree-insertArea'});
        this.append(this.insertArea); 

        this.header = new mgs.UIBase({id:'header', styleClass:'UITree-header'});
        this.append(this.header);
        {
            this.foldButton = new mgs.UIBase({id:'foldButton', styleClass:'UITree-foldButton', innerHTML:mgs.ICONS.foldButtonBottom});
            this.header.append(this.foldButton);
    
            this.icon = new mgs.UIBase({id:'icon', styleClass:'UITree-icon', innerHTML:options.icon ? options.icon: mgs.ICONS.viewtreeDefault});
            this.header.append(this.icon);
    
            this.title = new mgs.UIBase({id:'title', styleClass:'UITree-title', innerHTML:options.text ? options.text: ''});
            this.header.append(this.title);
        }

        this.content = new mgs.UIBase({id:'content', styleClass:'UITree-content'});
        this.append(this.content);

        if (!this.rootTree)
        {
            // self is root
            this.dragHandler = new mgs.UIBase({id:'dragHandler', styleClass:'UITree-dragHandler', styleOptions:{display:'none'}});
            document.body.appendChild(this.dragHandler.getRoot());

            this.selectTrees = [];

            this.on('onFirstKeyDown', this.onFirstKeyDown.bind(this));
            this.on('onKeyUp', this.onKeyUp.bind(this));
        }

        this.checkFoldButtonVisible();
        this.processChildStyleOptions();

        // deal events
        let foldButtonRoot = this.foldButton.getRoot();
        let insertAreaRoot = this.insertArea.getRoot();
        let headerRoot = this.header.getRoot();

        foldButtonRoot.addEventListener('click', this._onFoldButtonClick.bind(this), this);

        insertAreaRoot.addEventListener('dragover', this._onDragOver_InsertArea.bind(this), this);
        insertAreaRoot.addEventListener('dragleave', this._onDragLeave_InsertArea.bind(this), this);
        // insertAreaRoot.addEventListener('drop', this._onDrop_InsertArea.bind(this), this);//emit
        insertAreaRoot.addEventListener('drop', this.onDrop_InsertArea.bind(this), this);//emit

        headerRoot.draggable = true;
        headerRoot.addEventListener('dragstart', this._onDragStart_Header.bind(this), this);
        headerRoot.addEventListener('dragover', this._onDragOver_Header.bind(this), this);
        headerRoot.addEventListener('dragleave', this._onDragLeave_Header.bind(this), this);
        // headerRoot.addEventListener('drop', this._onDrop_Header.bind(this), this);//emit
        headerRoot.addEventListener('drop', this.onDrop_Header.bind(this), this);//emit
        headerRoot.addEventListener('dragend', this._onDragEnd_Header.bind(this), this);
        headerRoot.addEventListener('mouseup', this._onMouseUp_Header.bind(this), this);
        // headerRoot.addEventListener('contextmenu', this._onContextMenu.bind(this), this);


        this.on('onRemoveTree', this.onRemoveTree.bind(this), this);
        this.on('onSelectTree', this.onSelectTree.bind(this), this);
    };
    mgs.classInherit(UITree, mgs.UIBase);

    let treeCompare = function(tree)
    {
        return tree;
    };

    let treeSort = function(a, b)
    {
        let treeLinkA = a._getTreeLink();
        let treeLinkB = b._getTreeLink();

        for (let i = 0;i < treeLinkA.length && i < treeLinkB.length;i ++)
        {
            let treeA = treeLinkA[i];
            let treeB = treeLinkB[i];

            let result = treeA.getIndex() - treeB.getIndex();
            if (result !== 0)
            {
                return result;
            }
        }

        return treeLinkA.length - treeLinkB.length;
    };

    UITree.prototype.getRootTree = function()
    {
        return this.rootTree || this;
    };

    UITree.prototype.checkFoldButtonVisible = function()
    {
        let foldButtonRoot = this.foldButton.getRoot();
        let contentRoot = this.content.getRoot();

        if (contentRoot.childNodes.length === 0)
        {
            //foldButtonRoot.style.display = 'none';
            foldButtonRoot.style.visibility = 'hidden';
        }
        else
        {
            //foldButtonRoot.style.display = 'inline-block';
            foldButtonRoot.style.visibility = 'visible';
        }
    };
    
    UITree.prototype.setFold = function(isFold)
    {
        let contentRoot = this.content.getRoot();
        let foldButtonRoot = this.foldButton.getRoot();

        if (isFold)
        {
            contentRoot.style.display = 'none';
            foldButtonRoot.innerHTML = mgs.ICONS.treeCollapse;
        }
        else
        {
            contentRoot.style.display = 'block';
            foldButtonRoot.innerHTML = mgs.ICONS.treeExpand;
        }

        this.isFold = isFold;
    };

    // events
    UITree.prototype._onFoldButtonClick = function(evt)
    {
        evt.stopPropagation();
        this.setFold(!this.isFold);
    };

    UITree.prototype._onDragOver_InsertArea = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

     // console.log('_onDragOver_InsertArea');

        let rootTree = this.getRootTree();
        let rect = this.insertArea.getRoot().getBoundingClientRect();
        let dragHandlerRoot = rootTree.dragHandler.getRoot();
        dragHandlerRoot.style.top = (rect.top) + 'px';
        dragHandlerRoot.style.left = (rect.left) + 'px';
        dragHandlerRoot.style.width = rect.width + 'px';
        dragHandlerRoot.style.height = '0px';
        dragHandlerRoot.style.display = 'block';
    };

    UITree.prototype._onDragLeave_InsertArea= function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        // console.log('_onDragLeave_InsertArea');

        let rootTree = this.getRootTree();
        let dragHandlerRoot = rootTree.dragHandler.getRoot();
        dragHandlerRoot.style.display = 'none';
    };

    UITree.prototype.onDrop_InsertArea = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        let rootTree = this.getRootTree();
        let isCtrlDown = rootTree.isCtrlDown;
        let parentTree = this.getParentTree();
        if (parentTree)
        {
            let exist = mgs.Util.arrayExist(rootTree.selectTrees, rootTree.dragTree);
            
            let trees = null;
            if (exist)
            {
                trees = rootTree.selectTrees.slice();
            }
            else
            {
                if (isCtrlDown)
                {
                    trees = mgs.Math.intersectionSet([rootTree.dragTree], rootTree.selectTrees.slice(), treeCompare);
                }
                else
                {
                    trees = [rootTree.dragTree];
                }
                this._selectTrees(trees);
            }

            let appendTrees = parentTree.getValidAppendTrees(trees);
            appendTrees.sort(treeSort);
            for (let i = 0;i < appendTrees.length;i ++)
            {
                let tree = appendTrees[i];
                if (tree !== this)
                {
                    mgsEditor.emit(mgsEditor.EVENTS.sceneTree.onDropInsertArea, tree.entity, this.entity);
                }
            }
        }
    };

    UITree.prototype._onDrop_InsertArea = function(fid, nid)
    {

        let rootTree = this.getRootTree();

        let trees = rootTree.getTreeByEntityID(fid);
        if(trees.length === 0) return false;
        let firstTree = trees[0];
        trees = rootTree.getTreeByEntityID(nid);
        if(trees.length === 0) return false;
        let nextTree = trees[0];
        let parentTree = nextTree.getParentTree();
        parentTree.insertTree(firstTree, nextTree);
    };
     
    // drag events
    let globalImg = document.createElement("img");
    UITree.prototype._onDragStart_Header = function(evt)
    {
        // evt.preventDefault();
        evt.stopPropagation();

        evt.dataTransfer.setDragImage(globalImg, 0, 0);

        this.getRootTree().dragTree = this;
    };

    UITree.prototype._onDragOver_Header = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        // console.log('_onDragOver_Header');

        let rootTree = this.getRootTree();
        let rect = this.header.getRoot().getBoundingClientRect();
        let dragHandlerRoot = rootTree.dragHandler.getRoot();
        dragHandlerRoot.style.top = (rect.top) + 'px';
        dragHandlerRoot.style.left = (rect.left) + 'px';
        dragHandlerRoot.style.width = (rect.width - 3) + 'px';
        dragHandlerRoot.style.height = rect.height + 'px';
        dragHandlerRoot.style.display = 'block';
    };

    UITree.prototype._onDragLeave_Header = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        // console.log('_onDragLeave_Header');

        let rootTree = this.getRootTree();
        let dragHandlerRoot = rootTree.dragHandler.getRoot();
        dragHandlerRoot.style.display = 'none';
    };

    UITree.prototype.onDrop_Header = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        let rootTree = this.getRootTree();
        let isCtrlDown = rootTree.isCtrlDown;

        let exist = mgs.Util.arrayExist(rootTree.selectTrees, rootTree.dragTree);
            
        let trees = null;
        if (exist)
        {
            trees = rootTree.selectTrees.slice();
        }
        else
        {
            if (isCtrlDown)
            {
                trees = mgs.Math.intersectionSet([rootTree.dragTree], rootTree.selectTrees.slice(), treeCompare);
            }
            else
            {
                trees = [rootTree.dragTree];
            }
            this._selectTrees(trees);
        }
   
        let appendTrees = this.getValidAppendTrees(trees);
        appendTrees.sort(treeSort);
        for (let i = 0;i < appendTrees.length;i ++)
        {
            let tree = appendTrees[i];
            mgsEditor.emit(mgsEditor.EVENTS.sceneTree.onDropHeader, this.entity, tree.entity);
        }
    };

    UITree.prototype._onDrop_Header = function(pid, cid)
    {
        let rootTree = this.getRootTree();
        let trees = rootTree.getTreeByEntityID(pid);
        if(trees.length === 0) return false;
        let parentTree = trees[0];
        trees = rootTree.getTreeByEntityID(cid);
        if(trees.length === 0) return false;
        let childTree = trees[0];
        parentTree.appendTree(childTree);
    };

    UITree.prototype._onDragEnd_Header = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        let rootTree = this.getRootTree();
        let dragHandlerRoot = rootTree.dragHandler.getRoot();
        dragHandlerRoot.style.display = 'none';

        // console.log('_onDragEnd_Header');
    };

    UITree.prototype._onMouseUp_Header = function(evt)
    {
        // evt.preventDefault();
        evt.stopPropagation();
        this.triggerFocusDown(evt);

        let rootTree = this.getRootTree();
        // this._selectTrees([this], rootTree.isCtrlDown, rootTree.isShiftDown);
        this.emit('onSelectTree', [this], rootTree.isCtrlDown, rootTree.isShiftDown);
        // this._select(this, this.isCtrlDown);
    };

    // UITree.prototype._onContextMenu = function(evt)
    // {
    //     evt.preventDefault();
    //     evt.stopPropagation();
    //     this.triggerFocusDown(evt);

    //     let rootTree = this.getRootTree();
    //     this._selectTrees([this], rootTree.isCtrlDown, rootTree.isShiftDown);
    //     console.log('contentMenu');
    //     // this._select(this, this.isCtrlDown);
    // };

    // UITree.prototype.onKeyUp = function(evt)
    // {
    //     this.isCtrlDown = evt.ctrlKey || evt.metaKey;
    //     this.isShiftDown = evt.shiftKey;

    //     console.log('onKeyUp this.isCtrlDown:' + this.isCtrlDown);
    // };

    UITree.prototype.onFirstKeyDown = function(evt)
    {
        this.isCtrlDown = evt.ctrlKey || evt.metaKey;
        this.isShiftDown = evt.shiftKey;

        let rootTree = this.getRootTree();

        // ctrl + a
        if (this.isCtrlDown && evt.keyCode === 65)
        {
            let trees = rootTree.getChildrenTrees();
            // this._selectTrees(trees);
            this.emit('onSelectTree', trees);
        }
        // delete
        else if (evt.keyCode === 46) 
        {
            let trees = rootTree.selectTrees;
            this.emit('onRemoveTree', trees);
            // for (let i = 0;i < rootTree.selectTrees.length;i ++)
            // {
            //     rootTree.removeTree(rootTree.selectTrees[i]);
            //     mgsEditor.emit(mgsEditor.EVENTS.deleteEntity, rootTree.selectTrees[i].entity);
            // }
            // rootTree.selectTrees = [];
        }
    };

    UITree.prototype._removeTrees = function(pid, cid)
    { 

        let rootTree = this.getRootTree();
        let trees = rootTree.getTreeByEntityID(pid);
        if(trees.length === 0) return false;
        let parentTree = trees[0];
        trees = rootTree.getTreeByEntityID(cid);
        if(trees.length === 0) return false;
        childTree = trees[0];
        parentTree.removeTree(childTree);
        
        // for (let i = 0;i < rootTree.selectTrees.length;i ++)
        // {
        //     rootTree.removeTree(rootTree.selectTrees[i]);
        // }
        // rootTree.selectTrees = [];
    };

    UITree.prototype.onRemoveTree = function(trees)
    { 
        for (let i = 0;i < trees.length;i ++)
        {
            mgsEditor.emit(mgsEditor.EVENTS.sceneTree.entityDeleted, trees[i].entity);
        }
    };

    UITree.prototype.onSelectTree = function(trees, isCtrlDown, isShiftDown)
    { 
        mgsEditor.emit(mgsEditor.EVENTS.sceneTree.entitySelected, trees); 
    };

    UITree.prototype.onKeyUp = function(evt)
    {
        this.isCtrlDown = evt.ctrlKey || evt.metaKey;
        this.isShiftDown = evt.shiftKey;

    };

    UITree.prototype.onUnfocus = function(evt)
    {
        this.isCtrlDown = false;
        this.isShiftDown = false;
        // let rootTree = this.getRootTree();

        // // unselect old trees
        // for (let i = 0;i < rootTree.selectTrees.length;i ++)
        // {
        //     let tree = rootTree.selectTrees[i];
        //     tree._unSelect();
        // }
    };

    // select
    UITree.prototype._select = function()
    {
        let headerRoot = this.header.getRoot();
        headerRoot.classList.add('selected');
    };

    UITree.prototype._unSelect = function()
    {
        let headerRoot = this.header.getRoot();
        headerRoot.classList.remove('selected');
    };

    UITree.prototype._getTreeLink = function()
    {
        let treeLink = [];
        let tree = this;
        while (tree)
        {
            treeLink.push(tree);
            tree = tree.getParentTree();
        } 
        treeLink.reverse();

        return treeLink;
    };

    UITree.prototype._getShiftSelectTrees = function(tree, selectTrees)
    {
        if (selectTrees.length <= 0)
        {
            return [tree];
        }

        let rootTree = this.getRootTree();
        let trees = rootTree.getChildrenTrees();
        trees.sort(treeSort);

        let tempSelectTrees = selectTrees.slice();
        tempSelectTrees.sort(treeSort);

        let treeBeginIdx = trees.indexOf(tempSelectTrees[0]);
        let treeEndIdx = trees.indexOf(tempSelectTrees[tempSelectTrees.length - 1]);
        let treeIdx = trees.indexOf(tree);
        let beginIdx = Math.min(treeIdx, treeBeginIdx, treeEndIdx);
        let endIdx = Math.max(treeIdx, treeBeginIdx, treeEndIdx);

        return trees.slice(beginIdx, endIdx + 1);
    };

    UITree.prototype._getShiftSelectTrees = function(tree, selectTrees)
    {
        if (selectTrees.length <= 0)
        {
            return [tree];
        }

        let rootTree = this.getRootTree();
        let trees = rootTree.getChildrenTrees();
        trees.sort(treeSort);

        let tempSelectTrees = selectTrees.slice();
        tempSelectTrees.sort(treeSort);

        let treeBeginIdx = trees.indexOf(tempSelectTrees[0]);
        let treeEndIdx = trees.indexOf(tempSelectTrees[tempSelectTrees.length - 1]);
        let treeIdx = trees.indexOf(tree);
        let beginIdx = Math.min(treeIdx, treeBeginIdx, treeEndIdx);
        let endIdx = Math.max(treeIdx, treeBeginIdx, treeEndIdx);

        return trees.slice(beginIdx, endIdx + 1);
    };

    UITree.prototype._selectTrees = function(trees, isCtrlSelect, isShiftDown)
    {
        let rootTree = this.getRootTree();
        
        let newSelectTrees = null;

        if (isCtrlSelect)
        {
            newSelectTrees = mgs.Math.differenceSet(trees, rootTree.selectTrees, treeCompare);
        }
        else
        {
            if (isShiftDown && trees.length === 1)
            {
                newSelectTrees = this._getShiftSelectTrees(trees[0], rootTree.selectTrees);
            }
            else
            {
                newSelectTrees = trees;
            }  
        }

        // unselect old trees
        for (let i = 0;i < rootTree.selectTrees.length;i ++)
        {
            let tree = rootTree.selectTrees[i];
            tree._unSelect();
        }

        // ordered
        newSelectTrees.sort(treeSort);

        // select new trees
        for (let i = 0;i < newSelectTrees.length;i ++)
        {
            let tree = newSelectTrees[i];
            tree._select();
        }

        rootTree.selectTrees = newSelectTrees;
    };

    UITree.prototype._selectTreesByID = function(id)
    {
        let rootTree = this.getRootTree();
        
        let newSelectTrees = rootTree.getTreeByEntityID(id);
        this._selectTrees(newSelectTrees);
    };

    UITree.prototype.hasParent = function(trees)
    {
        for (let i = 0;i < trees.length;i ++)
        {
            let tree = trees[i];
            if (this.getParentTree() === tree)
            {
                return true;
            }
        }

        return false;
    };

    UITree.prototype.getValidAppendTrees = function(trees)
    {
        // self is the target to append

        // remove children trees
        let removeChildrenTrees = [];
        for (let i = 0;i < trees.length;i ++)
        {
            let tree = trees[i];
            if 
            (
                tree.hasParent(trees) || 
                tree === this || 
                tree.isAncestorOf(this) ||
                tree.getRootTree() != this.getRootTree()
            )
            {
                removeChildrenTrees.push(tree);
            }
        }

        for (let i = 0;i < removeChildrenTrees.length;i ++)
        {
            let tree = removeChildrenTrees[i];
            mgs.Util.arrayRemove(trees, tree);
        }

        return trees;
    };

    // items
    UITree.prototype.canAppend = function(tree)
    {
        if (tree.getClass() !== UITree)
        {
            return false;
        }

        if (tree.getRootTree() !== this.getRootTree())
        {
            return false;
        }

        if (tree.isAncestorOf(this))
        {
            return false;
        }

        if (tree === this)
        {
            return false;
        }

        return true;
    };

    UITree.prototype.insertTree = function(tree, refTree)
    {
        if (!this.canAppend(tree))
        {
            return null;
        }

        tree.detachFromParentTree();

        let returnTree = this.content.insertBefore(tree, refTree);
        return returnTree;
    };

    UITree.prototype.appendTree = function(tree, nextTree)
    {
        if (!this.canAppend(tree))
        {
            return null;
        }

        tree.detachFromParentTree();

        let returnTree;
        if(nextTree)
        {
            returnTree = this.content.insertBefore(tree, nextTree);
        } 
        else
        {
            returnTree = this.content.append(tree);
        }
        
        if (returnTree)
        {
            this.setFold(false);
            this.checkFoldButtonVisible();
        }

        return returnTree;
    };

    UITree.prototype.removeTree = function(tree)
    {
        let parentTree = tree.getParentTree();
        let returnTree = parentTree.content.remove(tree);
        if (returnTree)
        {
            parentTree.checkFoldButtonVisible();
        }

        return returnTree;
    };

    UITree.prototype.getParentTree = function()
    {
        if (this.getRootTree() === this)
        {
            return null;
        }

        let parentTreeContent = this.getParent();
        if (parentTreeContent)
        {
            let parentTree = parentTreeContent.getParent();
            return parentTree;
        }

        return null;
    };

    UITree.prototype.getChildrenTrees = function()
    {
        let trees = this.content.getChildren();
        for (let i = 0;i < trees.length;i ++)
        {
            let tree = trees[i];
            trees.push(...tree.getChildrenTrees());
        }

        return trees;
    };

    UITree.prototype.getTreeByEntityID = function(id)
    {
        let treeArr = [];
        let rootTree = this.getRootTree();
        treeArr.push(rootTree);
        let trees = this.content.getChildren();
        treeArr.push(...trees);
        for (let i = 0;i < trees.length;i ++)
        {
            let tree = trees[i];
            let children = tree.getChildrenTrees();
            treeArr.push(...children);
        }
        
        for(let i = 0; i < treeArr.length; i++)
        {
            let tree = treeArr[i];
            if(tree.entity && tree.entity.uniqueID === id)
            {
                return [tree];
            }
        }
        return [];
    };


    UITree.prototype.detachFromParentTree = function()
    {
        let parentTree = this.getParentTree();
        if (parentTree)
        {
            parentTree.removeTree(this);
        }
    };

    UITree.prototype.getSelectTrees = function()
    {
        let rootTree = this.getRootTree();
        return rootTree.selectTrees;
    }

    

}());