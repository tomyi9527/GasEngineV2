(function()
{
    let Util = 
    {
        simpleCopy : function(source) 
        {
            let target = {};
            for (let key in source)
            {
                target[key] = source[key];
            }
            
            return target;
        },

        isObject : function(o)
        {
            return o !== null && typeof o === 'object' && Array.isArray(o) === false;
        },
        
        isArray : function(o)
        {
            return Array.isArray(o);
        },

        isFunction : function(o)
        {
            return typeof o === 'function';
        },

        deepCopy : function(source)
        {
            if (source === null)
            {
                return null;
            }

            if (this.isObject(source) || this.isArray(source))
            {
                return JSON.parse(JSON.stringify(source));
            }
            else
            {
                return source;
            } 
        },

        // deepCopy : function(data) 
        // {
        //     if (data == null || typeof(data) !== 'object')
        //         return data;
        
        //     if (data instanceof Array) 
        //     {
        //         var arr = [ ];
        //         for(var i = 0; i < data.length; i++) 
        //         {
        //             arr[i] = deepCopy(data[i]);
        //         }
        //         return arr;
        //     } 
        //     else 
        //     {
        //         var obj = { };
        //         for(var key in data) 
        //         {
        //             if (data.hasOwnProperty(key))
        //                 obj[key] = deepCopy(data[key]);
        //         }
        //         return obj;
        //     }
        // },


        //////////////////////////// string & buffer ////////////////////////////
        Uint8ArrayToUtf8String:function(array) 
        {
            var out, i, len, c;
            var char2, char3;
            
            out = "";
            len = array.length;
            i = 0;
            while(i < len) 
            {
                c = array[i++];
                switch(c >> 4)
                { 
                    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                    // 0xxxxxxx
                    out += String.fromCharCode(c);
                    break;

                    case 12: case 13:
                    // 110x xxxx 10xx xxxx
                    char2 = array[i++];
                    out += String.fromCharCode(((c & 0x1F) << 6) | (char2 & 0x3F));
                    break;

                    case 14:
                    // 1110 xxxx 10xx xxxx 10xx xxxx
                    char2 = array[i++];
                    char3 = array[i++];
                    out += String.fromCharCode(((c & 0x0F) << 12) |
                    ((char2 & 0x3F) << 6) |
                    ((char3 & 0x3F) << 0));
                    break;
                }
            }
            
            return out;
        },

        Utf8StringToUint8Array:function(str)
        {
            var utf8 = [];
            for (var i=0; i < str.length; i++) 
            {
                var charcode = str.charCodeAt(i);
                if (charcode < 0x80) utf8.push(charcode);
                else if (charcode < 0x800) {
                    utf8.push(0xc0 | (charcode >> 6), 
                              0x80 | (charcode & 0x3f));
                }
                else if (charcode < 0xd800 || charcode >= 0xe000) 
                {
                    utf8.push(0xe0 | (charcode >> 12), 
                              0x80 | ((charcode>>6) & 0x3f), 
                              0x80 | (charcode & 0x3f));
                }
                // surrogate pair
                else 
                {
                    i++;
                    // UTF-16 encodes 0x10000-0x10FFFF by
                    // subtracting 0x10000 and splitting the
                    // 20 bits of 0x0-0xFFFFF into two halves
                    charcode = 0x10000 + (((charcode & 0x3ff)<<10)
                              | (str.charCodeAt(i) & 0x3ff));
                    utf8.push(0xf0 | (charcode >>18), 
                              0x80 | ((charcode>>12) & 0x3f), 
                              0x80 | ((charcode>>6) & 0x3f), 
                              0x80 | (charcode & 0x3f));
                }
            }

            return new Uint8Array(utf8);
        },

        arrayBuffer2String : function(buf) 
        {
            return this.Uint8ArrayToUtf8String(new Uint8Array(buf)); 
        },
    
        string2ArrayBuffer : function(str)
        {
            let uint8Array = this.Utf8StringToUint8Array(str);
            return uint8Array.buffer; 
        },

        arrayBufferCopy : function(src)  
        {
            var dst = new ArrayBuffer(src.byteLength);
            new Uint8Array(dst).set(new Uint8Array(src));
            return dst;
        },

        
        /////////////////// array ///////////////////
        arrayRemove : function(arr, element)
        {
            var index = arr.indexOf(element);
            if (index > -1) 
            {
                arr.splice(index, 1);
            }
        },

        arrayInsert : function(arr, index, element)
        {
            arr.splice(index, 0, element);
        },

        arrayExist : function(arr, element)
        {
            return arr.indexOf(element) !== -1;
        },

        arrayElementIndex : function(arr, element)
        {
            return arr.indexOf(element);
        },
        
        arrayCopy : function(arrFrom, arrTo)
        {
            arrTo.length = 0;
            var length = arrFrom.length;
            for (var i = 0;i < length;i ++)
            {
                arrTo.push(arrFrom[i]);
            }
        },

        /////////////////// string ///////////////////
        isSubString : function(subStr, superStr)
        {
            return (superStr.indexOf(subStr) !== -1);
        },

        definePathString : function(constTable, pathString)
        {
            let spector = '.';
            let pathes = pathString.split(spector);
            let parent = constTable;
            let currentPathString = '';
    
            for (let i = 0;i < pathes.length;i ++)
            {
                let path = pathes[i];
                let child = parent[path];
    
                let midChar = (currentPathString === '') ? '' : spector;
                currentPathString += midChar;
                currentPathString += path;
    
                if (i == pathes.length - 1)
                {
                    if (child)
                    {
                        mgs.Log.error('constTable namespace props name:' + '"' + currentPathString + '"' + ' is eixsted, try set other name!');
                    }
                    else
                    {
                        parent[path] = pathString;
                    }
                }
                else
                {
                    if (!child)
                    {
                        child = {};
                        parent[path] = child;
                    }
                }
    
                parent = child;
            }
        },

        /////////////////// system ///////////////////
        printMemory: function()
        {
            var format = function(bytes) 
            { 
                return (bytes/1024/1024).toFixed(2)+'MB'; 
            };
            var mem = process.memoryUsage();
            console.log('Process: heapTotal '+format(mem.heapTotal) + ' heapUsed ' + format(mem.heapUsed) + ' rss ' + format(mem.rss)); 
        },
    };

    // let str = 'jldk的好sjflds是独立开发建设的方式独立909()_（））';
    // let buf = Util.String2ArrayBuffer(str);
    // console.log(Util.ArrayBuffer2String(buf));


    /////////////////// element event utils ///////////////////
    // var elementListener = {};

    // var originalEventListener = window.addEventListener;
    // window.addEventListener = function(type, fn, options) 
    // {
    //     if (!listeners[type])
    //         listeners[type] = [];

    //     listeners[type].push(fn);
    //     return originalEventListener(type, fn, options);
    // }

    // var removeAllEventListener = function(type) 
    // {
    //     if (!listeners[type] || !listeners[type].length)
    //         return;

    //     for (let i = 0; i < listeners[type].length; i++)
    //         window.removeEventListener(type, listeners[type][i]);
    // }

    mgs.assign('Util', Util);
}());

