(function()
{
    let Log = 
    {
        write: function (text) 
        {
            console.log(text);
        },

        open: function () 
        {
            this.write("Powered by MiniGameStudio " + pc.version + " " + pc.revision);
        },

        info: function (text) 
        {
            console.info("INFO:    " + text);
        },

        error: function (text) 
        {
            console.trace();
            console.error("ERROR:   " + text);
        },

        alert: function (condition) 
        {
            if (!condition)
            {
                this.error("alert triggered!" );
            }
        },

        warning: function (text) {
            console.trace();
            console.warn("WARNING: " + text);
        },
    };

    mgs.assign('Log', Log);
}());

