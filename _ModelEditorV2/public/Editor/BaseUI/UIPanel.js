(function ()
{
    let UIPanel = function (options) 
    {
        mgs.UILayout.call(this, options);

        options = options ? options : {};

        // create chidlren
        this.header =  new mgs.UIBase({id:'header', styleClass:'UIPanel-header'});
        this.append(this.header);

        this.content =  new mgs.UIBase({id:'content', styleClass:'UIPanel-content'});
        this.append(this.content);

        this.foldButton =  new mgs.UIBase({id:'foldButton', styleClass:'UIPanel-foldButton', innerHTML:mgs.ICONS.foldButtonBottom});
        this.header.append(this.foldButton);

        this.title = new mgs.UIBase({id:'title', styleClass:'UIPanel-title', innerHTML:options.text ? options.text: 'TITLE'});
        this.header.append(this.title);

        if (options.childStyleOptions && options.childStyleOptions.header)
        {
            if (options.childStyleOptions.header.height)
            {
                // adjust content
                this.content.getRoot().style.top = options.childStyleOptions.header.height;

                // adjust foldButton
                this.foldButton.getRoot().style['line-height'] = options.childStyleOptions.header.height;
                this.title.getRoot().style['line-height'] = options.childStyleOptions.header.height;
            }
        }

        // process style options
        this.processChildStyleOptions();

        // bind events
        this.foldButton.getRoot().addEventListener('click', this._onFoldButtonClick.bind(this), this);

        // get origin values
        this.foldOption = options.foldOption;
        this.getOriginValues();
        this.setFold(options.isFold);    
    };
    mgs.classInherit(UIPanel, mgs.UILayout);

    UIPanel.prototype.getOriginValues = function()
    {
        let root = this.getRoot();
        let contentRoot = this.content.getRoot();
        let headerRoot = this.header.getRoot();

        this.rootOriginWidth = root.style.width;
        this.rootOriginHeight = root.style.height;
        this.rootOriginOverflow = root.style.overflow;
        this.contentOriginDisplayStyle = contentRoot.style.display;
        this.contentOverflowStyle = contentRoot.style.overflow;
        this.headerOriginWidth = headerRoot.style.width;
        this.headerOriginPosition = headerRoot.style.position;
        this.headerOriginLeft = headerRoot.style.left;
        this.headerOriginTop = headerRoot.style.top;
        this.headerOriginTransformOrigin = headerRoot.style['transform-origin'];
        this.headerOriginTransform = headerRoot.style.transform;
    };

    UIPanel.prototype.contentAppend = function(ui)
    {
        this.content.append(ui);
    };

    UIPanel.prototype.contentClear = function()
    {
        this.content.clear();
    };

    UIPanel.prototype._onFoldButtonClick = function(evt)
    {
        this.setFold(!this.isFold);
    };

    UIPanel.prototype.setFold = function(isFold)
    {
        let root = this.getRoot();
        let contentRoot = this.content.getRoot();
        let headerRoot = this.header.getRoot();
        let foldButtonRoot = this.foldButton.getRoot();
        let handlerRoot =  this.handler ? this.handler.getRoot() : null;

        if (isFold)
        {
            this.getOriginValues(); 
        }

        if (this.foldOption)
        {
            if (this.foldOption.horizontal)
            {
                if (isFold)
                {
                    root.style.height = headerRoot.style.height;
                    root.style.overflow = 'hidden';
                    contentRoot.style.overflow = 'hidden';
                }
                else
                {
                    root.style.height = this.rootOriginHeight;
                    root.style.overflow = this.rootOriginOverflow;
                    contentRoot.style.overflow = this.contentOverflowStyle;
                }
            }

            if (this.foldOption.vertial)
            {
                if (isFold)
                {
                    let headerRect = headerRoot.getBoundingClientRect();
                    let halfHeight = headerRect.height/2;
                    
                    headerRoot.style.width = '10000px';
                    headerRoot.style.position = 'absolute';
                    headerRoot.style.left = '0px';
                    headerRoot.style.top = '0px';
                    headerRoot.style['transform-origin'] = halfHeight + 'px' + ' ' + halfHeight + 'px';
                    headerRoot.style.transform = 'rotate(90deg)';

                    contentRoot.style.display = 'none';

                    root.style.width = headerRect.height + 'px';
                }
                else
                {
                    headerRoot.style.width = this.headerOriginWidth;
                    headerRoot.style.position = this.headerOriginPosition;
                    headerRoot.style.left = this.headerOriginLeft;
                    headerRoot.style.top = this.headerOriginTop;
                    headerRoot.style['transform-origin'] = this.headerOriginTransformOrigin;
                    headerRoot.style.transform = this.headerOriginTransform;

                    contentRoot.style.display = this.contentOriginDisplayStyle;

                    root.style.width = this.rootOriginWidth;
                }
            }
        }

        if (isFold)
        {
            if (handlerRoot)
            {
                handlerRoot.style.display = 'none';
            }
            foldButtonRoot.innerHTML = mgs.ICONS.foldButtonRight
            

            if (this.foldOption && this.foldOption.buttonDirection)
            {
                foldButtonRoot.innerHTML = mgs.ICONS['foldButton' + this.foldOption.buttonDirection];
            }
        }
        else
        {
            if (handlerRoot)
            {
                handlerRoot.style.display = 'inline-block';
            }
            foldButtonRoot.innerHTML = mgs.ICONS.foldButtonBottom;

        }
        
        this.isFold = isFold;
    };
}());