(function ()
{
    // 统一处理focus事件（focus下可响应键盘）

    // global UIBase infos
    let UIBaseGlobalID = 0;
    let focusUI = null;

    let globalSelectRect = document.createElement('div');
    document.body.append(globalSelectRect);
    globalSelectRect.classList.add('UIGrid-SelectRect');

    let UIBase = function (options) 
    {
        mgs.Events.call(this);

        options = options ? options : {};

        // create root
        let tag = options.elementTag ? options.elementTag : 'div';
        this._root = document.createElement(tag);
        this._root.baseUI = this;

        // set innerhtml
        if (options.innerHTML)
        {
            this._root.innerHTML = options.innerHTML;
        }

        // set root class style
        let currentClass = this.getClass();
        while(currentClass)
        {
            this._root.classList.add(currentClass.prototype.getClassName());
            currentClass = currentClass.prototype.getSuperClass();
        }

        if (options.styleClass)
        {
            this._root.classList.add(options.styleClass);
        }

        if (options.childStyleOptions)
        {
            this.childStyleOptions = options.childStyleOptions;
        }

        if (options.childStyleClasses)
        {
            this.childStyleClasses = options.childStyleClasses;
        }

        // set custom style
        UIBaseGlobalID ++;
        this._root.id = options.id ? options.id : this.getClassName() + UIBaseGlobalID;
        this.processStyleOptions(this._root, options.styleOptions);

        if (options.updateScrollWidth)
        {
            let interval = 200;
            let self = this;
            setInterval(() => {
                self._updateScrollWidth();
            }, interval);
        }

        // events
        this.canFocus = options.canFocus;
    };
    mgs.classInherit(UIBase, mgs.Events);

    ///////////////// events /////////////////
    var keysdown = {};

    let onFocusKeyDown = function(evt)
    {
        if (focusUI)
        {
            if (!keysdown[evt.keyCode])
            {
                keysdown[evt.keyCode] = true;

                // first keydown
                // if (focusUI.onFirstKeyDown)
                // {
                //     focusUI.onFirstKeyDown(evt);
                // }
                focusUI.emit('onFirstKeyDown', evt);
            }

            focusUI.emit('onKeyDown', evt);
            // if (focusUI.onKeyDown)
            // {
            //     focusUI.onKeyDown(evt);
            // }
        }
    };

    let onFocusKeyUp = function(evt)
    {
        if (focusUI)
        {
            delete keysdown[evt.keyCode];

            focusUI.emit('onKeyUp', evt);
            // if (focusUI.onKeyUp)
            // {
            //     focusUI.onKeyUp(evt);
            // }
        }
    };

    let _checkFocus = function(evt)
    {
        let x = evt.clientX, y = evt.clientY;
        let elementMouseIsOver = document.elementFromPoint(x, y);

        // find new focus ui
        let newFocusUI = null;
        let checkUI = elementMouseIsOver.baseUI;
        while (checkUI)
        {
            if (checkUI.canFocus)
            {
                newFocusUI = checkUI;
            }
            checkUI = checkUI.getParent();
        }   

        // reset focus ui
        if (newFocusUI !== focusUI)
        {
            if (focusUI)
            {
                focusUI.unFocus();
            }

            focusUI = newFocusUI;
            window.addEventListener('keydown', onFocusKeyDown, false);
            window.addEventListener('keyup', onFocusKeyUp, false);
        }
    };
    window.addEventListener('mousedown', _checkFocus, true);

    UIBase.prototype.triggerFocusDown = function(evt)
    {
        _checkFocus(evt);
    };

    UIBase.prototype.isFocused = function()
    {
        return focusUI === this;
    };

    UIBase.prototype.unFocus = function()
    {
        if (this.isFocused())
        {
            if (this.onUnfocus)
            {
                this.onUnfocus();
            }

            window.removeEventListener('keydown', onFocusKeyDown, false);
            window.removeEventListener('keyup', onFocusKeyUp, false);
            keysdown = {};
            focusUI = null;
        }
    };

    ///////////////// style operation /////////////////
    UIBase.prototype.processStyleOptions = function(element, styleOptions)
    {
        if (styleOptions)
        {
            for (let key in styleOptions)
            {
                element.style[key] = styleOptions[key];
            }
        }
    };

    UIBase.prototype.processChildStyleOptions = function()
    {
        if (this.childStyleOptions)
        {
            for (let key in this.childStyleOptions)
            {
                let styleOptions = this.childStyleOptions[key];
                if (styleOptions)
                {
                    let ui = this.getChildByID(key);
                    if (ui)
                    {
                        this.processStyleOptions(ui.getRoot(), styleOptions);
                    }
                }
            }
        }

        if (this.childStyleClasses)
        {
            for (let key in this.childStyleClasses)
            {
                let childStyleClass = this.childStyleClasses[key];
                if (childStyleClass)
                {
                    let ui = this.getChildByID(key);
                    if (ui)
                    {
                        ui.getRoot().classList.add(childStyleClass);
                    }
                }
            }
        }
    };

    UIBase.prototype.getRoot = function()
    {
        return this._root;
    };

    
    ///////////////// element operation /////////////////
    UIBase.prototype.appendElement = function(element)
    {
        return this._root.appendChild(element);
    };

    UIBase.prototype.insertElementBefore = function(element, existElement)
    {
        let insertElement = this._root.insertBefore(element, existElement);
        return insertElement;
    };

    UIBase.prototype.removeElement = function(element)
    {
        if (element.parentNode != this._root)
        {
            return false;
        }

        let ui = element.baseUI;
        if (ui)
        {
            ui.clear();
            if (ui.onRemove)
            {
                ui.onRemove();
            }
        }

        return this._root.removeChild(element);
    };

    UIBase.prototype.getElement = function(index)
    {
        return this._root.childNodes[index];
    };

    UIBase.prototype.appendToElement = function(element)
    {
        element.appendChild(this._root);
    };

    UIBase.prototype.getElementIndex = function(element)
    {
        let childNodes = this._root.childNodes;
        let childCount = childNodes.length;

        for (let i = 0;i < childCount;i ++)
        {
            if (childNodes[i] === element)
            {
                return i;
            }
        }

        return null;
    };

    ///////////////// children operation /////////////////
    UIBase.prototype.append = function(newUI)
    {
        if (newUI._root.parentNode === this._root)
        {
            return null;
        }

        if (newUI.isAncestorOf(this))
        {
            return null;
        }

        let element = this._root.appendChild(newUI._root);
        if (element)
        {
            return newUI;
        }
    };

    UIBase.prototype.insert = function(newUI, index)
    {
        if (newUI._root.parentNode === this._root)
        {
            return null;
        }

        let count = this._root.childNodes.length;
        if (index >= count)
        {
            this.append(newUI);
        }
        else
        {
            let existUI = this.getChildByIndex(index);
            this.insertBefore(newUI, existUI);
        }
    }

    UIBase.prototype.insertBefore = function(newUI, existUI)
    {
        if (newUI._root.parentNode === this._root ||
            existUI._root.parentNode !== this._root)
        {
            return null;
        }

        let element = this._root.insertBefore(newUI._root, existUI._root);
        if (element)
        {
            return newUI;
        }
    };

    UIBase.prototype.insertAfter = function(newUI, existUI) 
    {
        if (newUI._root.parentNode === this._root ||
            existUI._root.parentNode !== this._root)
        {
            return null;
        }

        if (this._root.lastChild == existUI._root) 
        {
            this._root.appendChild(newUI._root);
        } 
        else 
        {
            this._root.insertBefore(newUI._root, existUI._root.nextSibling);
        }
    }

    UIBase.prototype.remove = function(ui)
    {
        if (ui._root.parentNode !== this._root)
        {
            return false;
        }

        if (ui.onRemove)
        {
            ui.onRemove();
        }

        return this._root.removeChild(ui._root);
    };

    UIBase.prototype.removeByIndex = function(index)
    {
        let ui = this.getChildByIndex(index);
        if (ui)
        {
            return this.remove(ui);
        }
    };

    UIBase.prototype.removeByID = function(id)
    {
        let ui = this.getChildByID(id);
        if (ui)
        {
            return this.remove(ui);
        }
    };

    UIBase.prototype.getChildByIndex = function(index)
    {
        let element = this._root.childNodes[index];
        if (element && element.baseUI)
        {
            return element.baseUI;
        }
    };

    UIBase.prototype.getChildByID = function(id)
    {
        for (let i = 0;i < this._root.childNodes.length;i ++)
        {
            if (this._root.childNodes[i].id === id)
            {
                return this._root.childNodes[i].baseUI;
            }
        }

        return null;
    };

    UIBase.prototype.getChildIndex = function(ui)
    {
        for (let i = 0;i < this._root.childNodes.length;i ++)
        {
            if (this._root.childNodes[i] === ui._root)
            {
                return i;
            }
        }

        return null;
    };

    UIBase.prototype.getIndex = function()
    {
        let parentUI = this._root.parentNode ? this._root.parentNode.baseUI : null;
        if (parentUI)
        {
            return parentUI.getChildIndex(this);
        }

        return null;
    };

    UIBase.prototype.clear = function()
    {
        // remove all childs
        for (let i = this._root.childNodes.length - 1;i >= 0;i--)
        {
            let element = this._root.childNodes[i];
            if (element.baseUI)
            {
                this.remove(element.baseUI);
            }
        }
    };

    UIBase.prototype.getChildren = function()
    {
        let children = [];

        for (let i = 0;i < this._root.childNodes.length;i ++)
        {
            let element = this._root.childNodes[i];
            if (element && element.baseUI)
            {
                children.push(element.baseUI);
            }
        }

        return children;
    };

    UIBase.prototype.getParent = function()
    {
        if (this._root.parentNode)
        {
            return this._root.parentNode.baseUI;
        }

        return null;
    };

    UIBase.prototype.isAncestorOf  = function(ui)
    {
        let parentNode = ui._root.parentNode;
        while(parentNode)
        {
            if (parentNode.baseUI === this)
            {
                return true;
            }
            parentNode = parentNode.parentNode;
        }

        return false;
    };

     ///////////////// auto update /////////////////
     UIBase.prototype._updateScrollWidth = function()
     {
         let root = this.getRoot();
         root.style.width = '';
         if (root.parentNode.clientWidth - 1 > root.scrollWidth)
         {
             root.style.width = (root.parentNode.clientWidth - 1) + 'px';
         }
         else
         {
             root.style.width = (root.scrollWidth) + 'px';
         }
     }

    ///////////////// selector /////////////////
    UIBase.showSelectRect = function(bShow)
    {
        globalSelectRect.style.display = bShow ? 'block' : 'none';
    };

    UIBase.setSelectRect = function(rect)
    {
        globalSelectRect.style.left = rect.left;
        globalSelectRect.style.top = rect.top;
        globalSelectRect.style.width = rect.width;
        globalSelectRect.style.height = rect.height;
    };
}());