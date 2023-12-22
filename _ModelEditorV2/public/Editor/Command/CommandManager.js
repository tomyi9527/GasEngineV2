(function ()
{
    // command设计基于原则1：editor只有两种可操作对象entity和asset（场景也对应为asset）
    // command设计基于原则2：entity和asset都具备init/clear/add/delete/move/select/modify等操作
    // command设计基于原则3：entity和asset都必须选中后，才能进行delete/move/modify操作
    // command设计基于原则4：entity和asset都具有集合对象，只有集合对象才能进行init/clear/add/delete/move/select，entity和asset对象本身只能进行modify操作
    // command设计基于原则5：集合对象进行init/clear操作时，不作历史记录，且需要清空历史记录栈
    // command设计基于原则6：集合对象进行add操作时，需先产生entity/asset对象，记录入对象池内，再执行add操作。
    // command设计基于原则7：集合对象进行delete操作时，数据层将entity/asset对象移出集合树结构，但是记录入对象池内，以防undo操作需要使用。
    // command设计基于原则8：操作会触发所有观察者进行对应数据和视图的修改，数据观察者和视图观察者地位一样，都是观察命令，并进行修改，因为操作命令原则上就应该包含了要修改的内容，不需要视图层去数据层获取（这也是不合理的，这样会使得视图层和数据层直接耦合），这里数据指entity/asset对象。
    
    // command设计基于原则9：
    // 原则8的补充，因为视图层在遇到修改命令时，可以通过命令获取所有需要的信息，但是视图层在初始化的时候，信息量可能太大，如果全部拷贝到命令中是不合适的（命令只适合传输轻量指令，否则会造成严重性能甚至网络带宽问题，例如删除一个场景的根节点，如果拷贝根节点整个架构，那将会是的命令数据极大），但是又为了防止视图层直接和数据层关联造成耦合，所以引入数据对象的代理对象，例如entity/asset对象都各对应一个代理，代理只暴露视图层关心的接口，代理的实现，使用具体数据对象来为接口提供数据，这样就使得视图层可以很自由地获取自身关心的视图数据，同时又不会造成视图层和数据层的耦合（耦合的坏处是：以后如果引擎对象做了修改，而这种修改的可能性是很大的，大量视图层逻辑都得修改，使用代理只需要修改代理接口的实现即可）。因为视图层需要通过代理获取数据，所以，原则上，需要数据观察者第一个响应cmd命令，否则需要获取数据的视图将无法获取最新数据
    
    // command设计基于原则10：
    // 视图层除了关心代理对象，还可能关心对象的集合，例如场景树、资源树，它们分别对应entity对象集合和asset对象集合，所以，需要为这类视图提供代理集合对象，提供接口对代理集合进行集合相关的操作
    
    // command设计基于原则11：
    // 为了使得操作命令也和数据层不耦合，提供代理管理对象（管理代理对象和代理集合）进行操作命令的数据修改（同原则8中所说类似，操作命令的执行只是和编辑逻辑强相关的逻辑，和底层引擎数据并不应该直接关联，否则引擎修改也需要造成大量修改，使用代理则只需要修改代理接口的实现即可）

    // command设计基于原则11：
    // 选取操作可以选取entity集合或者entity集合，但是一次只能选取某类集合中的对象，因为不同集合的对象在属性面板的展示是不一致的，所以不能混选。所以，选取逻辑不应该在代理集合内处理，而是应该有专门的选取管理器来统一处理。注意：在场景树面板Controller逻辑中，如果遇到选取的不是Entity，则需要清空面板选取状态。在资源树的面板Controller逻辑中，如果遇到选取的不是Asset，则需要清空面板选取状态。



    // command设计思路1：每条命令内含子命令集，使得每条命令可对多个对象进行操作（删除操作包含：取消选择 + 删除，添加操作包含：添加 + 选择）
    // command设计思路2：每条子命令必须含命令本身和其反向命令，注意：add、resume逆向命令都是delete，delete逆向命令与否主动触发有关，(主动触发的delete逆向命令对应resume)
    // command设计思路3：每条子命令含一个目标对象，通过字段标识目标对象类型，通过ID标识具体对象(id可能多个，例如entity包含场景ID和entity ID)
    // command设计思路4：每条子命令对目标的操作包含 初始、清空、创建、删除、移动、选择、修改
    // command设计思路5：目标的“修改”操作，包含路径名 + 修改信息(修改信息与该路径名对应的具体对象修改方式有关)
    // command设计思路6：每个目标对象都对应一个Delegate，Delegate根据命令中的路径找到该目标对象应该修改的“属性视图”和“对象数据”，Delegate包含DelegateObject和DelegateMgr
                        // DelegateObject用于“修改”，DelegateMgr用于初始、清空、创建、删除、移动、选择
                        // 每一类DelegateObject对应一个DelegateMgr，例如3D编辑场景对应一个DelegateScene（继承自DelegateMgr），资源管理器对应DelegateAssetMgr（继承自DelegateMgr）
                        // DelegateObject及对应的目标对象Object的初始、清空、创建、删除、移动、选择都是通过其对应的DelegateMgr来操作的
                        // Delegate是内存对象数值管理、数据显示统一描述的中介，方便通过统一的命令进行界面的展示和数据的修改

                        // 例如：entity位移的修改，会根据DelegateEntity找到属性面板修改位移属性视图，通过DelegateEntity修改entity数据，属性视图初始时还会需要从Delegate中获取数据
                        // 例如：scene初始、清空、创建、删除、移动、选择Entity，会根据DelegateScene做对应Entity的操作
                        // 例如：asset内容的修改，会根据DelegateAsset找到属性面板修改位移属性视图，通过DelegateAsset修改asset数据
    // command设计思路7：Delegate引用和映射创建，某个Delegate对应的Object可能通过ID引用另一个Delegate的Object，且两个Delegate对应的DelegateMgr集可能不一样，需要通过统一的机制进行映射，
                        // 通过DelegateMapMgr进行管理，这样能够高效处理Delegate关联、销毁、修改等映射问题

    /**
     * command格式：
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          objectID, 0,
     *          type: 'init'/'clear'/'add'/delete'/'resume'/'copy'/move'/'select'/'modify',
     *          param: {},
     *      },
     *      reverse:
     *      {
     *          managerID: 0,
     *          objectID, 0,
     *          type: 'init'/'clear'/'add'/delete'/'resume'/'copy'/move'/'select'/'modify',
     *          param: {},
     *      }
     * }
     * 
     * init:
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          type: 'init',
     *      },
     * }
     * 
     * clear:
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          type: 'clear',
     *      },
     * }
     * 
     * modify:
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          objectID, 0,
     *          type: 'modify',
     *          param: 
     *          {
     *              value: value,
     *              fullPath: fullPath,
     *          },
     *      },
     *      reverse:
     *      {
     *          managerID: 0,
     *          objectID, 0,
     *          type: 'modify',
     *          param: 
     *          {
     *              value: value,
     *              fullPath: fullPath,
     *          },
     *      }
     * }
     * 
     * select:
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          type: 'select',
     *          param: 
     *          {
     *              selectIDs: selectIDs,
     *          },
     *      },
     *      reverse:
     *      {
     *          managerID: 0,
     *          type: 'select',
     *          param: 
     *          {
     *              selectIDs: selectIDs,
     *          },
     *      }
     * }
     * 
     * move:
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          type: 'move',
     *          param: 
     *          {
     *              idInfos: [{fromID:0, ToID: 1, refID: 2}, {fromID:0, ToID: 1, refID: 2}],
     *          },
     *      },
     *      reverse:
     *      {
     *          managerID: 0,
     *          type: 'move',
     *          param: 
     *          {
     *              idInfos: [{fromID:0, ToID: 1, refID: 2}, {fromID:0, ToID: 1, refID: 2}],
     *          },
     *      }
     * }
     * 
     * add:
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          type: 'add',
     *          param:
     *          {
     *              idInfos: [{objectID:0, parentID: 1, refID: 2}, {objectID:3, parentID: 4, refID: 5}],
     *          },
     *      },
     *      reverse:
     *      {
     *          managerID: 0,
     *          type: 'delete',
     *          param: 
     *          {
     *              idInfos: [{objectID:0, parentID: 1}, {objectID:3, parentID: 4}],
     *          },
     *      }
     * }
     * 
     * delete:
     * {
     *      cmd:
     *      {
     *          managerID: 0,
     *          type: 'delete',
     *          param: 
     *          {
     *              idInfos: [{objectID:0, parentID: 1}, {objectID:3, parentID: 4}],
     *          },
     *      }
     *      reverse:
     *      {
     *          managerID: 0,
     *          type: 'add',
     *          param:
     *          {
     *              idInfos: [{objectID:0, parentID: 1, refID: 2}, {objectID:3, parentID: 4, refID: 5}],
     *          },
     *      },
     * }
     * 
     */
        

    let CommandManager = function () 
    {
        mgs.Events.call(this);

        this._commandHistory = new mgs.CommandHistory(this);
    };
    mgs.classInherit(CommandManager, mgs.Events);

    CommandManager.prototype.getInverseCmd = function(command)
    {
        let inverseCommand = 
        {
            cmd: command.reverse,
            reverse: command.cmd,
        };

        return inverseCommand;
    };

    CommandManager.prototype.processCommands = function(commands, isReverse, ignoreHistory, clearHistory)
    {
        let count = commands.length;

        let i = isReverse ? count -1 : 0;
        for (; isReverse ? i >= 0 : i < count; isReverse ? i-- : i ++)
        {
            let command = isReverse ? this.getInverseCmd(commands[i]) : commands[i];
            this.emit('onCommand', command.cmd);
        }

        if (!ignoreHistory && !clearHistory)
        {
            this._commandHistory.add(commands);
        }

        if (clearHistory)
        {
            this._commandHistory.clear();
        }
    };

    CommandManager.prototype.getCommandHistory = function()
    {
        return this._commandHistory;
    };
}());