(function ()
{
    let UIMenuList = function (options) 
    {
        mgs.UIOverlay.call(this, options);
        
        this.options = options ? options : {};

        // events
        this.onShow = function(bShow)
        {
            if (bShow)
            {
                for (let i in this.childListInfos)
                {
                    let childListInfo = this.childListInfos[i];
                    childListInfo.list.getRoot().style.display = 'none';
                }
            }
        };

        this.show(false);
    };
    mgs.classInherit(UIMenuList, mgs.UIOverlay);

    UIMenuList.prototype.getFitPosition = function(pos, windowSize, selfSize)
    {
        let offset = 2;

        if (pos.x + selfSize.w + offset > windowSize.w)
        {
            pos.x = windowSize.w - selfSize.w - offset;
            pos.xOut = true;
        }


        if (pos.y + selfSize.h + offset > windowSize.h)
        {
            pos.y = windowSize.h - selfSize.h - offset;
        }

        return pos;
    };

    UIMenuList.prototype.adjustListPosition = function(relativePos, list, test)
    {
        let root = list.getRoot();
        let rect = list.getRoot().getBoundingClientRect();
        let parentRect = root.parentNode.getBoundingClientRect();
        let parentPos = 
        {
            x : parentRect.left,
            y : parentRect.top,
        };

        root.style.display = 'block';

        let absolutePos =
        {
            x : parentPos.x + relativePos.x,
            y : parentPos.y + relativePos.y,
        };
        let windowSize = 
        {
            w : window.innerWidth,
            h : window.innerHeight,
        };
        let listSize = 
        {
            w : rect.width,
            h : rect.height,
        };
        let fitPos = this.getFitPosition(absolutePos, windowSize, listSize);

        if (!test)
        {
            root.style.left = (fitPos.x - parentPos.x) + 'px';
            root.style.top = (fitPos.y - parentPos.y) + 'px';
        }

        return fitPos;
    };

    UIMenuList.prototype.setMenuList = function(menuListInfo, parent, level)
    {
        if (!parent)
        {  
            this.childListInfos = [];
            this.clear();
        }

        level = level || 0;

        let list = new mgs.UIList();
        list.getRoot().style.position = 'absolute';

        for (let i in menuListInfo)
        {
            let menuItemInfo = menuListInfo[i];
            let item = list.appendItem();
            item.info = menuItemInfo.info;
            item.level = level;

            if (this.options.onAppend)
            {
                this.options.onAppend(item);
            }

            if (menuItemInfo.menuListInfo)
            {
                let extendIcon = document.createElement('div');
                extendIcon.classList.add('UIMenuList-item-extend');
                extendIcon.innerHTML = mgs.ICONS.menuListItemExtend;
                item.appendElement(extendIcon);

                this.setMenuList(menuItemInfo.menuListInfo, item, level + 1);
            }
        }

        // bind events
        let self = this;
        list.onHover = function(idx, item)
        {
            if (self.onHover)
            {
                self.onHover(idx, item);
            }

            // 隐藏后面层级列表
            for (let i in self.childListInfos)
            {
                let childListInfo = self.childListInfos[i];
                if (childListInfo.level > item.level)
                {
                    if (childListInfo.list !== item.childList)
                    {
                        childListInfo.list.getRoot().style.display = 'none';
                    }
                }
            }

            if (item.childList)
            {
                // 显示子列表
                var itemRect = item.getRoot().getBoundingClientRect();
                item.childList.getRoot().style.display = 'block';
                
                let relativePos =
                {
                    x : itemRect.width,
                    y : 0,
                };
                let fitPos = self.adjustListPosition(relativePos, item.childList, true);
                if (fitPos.xOut)
                {
                    relativePos.x = -itemRect.width;
                }
                self.adjustListPosition(relativePos, item.childList);
            }
        };

        list.onSelect = function(idx, item)
        {
            if (self.onSelect)
            {
                if (!item.childList)
                {
                    self.onSelect(idx, item);

                    if (!self.options.notHideWhenSelect)
                    {
                        self.show(false);
                    }
                }
            }
        };

        // append parent
        if (parent)
        {  
            parent.append(list);
            parent.childList = list;
            this.childListInfos.push({level:level, list:list});
        }
        else
        {
            this.rootList = list;
            this.append(list);
        }
    };

    UIMenuList.prototype.filterItems = function(filter, list)
    {
        list = list ? list : this.rootList;
        if (list)
        {
            let items = list.getChildUIs();
            for (let i = 0;i < items.length;i ++)
            {
                let item = items[i];
                item.getRoot().style.display = filter(item) ? 'none' : 'block';
                if (item.childList)
                {
                    this.filterItems(filter, item.childList);
                }
            }
        }
    };

    UIMenuList.prototype.open = function(x, y)
    {
        this.show(true);

        let left = x + 1;
        let top = y;

        if (this.rootList)
        {
            let absolutePos =
            {
                x : x + 1,
                y : y,
            };
            this.adjustListPosition(absolutePos, this.rootList);
        }
    };
}());
