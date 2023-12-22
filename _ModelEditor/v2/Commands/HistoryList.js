//Author: saralu
//Date: 2019/6/25
//历史信息记录，方便redo和undo

SceneEditor.HistoryList = function()
{
    this.historyList = [];
    this.redoList = [];
    this.selectorList = [];
    // this.currSelector = null;
    SceneEditor.HistoryList.Instance = this;
}


SceneEditor.HistoryList.prototype.addSelector = function(entity)
{
    // this.currSelector = entity.uniqueID;
    if(!entity) return;
    this.selectorList.push(entity);
}

SceneEditor.HistoryList.prototype.popSelector = function()
{
    this.selectorList.pop();
}

SceneEditor.HistoryList.prototype.getLastSelector = function()
{
    var len = this.selectorList.length;
    if(len <= 0) return null;
    var entity = this.selectorList[len - 1];
    return entity;
}

SceneEditor.HistoryList.prototype.addHistoryList = function(cmd)
{
    this.historyList.push(cmd);
}

SceneEditor.HistoryList.prototype.undo = function()
{
    if(this.historyList.length === 0) return false;
    var historyItem = this.historyList.pop();
    this.redoList.push(historyItem);
    return historyItem;
}

SceneEditor.HistoryList.prototype.redo = function()
{
    // console.log('redo');
    if(this.redoList.length === 0) return false;
    var historyItem = this.redoList.pop();
    this.historyList.push(historyItem);
    return historyItem;
}

SceneEditor.HistoryList.prototype.clearHistory = function()
{
    // console.log('clearHistory');
    this.historyList = [];
    this.redoList = [];
    this.selectorList = [];
}



