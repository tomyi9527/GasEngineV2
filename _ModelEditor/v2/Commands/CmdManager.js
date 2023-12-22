//Author: saralu
//Date: 2019/7/9
//编辑时的命令分发
SceneEditor.CmdManager = function()
{
    this._handlers = {};

    this._handlers['d'] = new mgs.DelegateHandler(); //set, del, insert
    this._handlers['s'] = new mgs.SelectorHandler();// change

    this._cmdHistory = new SceneEditor.HistoryList();
}

SceneEditor.CmdManager.prototype.getHandler = function(handlerName)
{
    if(handlerName)
    {
        return this._handlers[handlerName];
    }
    else 
    {
        return this._handlers['d'];
    }
}

SceneEditor.CmdManager.prototype.processCmds = function(cmds, isLog = true)
{
    let count = cmds.length, cmd, handler, halfCmd;
    for(let i = 0; i < count; i++)
    {
        cmd = cmds[i];
        if(Array.isArray(cmd))
        {
            if(isLog) 
            {
                this._cmdHistory.addHistoryList(cmd);
            }
            for(let j = 0; j < cmd.length; j ++)
            {
                halfCmd = cmd[j];
                handler = this._handlers[halfCmd.h];
                handler.processCmd(halfCmd);
                if(isLog) 
                {
                    if(halfCmd.t === 'select') this._cmdHistory.addSelector(halfCmd.n);
                }
                else 
                {
                    if(halfCmd.t === 'select') this._cmdHistory.popSelector(halfCmd.n);
                }
            }
        }
        else 
        {
            if(isLog) 
            {
                this._cmdHistory.addHistoryList(cmd);
                if(cmd.t === 'select') this._cmdHistory.addSelector(cmd.n);
            }
            else 
            {
                if(cmd.t === 'select') this._cmdHistory.popSelector(cmd.n);
            }
            handler = this._handlers[cmd.h];
            handler.processCmd(cmd);
        }
    }

    // console.log(this._cmdHistory);
}

SceneEditor.CmdManager.prototype.addSelector = function(entity)
{
    this._cmdHistory.addSelector(entity);
}

SceneEditor.CmdManager.prototype.popSelector = function()
{
    this._cmdHistory.popSelector();
}

SceneEditor.CmdManager.prototype.getLastSelector = function()
{
    var entity = this._cmdHistory.getLastSelector(entity);
    return entity;
}

SceneEditor.CmdManager.prototype.undo = function()
{
    var cmd = this._cmdHistory.undo();
    if(!cmd) return;
    if(Array.isArray(cmd))
    {
        for(var i = 0; i < cmd.length; i++)
        {
            cmd[i].getInverse();
        }
        cmd.reverse();
    }
    else
    {
        cmd.getInverse();
    }
    this.processCmds([cmd], false);
}

SceneEditor.CmdManager.prototype.redo = function()
{
    var cmd = this._cmdHistory.redo();
    if(!cmd) return;
    if(Array.isArray(cmd))
    {
        for(var i = 0; i < cmd.length; i++)
        {
            cmd[i].getInverse();
        }
        cmd.reverse();
    }
    else
    {
        cmd.getInverse();
    }
    this.processCmds([cmd]);
}