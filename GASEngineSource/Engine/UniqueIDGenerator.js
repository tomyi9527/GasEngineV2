//Author: beanpliu
//Date: 2020-12-21

GASEngine.UniqueIDGenerator = function()
{
    this.maxID = 300000;
    GASEngine.UniqueIDGenerator.Instance = this;
};

GASEngine.UniqueIDGenerator.prototype =
{
    constructor: GASEngine.UniqueIDGenerator,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        
    },

    updateID: function(id) {
        id = parseInt(id);
        if (id && id > 0 && !isNaN(id)) {
            this.maxID = Math.max(this.maxID, id);
        }
    },

    generateID: function() {
        this.maxID = this.maxID + 1;
        return this.maxID;
    },

    checkAndGetID: function(id) {
        id = parseInt(id);
        if (!id || id === -1 || isNaN(id)) {
            id = this.generateID();
        } else {
            this.updateID(id);
        }
        return id;
    }
};