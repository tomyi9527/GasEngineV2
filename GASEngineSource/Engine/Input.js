
//Keyboard
GASEngine.KeyboardEvent = function(keyboard, event)
{
    if(event)
    {
        this.key = event.keyCode;
        this.element = event.target;
        this.event = event;
    }
    else
    {
        this.key = null;
        this.element = null;
        this.event = null;
    }
};

GASEngine.Keyboard = function(element, options) 
{
    GASEngine.Events.attach(this);

    options = options || {};
    this._element = null;

    this._keyDownHandler = this._handleKeyDown.bind(this);
    this._keyUpHandler = this._handleKeyUp.bind(this);
    this._keyPressHandler = this._handleKeyPress.bind(this);

    this._keymap = {};
    this._lastmap = {};

    if(element)
    {
        this.attach(element);
    }

    this.preventDefault = options.preventDefault || false;
    this.stopPropagation = options.stopPropagation || false;

    GASEngine.Keyboard.Instance = this;
};

GASEngine.Keyboard.MSG_ON_KEY_UP = 'MSG_ON_KEY_UP';
GASEngine.Keyboard.MSG_ON_KEY_DOWN = 'MSG_ON_KEY_DOWN';
GASEngine.Keyboard.MSG_ON_KEY_PRESS = 'MSG_ON_KEY_PRESS';

GASEngine.Keyboard._keyCodeToKeyIdentifier =
{
    '9': 'Tab',
    '13': 'Enter',
    '16': 'Shift',
    '17': 'Control',
    '18': 'Alt',
    '27': 'Escape',
    '37': 'Left',
    '38': 'Up',
    '39': 'Right',
    '40': 'Down',
    '46': 'Delete',
    '91': 'Win'
};

GASEngine.Keyboard.prototype =
{
    constructor: GASEngine.Keyboard,
    finl: function()
    {
        this.detach();
    }
};

GASEngine.Keyboard.prototype.toKeyCode = function(s)
{
    if(typeof(s) === "string")
    {
        return s.toUpperCase().charCodeAt(0);
    }
    else
    {
        return s;
    }
};

GASEngine.Keyboard.prototype.makeKeyboardEvent = (function()
{
    var _keyboardEvent = new GASEngine.KeyboardEvent();

    return function(event)
    {
        _keyboardEvent.key = event.keyCode;
        _keyboardEvent.element = event.target;
        _keyboardEvent.event = event;
        return _keyboardEvent;
    };
})();

GASEngine.Keyboard.prototype.attach = function (element) 
{
    if(this._element) 
    {
        this.detach();
    }

    this._element = element;
    this._element.addEventListener("keydown", this._keyDownHandler, false);
    this._element.addEventListener("keypress", this._keyPressHandler, false);
    this._element.addEventListener("keyup", this._keyUpHandler, false);
};

GASEngine.Keyboard.prototype.detach = function()
{
    this._element.removeEventListener("keydown", this._keyDownHandler);
    this._element.removeEventListener("keypress", this._keyPressHandler);
    this._element.removeEventListener("keyup", this._keyUpHandler);
    this._element = null;
};

GASEngine.Keyboard.prototype.toKeyIdentifier = function(keyCode)
{
    keyCode = this.toKeyCode(keyCode);
    var count;
    var hex;
    var length;
    var id = GASEngine.Keyboard._keyCodeToKeyIdentifier[keyCode.toString()];

    if (id) 
    {
        return id;
    }

    // Convert to hex and add leading 0's
    hex = keyCode.toString(16).toUpperCase();
    length = hex.length;
    for (count = 0; count < (4 - length); count++)
    {
        hex = '0' + hex;
    }

    return 'U+' + hex;
};

GASEngine.Keyboard.prototype._handleKeyDown = function(event)
{
    var code = event.keyCode || event.charCode;
    var id = this.toKeyIdentifier(code);

    this._keymap[id] = true;

    var innerEvent = this.makeKeyboardEvent(event);
    this.emit(GASEngine.Keyboard.MSG_ON_KEY_DOWN, innerEvent);

    if(this.preventDefault)
    {
        event.preventDefault();
    }

    if(this.stopPropagation)
    {
        event.stopPropagation();
    }
};

GASEngine.Keyboard.prototype._handleKeyUp = function(event)
{
    var code = event.keyCode || event.charCode;
    var id = this.toKeyIdentifier(code);
    event.keyIdentifier = event.keyIdentifier || id;

    delete this._keymap[id];

    var innerEvent = this.makeKeyboardEvent(event);
    this.emit(GASEngine.Keyboard.MSG_ON_KEY_UP, innerEvent);

    if(this.preventDefault)
    {
        event.preventDefault();
    }

    if(this.stopPropagation)
    {
        event.stopPropagation();
    }
};

GASEngine.Keyboard.prototype._handleKeyPress = function(event)
{
    var code = event.keyCode || event.charCode;
    var id = this.toKeyIdentifier(code);
    event.keyIdentifier = event.keyIdentifier || id;

    var innerEvent = this.makeKeyboardEvent(event);
    this.emit(GASEngine.Keyboard.MSG_ON_KEY_PRESS, innerEvent);

    if (this.preventDefault)
    {
        event.preventDefault();
    }

    if (this.stopPropagation)
    {
        event.stopPropagation();
    }
};

GASEngine.Keyboard.prototype.update = function(delta)
{
    var prop;

    // clear all keys
    for(prop in this._lastmap)
    {
        delete this._lastmap[prop];
    }

    for(prop in this._keymap)
    {
        if(this._keymap.hasOwnProperty(prop))
        {
            this._lastmap[prop] = this._keymap[prop];
        }
    }
};

GASEngine.Keyboard.prototype.GetKey = function(key)
{
    var keyCode = this.toKeyCode(key);
    var id = this.toKeyIdentifier(keyCode);

    return !!(this._keymap[id]);
};

GASEngine.Keyboard.prototype.GetKeyDown = function(key)
{
    var keyCode = this.toKeyCode(key);
    var id = this.toKeyIdentifier(keyCode);

    return (!!(this._keymap[id]) && !!!(this._lastmap[id]));
};

GASEngine.Keyboard.prototype.GetKeyUp = function(key)
{
    var keyCode = this.toKeyCode(key);
    var id = this.toKeyIdentifier(keyCode);

    return (!!!(this._keymap[id]) && !!(this._lastmap[id]));
};


//Mouse Input
GASEngine.MOUSEBUTTON_NONE = -1;
GASEngine.MOUSEBUTTON_LEFT = 0;
GASEngine.MOUSEBUTTON_MIDDLE = 1;
GASEngine.MOUSEBUTTON_RIGHT = 2;
GASEngine.MOUSEBUTTON_BACK = 3;
GASEngine.MOUSEBUTTON_FORWARD = 4;

GASEngine.MouseEvent = function()
{
    this.x = 0;
    this.y = 0;
    this.wheel = 0;
    this.dx = 0;
    this.dy = 0;
    this.button = GASEngine.MOUSEBUTTON_NONE;

    this.element = null;
    this.ctrlKey = false;
    this.altKey = false;
    this.shiftKey = false;
    this.metaKey = false;
};

GASEngine.MouseEvent.prototype =
{
    constructor: GASEngine.MouseEvent,

    computeMouseParameters: function(mouse, event)
    {
        this.x = mouse._currentX;
        this.y = mouse._currentY;

        //if(GASEngine.Mouse.isPointerLocked())
        //{
        //    this.x = 0;
        //    this.y = 0;
        //}
        
        // FF uses 'detail' and returns a value in 'no. of lines' to scroll
        // WebKit and Opera use 'wheelDelta', WebKit goes in multiples of 120 per wheel notch
        if(event.detail)
        {
            this.wheel = -1 * event.detail;
        }
        else if(event.wheelDelta)
        {
            this.wheel = event.wheelDelta / 120;
        }
        else
        {
            this.wheel = 0;
        }

        if(GASEngine.Mouse.isPointerLocked())
        {
            this.dx = event.movementX || event.webkitMovementX || event.mozMovementX || 0;
            this.dy = event.movementY || event.webkitMovementY || event.mozMovementY || 0;
        }
        else
        {
            this.dx = this.x - mouse._lastX;
            this.dy = this.y - mouse._lastY;
        }

        if(event.type === 'mousedown' || event.type === 'mouseup')
        {
            this.button = event.button;
        }
        else
        {
            this.button = GASEngine.MOUSEBUTTON_NONE;
        }

        this.element = event.target;
        this.ctrlKey = event.ctrlKey || false;
        this.altKey = event.altKey || false;
        this.shiftKey = event.shiftKey || false;
        this.metaKey = event.metaKey || false;

        this.event = event;
    }
};

GASEngine.Mouse = function(element)
{
    GASEngine.Events.attach(this);

    this._currentX = 0;
    this._currentY = 0;

    this._lastX      = 0;
    this._lastY      = 0;
    this._buttons      = [false,false,false];
    this._lastbuttons  = [false, false, false];

    this._upHandler = this._handleUp.bind(this);
    this._downHandler = this._handleDown.bind(this);
    this._moveHandler = this._handleMove.bind(this);
    this._wheelHandler = this._handleWheel.bind(this);
    this._contextMenuHandler = function(event){ event.preventDefault(); };

    this._target = null;
    this._attached = false;

    this._innerEvent = new GASEngine.MouseEvent();

    this.attach(element);

    GASEngine.Mouse.Instance = this;
};

GASEngine.Mouse.isPointerLocked = function()
{
    return !!(document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement);
};

GASEngine.Mouse.MSG_ON_MOUSE_UP = 'MSG_ON_MOUSE_UP';
GASEngine.Mouse.MSG_ON_MOUSE_DOWN = 'MSG_ON_MOUSE_DOWN';
GASEngine.Mouse.MSG_ON_MOUSE_MOVE = 'MSG_ON_MOUSE_MOVE';
GASEngine.Mouse.MSG_ON_MOUSE_WHEEL = 'MSG_ON_MOUSE_WHEEL';

GASEngine.Mouse.prototype =
{
    constructor: GASEngine.Mouse,

    finl: function()
    {
        this.detach();
    },

    attach: function(element)
    {
        this._target = element;

        if(this._attached)
        {
            return;
        }

        this._attached = true;

        window.addEventListener("mouseup", this._upHandler, false);
        window.addEventListener("mousedown", this._downHandler, false);
        window.addEventListener("mousemove", this._moveHandler, false);
        window.addEventListener("mousewheel", this._wheelHandler, false); // WekKit
        window.addEventListener("DOMMouseScroll", this._wheelHandler, false); // Gecko
    },

    detach: function()
    {
        if (!this._attached)
            return;

        this._attached = false;

        window.removeEventListener("mouseup", this._upHandler);
        window.removeEventListener("mousedown", this._downHandler);
        window.removeEventListener("mousemove", this._moveHandler);
        window.removeEventListener("mousewheel", this._wheelHandler); // WekKit
        window.removeEventListener("DOMMouseScroll", this._wheelHandler); // Gecko
    },

    disableContextMenu: function()
    {
        if (!this._target) 
            return;

        this._target.addEventListener("contextmenu", this._contextMenuHandler);
    },

    enableContextMenu: function()
    {
        if (!this._target)
            return;

        this._target.removeEventListener("contextmenu", this._contextMenuHandler);
    },

    enablePointerLock: function(success_callback, error_callback)
    {
        if(!document.body.requestPointerLock)
        {
            if(error)
            {
                error();
            }

            return;
        }

        var s = function()
        {
            success_callback();
            document.removeEventListener('pointerlockchange', s);
        };

        var e = function()
        {
            error_callback();
            document.removeEventListener('pointerlockerror', e);
        };

        if(success_callback)
        {
            document.addEventListener('pointerlockchange', s, false);
        }

        if(error_callback)
        {
            document.addEventListener('pointerlockerror', e, false);
        }

        document.body.requestPointerLock();
    },

    disablePointerLock: function(success_callback)
    {
        if(!document.exitPointerLock)
        {
            return;
        }

        var s = function()
        {
            success_callback();
            document.removeEventListener('pointerlockchange', s);
        };

        if(success_callback)
        {
            document.addEventListener('pointerlockchange', s, false);
        }

        document.exitPointerLock();
    },

    update: function(delta)
    {
        this._lastbuttons[0] = this._buttons[0];
        this._lastbuttons[1] = this._buttons[1];
        this._lastbuttons[2] = this._buttons[2];
    },

    getMouseButton: function(button)
    {
        return this._buttons[button];
    },

    getMouseButtonDown: function(button)
    {
        return (this._buttons[button] && !this._lastbuttons[button]);
    },

    getMouseButtonUp: function(button)
    {
        return (!this._buttons[button] && this._lastbuttons[button]);
    },

    _handleUp: function(event)
    {
        this._buttons[event.button] = false;

        this._updateTargetCoords(event);
        this._innerEvent.computeMouseParameters(this, event);

        this.emit(GASEngine.Mouse.MSG_ON_MOUSE_UP, this._innerEvent);
    },

    _handleDown: function(event)
    {
        this._buttons[event.button] = true;

        this._updateTargetCoords(event);
        this._innerEvent.computeMouseParameters(this, event);

        this.emit(GASEngine.Mouse.MSG_ON_MOUSE_DOWN, this._innerEvent);
    },

    _handleMove: function(event)
    {
        // When Pointer lock is enabled, the standard MouseEvent properties clientX, clientY, screenX, 
        // and screenY are held constant, as if the mouse is not moving.
        this._updateTargetCoords(event);
        this._innerEvent.computeMouseParameters(this, event);

        this.emit(GASEngine.Mouse.MSG_ON_MOUSE_MOVE, this._innerEvent);

        this._lastX = this._currentX;
        this._lastY = this._currentY;

        //console.log(this._innerEvent.dx + '-:-' + this._innerEvent.dy);
        //console.log(event.movementX + ' : ' + event.movementY);
    },

    _handleWheel: function (event)
    {
        this._updateTargetCoords(event);
        this._innerEvent.computeMouseParameters(this, event);

        this.emit(GASEngine.Mouse.MSG_ON_MOUSE_WHEEL, this._innerEvent);
    },

    _updateTargetCoords: function(event)
    {
        var rect = this._target.getBoundingClientRect();
        var left = Math.floor(rect.left);
        var top = Math.floor(rect.top);

        // mouse is outside of canvas
        if (event.clientX < left ||
            event.clientX >= left + this._target.clientWidth ||
            event.clientY < top ||
            event.clientY >= top + this._target.clientHeight)
        {
           
        }
        else
        {
            this._currentX = event.clientX - left;
            this._currentY = event.clientY - top;
        }
    }
};

// For compatibility
(function()
{
    if(typeof navigator === 'undefined' || typeof document === 'undefined')
    {
        return;
    }

    navigator.pointer = navigator.pointer || navigator.webkitPointer || navigator.mozPointer;

    var pointerlockchange = function ()
    {
        var e = document.createEvent('CustomEvent');
        e.initCustomEvent('pointerlockchange', true, false, null);
        document.dispatchEvent(e);
    };

    var pointerlockerror = function ()
    {
        var e = document.createEvent('CustomEvent');
        e.initCustomEvent('pointerlockerror', true, false, null);
        document.dispatchEvent(e);
    };

    document.addEventListener('webkitpointerlockchange', pointerlockchange, false);
    document.addEventListener('webkitpointerlocklost', pointerlockchange, false);
    document.addEventListener('mozpointerlockchange', pointerlockchange, false);
    document.addEventListener('mozpointerlocklost', pointerlockchange, false);

    document.addEventListener('webkitpointerlockerror', pointerlockerror, false);
    document.addEventListener('mozpointerlockerror', pointerlockerror, false);

    if(Element.prototype.mozRequestPointerLock)
    {
        // FF requires a new function for some reason
        Element.prototype.requestPointerLock = function ()
        {
            this.mozRequestPointerLock();
        };
    }
    else
    {
        Element.prototype.requestPointerLock =
            Element.prototype.requestPointerLock ||
            Element.prototype.webkitRequestPointerLock ||
            Element.prototype.mozRequestPointerLock;
    }

    if(!Element.prototype.requestPointerLock && navigator.pointer)
    {
        Element.prototype.requestPointerLock = function()
        {
            document.pointerLockElement = this;
            navigator.pointer.lock(this, pointerlockchange, pointerlockerror);
        };
    }

    document.exitPointerLock = document.exitPointerLock || document.webkitExitPointerLock || document.mozExitPointerLock;
    if(!document.exitPointerLock)
    {
        document.exitPointerLock = function()
        {
            if(navigator.pointer)
            {
                document.pointerLockElement = null;
                navigator.pointer.unlock();
            }
        };
    }
})();