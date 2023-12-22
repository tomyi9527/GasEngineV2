//

GASEngine.CameraManager = function()
{
    //this.camera = camera;

    this.tasks = [];

    GASEngine.CameraManager.Instance = this;
}

GASEngine.CameraManager.prototype =
{
    constructor: GASEngine.CameraManager,

    finl: function()
    {
        this.tasks.length = 0;
    },

    _retrieveStoredCameraStatus: function(id)
    {
        if(!this._cameraStatusForTest)
            return null;

        return this._cameraStatusForTest[id];
    },

    _storeCameraStatusForTest: function(id, eye, target, type)
    {
        if(!this._cameraStatusForTest)
        {
            this._cameraStatusForTest = {};
        }

        this._cameraStatusForTest[id] = {
            'eye': eye.clone(),
            'target': target.clone(),
            'type': type
        };
    },

    _findTask: function(camera)
    {
        for(var i = 0; i < this.tasks.length; ++i)
        {
            if(this.tasks[i].camera === camera)
            {
                return this.tasks[i];
            }
        }

        return null;
    },

    _deleteTask: function(camera)
    {
        var index = 0;
        while(index < this.tasks.length)
        {
            if(this.tasks[index].camera === camera)
            {
                this.tasks.splice(index, 1);
            }
            else
            {
                ++index;
            }
        }
    },

    update: function(delta)
    {
        'use strict';

        var index = 0;
        while(index < this.tasks.length)
        {
            var task = this.tasks[index];

            task.tick = Math.clamp(task.tick, 0.0, task.duration);
            var factor = task.tick / task.duration;

            var x = task.srcPosition.x * (1.0 - factor) + task.destPosition.x * factor;
            var y = task.srcPosition.y * (1.0 - factor) + task.destPosition.y * factor;
            var z = task.srcPosition.z * (1.0 - factor) + task.destPosition.z * factor;

            var rx = task.srcRotation.x * (1.0 - factor) + task.destRotation.x * factor;
            var ry = task.srcRotation.y * (1.0 - factor) + task.destRotation.y * factor;
            var rz = task.srcRotation.z * (1.0 - factor) + task.destRotation.z * factor;

            task.camera.position.set(x, y, z);
            task.camera.rotation.set(rx, ry, rz);

            if(task.tick === task.duration)
            {
                var finishedTask = this.tasks.splice(index, 1);
                if(finishedTask[0].callback)
                {
                    finishedTask[0].callback(finishedTask[0].destTarget);
                }
            }
            else
            {
                task.tick += delta;
                ++index;
            }            
        }
    },

    cameraGoto: function(camera, destPosition, destRotation, destTarget, duration, callback)
    {
        'use strict';

        this._deleteTask(camera);

        var task = {
                'camera': camera,
                'destPosition': destPosition,
                'destRotation': destRotation,
                'destTarget': destTarget,
                'srcPosition': camera.position.clone(),
                'srcRotation': camera.rotation.clone(),
                'duration': duration,
                'callback': callback,
                'tick': 0.0
        };

        this.tasks.push(task);
    },

    bindEvents: function(canvas_)
    {
        if(this.canvas)
        {
            this.unbindEvents(this.canvas);
        }

        this.canvas = canvas_;

        this.canvas.addEventListener('contextmenu', onContextMenu, false);
        this.canvas.addEventListener('mousedown', onMouseDown, false);
        this.canvas.addEventListener('mousewheel', onMouseWheel, false);
        this.canvas.addEventListener('MozMousePixelScroll', onMouseWheel, false); // firefox

        this.canvas.addEventListener('touchstart', onTouchStart, false);
        this.canvas.addEventListener('touchend', onTouchEnd, false);
        this.canvas.addEventListener('touchmove', onTouchMove, false);

        this.canvas.addEventListener('mousemove', this.onMouseMove, false);
        this.canvas.addEventListener('mouseup', this.onMouseUp, false);
        this.canvas.addEventListener('mouseout', this.onMouseUp, false);

        this.canvas.addEventListener('keydown', this.onKeyDown, false);
    },

    unbindEvents: function(canvas_)
    {
        if(this.canvas)
        {
            this.canvas.removeEventListener('contextmenu', this.onContextMenu, false);
            this.canvas.removeEventListener('mousedown', this.onMouseDown, false);
            this.canvas.removeEventListener('mousewheel', this.onMouseWheel, false);
            this.canvas.removeEventListener('MozMousePixelScroll', this.onMouseWheel, false); // firefox

            this.canvas.removeEventListener('touchstart', this.onTouchStart, false);
            this.canvas.removeEventListener('touchend', this.onTouchEnd, false);
            this.canvas.removeEventListener('touchmove', this.onTouchMove, false);

            this.canvas.removeEventListener('mousemove', this.onMouseMove, false);
            this.canvas.removeEventListener('mouseup', this.onMouseUp, false);
            this.canvas.removeEventListener('mouseout', this.onMouseUp, false);

            this.canvas.removeEventListener('keydown', this.onKeyDown, false);

            this.canvas = undefined;
        }
    },

    //Event handlers
    onContextMenu: function(event)
    {
        event.preventDefault();
    },

    onMouseDown: function(event)
    {

    },

    onMouseUp: function(event)
    {

    },

    onMouseMove: function(event)
    {

    },

    onMouseWheel: function(event)
    {

    },

    // Touch
    onTouchStart: function(event)
    {

    },

    onTouchEnd: function(event)
    {

    },

    onTouchMove: function(event)
    {

    },

    onKeyDown: function(event)
    {

    },
    //End Event handlers

    createCameraStatusRecord: function()
    {

    },

    updateCameraStatusRecord: function(recordID)
    {

    },

    setCameraPosture: function(recordID)
    {

    },

    getCameraPosture: function(recordID)
    {

    }
}