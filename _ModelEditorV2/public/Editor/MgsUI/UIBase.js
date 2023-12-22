(function ()
{
    let UIBaseGlobalID = 0;

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
}());