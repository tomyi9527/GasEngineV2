(function ()
{
    let MAX_RECORD_COUNT = 50;

    let CommandHistory = function(commandManager) 
    {
        mgs.Object.call(this);

        this._commandManager = commandManager;

        this._historyStack = [];
        this._currentIndex = -1;
    };
    mgs.classInherit(CommandHistory, mgs.Object);

    CommandHistory.prototype.add = function(commands)
    {
        // pop当前操作位置后的所有元素（新增操作，redo信息将失效）
        if (this._historyStack.length > 0)
        {
            let lenMinusOne = this._historyStack.length - 1;
            let redoCount = lenMinusOne - this._currentIndex;
            if (redoCount > 0)
            {
                this._historyStack.splice(this._currentIndex + 1, redoCount);
            }
        }

        // push新元素，将当前操作移动到最后一位
        this._historyStack.push(commands);

        // 限制个数
        if (this._historyStack.length > MAX_RECORD_COUNT)
        {
            this._historyStack.splice(0, 1);
        }

        this._currentIndex = this._historyStack.length - 1;
    };

    CommandHistory.prototype.undo = function()
    {
        if (this._currentIndex >= 0)
        {
            let commands = this._historyStack[this._currentIndex --];
            this._commandManager.processCommands(commands, true, true);
        }

        return false;
    };

    CommandHistory.prototype.redo = function()
    {
        let lenMinusOne = this._historyStack.length - 1;
        if (this._currentIndex < lenMinusOne)
        {
            let commands = this._historyStack[++ this._currentIndex];
            this._commandManager.processCommands(commands, false, true);
        }

        return false;
    };

    CommandHistory.prototype.clear = function()
    {
        this._historyStack = [];
        this._currentIndex = -1;
    };
}());