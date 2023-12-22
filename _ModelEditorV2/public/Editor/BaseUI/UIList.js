(function ()
{
    /*************************************** list item ***************************************/
    let UIListItem = function(container, options) 
    {
        mgs.UIBase.call(this, options);
        
        let root = this.getRoot();

        // set options
        this.container = container;
        options = options ? options : {};

        root.addEventListener('mouseover', this._onMouseOver.bind(this), false);
        root.addEventListener('mousedown', this._onMouseDown.bind(this), false);
        root.addEventListener('contextmenu', this._onContextMenu.bind(this), false);
    };
    mgs.classInherit(UIListItem, mgs.UIBase);

    UIListItem.prototype._onMouseOver = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        if (this.container._onHover)
        {
            this.container._onHover(this);
        }
    };

    UIListItem.prototype._onMouseDown = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        if (this.container._onSelect)
        {
            this.container._onSelect(this);
        }
    };

    UIListItem.prototype._onContextMenu = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        if (this.container._onContextMenu)
        {
            this.container._onContextMenu(this);
        }
    };
    

    /*************************************** list ***************************************/
    let UIList = function(options) 
    {
        mgs.UIBase.call(this, options);
        
        // let root = this.getRoot();

        // set options
        options = options ? options : {};
        this.itemOptions = options.itemOptions;

        // bind events
    };
    mgs.classInherit(UIList, mgs.UIBase);

    UIList.prototype._onHover = function(item)
    {
        if (this.onHover)
        {
            let index = this.getIndex(item);
            this.onHover(index, item);
        }
    }

    UIList.prototype._onSelect = function(item)
    {
        if (this.onSelect)
        {
            let index = this.getIndex(item);
            this.onSelect(index, item);
        }
    };

    UIList.prototype.appendItem = function()
    {
        let item = new UIListItem(this, this.itemOptions);
        this.append(item);
        item.parentList = this;
        return item;
    };

    UIList.prototype.insertItemBefore = function(index)
    {
        let item = new UIListItem(this, this.itemOptions);
        let existItem = this.get(index);
        return this.insertBefore(item, existItem);
    };

    UIList.prototype.getItem = function(index)
    {
        return this.get(index);
    };

    UIList.prototype.removeItem = function(index)
    {
        let item = this.get(index);
        if (!item)
        {
            return null;
        }

        return this.remove(item);
    };
}());

