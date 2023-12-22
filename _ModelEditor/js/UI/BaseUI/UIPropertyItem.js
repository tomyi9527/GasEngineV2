(function ()
{
    let UIPropertyItem = function(options) 
    {
        let defaultLableStyle = {
            width: '20%',
            display: 'inline-block',
            'min-width': '60px'
        };
        let defaultTextFieldStyle = {
            width: '70%',
            overflow: 'hidden',
            'min-width': '120px'
        };
        let labelOptions = 
        {
            elementTag: 'span',
            innerHTML: options.innerHTML,
            styleOptions: options.lableStyle || defaultLableStyle,
        };
    
        let textFieldOptions = 
        {
            id: options.id,
            text: options.text,
            styleOptions: options.textFieldSyyle || defaultTextFieldStyle
        };

        mgs.UIBase.call(this);
        var typeSpan = new mgs.UIBase(labelOptions);
        this.append(typeSpan);
        this.textInput = new mgs.UITextField(textFieldOptions);
        this.textInput.onChange = function(value)
        {
            // this.emit('propertyChange', options.id, value);
            // options.setFun(value);
            mgsEditor.emit(mgsEditor.EVENTS.attribute.changed, options.id, value);
        }
        this.append(this.textInput);
    };
    mgs.classInherit(UIPropertyItem, mgs.UIBase);

    UIPropertyItem.prototype.getTextField = function()
    {
        return this.textInput;
    }
}());