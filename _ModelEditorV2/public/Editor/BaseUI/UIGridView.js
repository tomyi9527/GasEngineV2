(function ()
{
    let UIGridItem = function(options)
    {
        options = options ? options : {};
        mgs.UIBase.call(this, options);  

        let root = this.getRoot();
        root.draggable = true;
        if (options.styleClass)
        {
            root.classList.add(options.styleClass);
        }

        this._info = options.info;

        let self = this;
        root.addEventListener('mousedown', function(evt)
        {
            evt.stopPropagation();
            self.triggerFocusDown(evt);

            self.emit('onMouseDown', self, evt);
        }, false);
        root.addEventListener('mouseup', function(evt)
        {
            self.emit('onMouseUp', self, evt);
        }, false);
        root.addEventListener('dragstart', function(evt)
        {
            self.emit('onDragStart', self, evt);
        }, false);
        root.addEventListener('dragover', function(evt)
        {
            self.emit('onDragOver', self, evt);
        }, false);
        root.addEventListener('drop', function(evt)
        {
            self.emit('onDrop', self, evt);
        }, false);
        root.addEventListener('dblclick', function(evt)
        {
            self.emit('onDoubleClick', self, evt);
        }, false);
    }
    mgs.classInherit(UIGridItem, mgs.UIBase);

    UIGridItem.prototype.getInfo = function()
    {
        return this._info;
    };

    UIGridItem.prototype.getID = function()
    {
        return this._id;
    };

    let UIGridView = function(options) 
    {
        options = options ? options : {};
        options.canFocus = true;
        mgs.UIBase.call(this, options);

        this._itemMap = {};
        this._selectItems = {};
        this._selectIDs = [];

        let root = this.getRoot();
        root.addEventListener('mousedown', this._onMouseDown.bind(this), false);
    };
    mgs.classInherit(UIGridView, mgs.UIBase);

    UIGridView.prototype.appendItem = function(options)
    {
        let realItemClass = options.itemClass ? options.itemClass : mgs.UIGridItem;
        let item = new realItemClass(options);
        let id = options.hasOwnProperty('id') ? options.id : item.uuid;
        item._id = id;
        this._itemMap[id] = item;
        this.append(item);

        // events
        item.on('onMouseDown', this._onChildMouseDown.bind(this));
        item.on('onMouseUp', this._onChildonMouseUp.bind(this));
        item.on('onDragStart', this._onChildDragStart.bind(this));
        item.on('onDragOver', this._onChildDragOver.bind(this));
        item.on('onDrop', this._onChildDrop.bind(this));
        item.on('onDoubleClick', this._onChildDoubleClick.bind(this));

        return item;
    };

    UIGridView.prototype.removeItem = function(item)
    {
        let id = item.getID();
        this.removeItemByID(id);
    };

    UIGridView.prototype.removeItemByID = function(id)
    {
        let item = this.getItem(id);
        if (!item)
        {
            return;
        }

        this.remove(item);
        delete this._itemMap[id];
    };

    UIGridView.prototype.getItem = function(id)
    {
        return this._itemMap[id];
    };

    UIGridView.prototype.getSelectItemIDList = function()
    {
        let list = [];
        for (let key in this._selectItems)
        {
            list.push(this._selectItems[key].getID());
        }
        return list;
    };

    UIGridView.prototype.getAllItemIDList = function()
    {
        let list = [];
        for (let key in this._itemMap)
        {
            list.push(this._itemMap[key].getID());
        }
        return list;
    };

    UIGridView.prototype.getShiftSelectItemIDList = function(newID)
    {
        let selectList = this.getSelectItemIDList();

        if (selectList.length <= 0)
        {
            return [newID];
        }

        let self = this;
        selectList.sort(function(id0, id1)
        {
            let item0 = self.getItem(id0);
            let item1 = self.getItem(id1);
            let idx0 = self.getChildIndex(item0);
            let idx1 = self.getChildIndex(item1);
            return idx0 - idx1;
        });

        let beginIndex = this.getChildIndex(this.getItem(selectList[0]));
        let endIndex = this.getChildIndex(this.getItem(selectList[selectList.length - 1]));
        let newIndex = this.getChildIndex(this.getItem(newID));

        if (newIndex < beginIndex)
        {
            endIndex = beginIndex;
            beginIndex = newIndex;
        }
        else
        {
            endIndex = newIndex;
        }

        let list = [];
        for (let key in this._itemMap)
        {
            let item = this._itemMap[key];
            let itemIndex = this.getChildIndex(item);
            if (itemIndex >= beginIndex && itemIndex <= endIndex)
            {
                list.push(item.getID());
            }
        }

        return list;
    };

    UIGridView.prototype.isItemSelected = function(id)
    {
        return !!this._selectItems[id];
    };

    ///////////////////////////////////////////// select events /////////////////////////////////////////////
    UIGridView.prototype.onFirstKeyDown = function(evt)
    {
        this.isCtrlDown = evt.ctrlKey || evt.metaKey;
        this.isShiftDown = evt.shiftKey;

        // https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/keyCode
        if (this.isCtrlDown && 
            evt.code === 'KeyA')
        {
            let list = this.getAllItemIDList();
            this.emit('onSelectCmd', list);
        }
    };

    UIGridView.prototype.onKeyUp = function(evt)
    {
        this.isCtrlDown = evt.ctrlKey || evt.metaKey;
        this.isShiftDown = evt.shiftKey;
    };

    UIGridView.prototype.onUnfocus = function(evt)
    {
        this.isCtrlDown = false;
        this.isShiftDown = false;
    };

    UIGridView.prototype._onMouseDown = function(evt)
    {
        this.emit('onSelectCmd', null);
    };

    UIGridView.prototype._onChildMouseDown = function(item, evt)
    {
     
    };

    UIGridView.prototype._onChildonMouseUp = function(item, evt)
    {
        let selectItemIDList = this.getSelectItemIDList();
        let id = item.getID();

        if (this.isCtrlDown)
        {
            if (this.isItemSelected(id))
            {
                mgs.Util.arrayRemove(selectItemIDList, id);
                this.emit('onSelectCmd', selectItemIDList);
            }
            else
            {
                selectItemIDList.push(id);
                this.emit('onSelectCmd', selectItemIDList);
            }
        }
        else
        {
            if (this.isShiftDown)
            {
                let shiftSelectItemIDList = this.getShiftSelectItemIDList(id);
                this.emit('onSelectCmd', shiftSelectItemIDList);    
            }
            else
            {
                this.emit('onSelectCmd', [id]);    
            }
        }
    };

    UIGridView.prototype._onChildDragStart = function(item, evt)
    {
        this.emit('onSelectCmd', null);
    };

    UIGridView.prototype._onChildDragOver = function(item, evt)
    {

    };

    UIGridView.prototype._onChildDrop = function(item, evt)
    {

    };

    UIGridView.prototype._onChildDoubleClick = function(item, evt)
    {
        this.emit('onOpenItem', item);
    };
  
    // do select
    UIGridView.prototype.selectItems = function(idList, isSelect)
    {
        for (let i = 0;i < idList.length;i ++)
        {
            let id = idList[i];
            let item = this.getItem(id);
            if (item)
            {
                item.emit('onSelect', isSelect);

                if (isSelect)
                {
                    if (!this._selectItems[id])
                    {
                        this._selectIDs.push(id);
                        this._selectItems[id] = item;
                    }
                }
                else
                {
                    if (this._selectItems[id])
                    {
                        mgs.Util.arrayRemove(this._selectIDs, id);
                        delete this._selectItems[id];
                    }
                    
                }
            }
        }
    };

}());