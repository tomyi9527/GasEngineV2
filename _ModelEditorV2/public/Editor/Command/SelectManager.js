(function ()
{
    let SelectManager = function () 
    {
        mgs.Object.call(this);

        this._selectManagerID = null;
        this._selectObjectIDs = [];

        let self = this;
        this._cmdHander = function(cmd)
        {
            let managerID = cmd.managerID;

            switch(cmd.type)
            {
                case 'select':
                {
                    let selectIDs = cmd.param.selectIDs;
                    self._selectManagerID = managerID;
                    mgs.Util.arrayCopy(selectIDs, self._selectObjectIDs);
                }
                break;
            }
        };
        mgs.editor.commandManager.on('onCommand', this._cmdHander);
    };
    mgs.classInherit(SelectManager, mgs.Object);

    SelectManager.prototype.onDestroy = function()
    {
        mgs.editor.commandManager.unbind('onCommand', this._cmdHander);
    };

    SelectManager.prototype.generateSelectCommand = function(managerID, selectIDs)
    {
        let tempSelectIDs = [];
        mgs.Util.arrayCopy(selectIDs, tempSelectIDs);

        let tempPreSelectIDs = [];
        mgs.Util.arrayCopy(this._selectObjectIDs, tempPreSelectIDs);

        let command =
        {
            cmd:
            {
                type: 'select',
                managerID: managerID,
                param:
                {
                    selectIDs: tempSelectIDs,
                },
            },
            reverse:
            {
                type: 'select',
                managerID: this._selectManagerID,
                param:
                {
                    selectIDs: tempPreSelectIDs,
                },
            },
        };
        return command;
    };

    SelectManager.prototype.sendSelectCommand = function(managerID, selectIDs)
    {
        let command = this.generateSelectCommand(managerID, selectIDs);
        mgs.editor.commandManager.processCommands([command]);
    };
}());