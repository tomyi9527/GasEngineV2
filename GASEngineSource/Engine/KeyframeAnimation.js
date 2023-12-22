GASEngine.KeyframeAnimation = function()
{  
    this._clipID = -1;
    this._clipName = '';
    
    this._enableClamp = false;
    this.animatedNodes = [];
    this.timeMultiplier = 1.0;
    this._clipLocalTime = 0.0;
    this.updateEnable = true;
    this.version = undefined;

    this.startFrame = 0.0;
    this.endFrame = 0.0;
    this.fps = 0.0;    

    this.clampedStartFrame = this.startFrame;
    this.clampedEndFrame = this.endFrame;

    this.onAnimationUpdateCallback = ()=>{};

    this.onAnimationStartCallback = ()=>{};
    this.onAnimationEndCallback = ()=>{};

    this.forceUpdate = false;
    this._clipStarted = false;
    this._clipEnded = false;

    this.guid = '';

    this._originValueDB_ = new Map();
};

GASEngine.KeyframeAnimation.TARGET_TX = 0;
GASEngine.KeyframeAnimation.TARGET_TY = 1;
GASEngine.KeyframeAnimation.TARGET_TZ = 2;
GASEngine.KeyframeAnimation.TARGET_RX = 3;
GASEngine.KeyframeAnimation.TARGET_RY = 4;
GASEngine.KeyframeAnimation.TARGET_RZ = 5;
GASEngine.KeyframeAnimation.TARGET_SX = 6;
GASEngine.KeyframeAnimation.TARGET_SY = 7;
GASEngine.KeyframeAnimation.TARGET_SZ = 8;
GASEngine.KeyframeAnimation.TARGET_MORPH_WEIGHT = 9;
GASEngine.KeyframeAnimation.TARGET_VISIBILITY = 10;
GASEngine.KeyframeAnimation.TARGET_RQ = 11;
GASEngine.KeyframeAnimation.TARGET_RQ_X = 11;
GASEngine.KeyframeAnimation.TARGET_RQ_Y = 12;
GASEngine.KeyframeAnimation.TARGET_RQ_Z = 13;
GASEngine.KeyframeAnimation.TARGET_RQ_W = 14;
GASEngine.KeyframeAnimation.TARGET_TOTAL_COUNT = 15;

GASEngine.KeyframeAnimation.prototype =
{
    constructor: GASEngine.KeyframeAnimation,

    //动画名称
    get id()
    {
        return `${this._clipName}_${this.guid}`;
    },

    get name() 
    {
        return this._clipName;
    },

    get clipName() 
    {
        return this._clipName;
    },

    set clipName(value)
    {
        this._clipName = value;
    },

    //动画片段ID
    set clipID(val)
    {
        this._clipID = val;
    },

    get clipID()
    {
        return this._clipID;
    },

    //FPS
    get FPS()
    {
        return this.fps;
    },

    //钳制启动开关
    set enableClamp(val)
    {
        this._enableClamp = val;
    },

    get enableClamp()
    {
        return this._enableClamp;
    },    

    //最小帧
    get SF()
    {
        return this.startFrame;
    },

    //最大帧
    get EF()
    {
        return this.endFrame;
    },

    //当前第几帧
    get CF()
    {
        return ((this._clipLocalTime - this.startFrame / this.fps) * this.fps);
    },

    //钳制最小帧
    set clampedSF(val)
    {
        if(val > this.clampedEndFrame)
        {
            val = this.clampedEndFrame;
        }        
        this.clampedStartFrame = val;
    },

    get clampedSF()
    {
        return this.clampedStartFrame;
    },

    //钳制最大帧
    set clampedEF(val)
    {
        if(val < this.clampedStartFrame)
        {
            val = this.clampedStartFrame;
        }
        this.clampedEndFrame = val;
    },
    
    get clampedEF()
    {
        return this.clampedEndFrame;
    },    

    //当前进度百分比
    get progress()
    {
        return (this._clipLocalTime*this.fps - this.startFrame) / (this.endFrame - this.startFrame);
    },

    //设置当前进度百分比
    set progress(val)
    {       
        var low = 0;
        var high = 1;
        if(this.enableClamp)
        {
            low = this.clampedSF / (this.endFrame - this.startFrame);
            high = this.clampedEF / (this.endFrame - this.startFrame);
        }
        val = Math.clamp(val, low, high);
        
        this.localTime = this.startFrame / this.fps + val * (this.endFrame / this.fps - this.startFrame / this.fps);

        this.forceUpdate = true;
    },    

    get startTime()
    {
        return (this.startFrame / this.fps);
    },

    get endTime()
    {
        return (this.endFrame / this.fps);
    },

    get duration()
    {
        return (this.endFrame / this.fps - this.startFrame / this.fps);
    },

    set enable(val)
    {
        this.updateEnable = val;
    },

    get enable()
    {
        return this.updateEnable;
    },

    set speed(val)
    {
        this.timeMultiplier = val;
    },

    get speed()
    {
        return this.timeMultiplier;
    },

    get localTime()
    {
        return this._clipLocalTime;
    },    

    set localTime(value)
    {
        if(typeof value === 'number' && !Number.isNaN(value)) {
            this._clipLocalTime = value;
        } else {
            console.error('GASEngine.KeyframeAnimation: .progress error: ', value);
        }
    },    
    
    get clipStarted()
    {
        return this._clipStarted;
    },

    get clipEnded()
    {
        return this._clipEnded;
    },

    interplolateQuaternion: function(keyIndexType, keyValueType, times, values, duration, p)
    {
        if(keyIndexType === undefined)
        {
            keyIndexType = 1;
        }

        if(keyValueType === undefined)
        {
            keyValueType = 10;
        }

        //var n = Math.fmod(p, duration);
        //if(n == 0 && p > 0)
        //{
        //    n = duration;
        //}
        var n = p;
    
        if(n <= times[0] || times.length === 1)
        {
            if(keyValueType === 10) // KEY_QUATERNION_BEZIER_MMD
            {
                return values[0].value;
            }
            else if(keyValueType === 9)
            {
                return values[0];
            }
            else
            {
                console.error("Unsupported key frame data type.");
            }
        }
    
        if(n >= times[times.length - 1])
        {
            if(keyValueType === 10) // KEY_QUATERNION_BEZIER_MMD
            {
                return values[values.length - 1].value;
            }
            else if(keyValueType === 9)
            {
                return values[values.length - 1];
            }
            else
            {
                console.error("Unsupported key frame data type.");
            }
        }
    
        var index;
        if(times._lastValue === undefined) {
            times._lastValue = 0;
        }
        
        if(n < times[times._lastValue])
        {
            for(var i = 0; i <= times._lastValue; i++)
            {
                if(n < times[i])
                {
                    index = i;
                    times._lastValue = i;
                    break;
                }
            }
        }
        else
        {
            for(var i = times._lastValue; i < times.length; i++)
            {
                if(n < times[i])
                {
                    index = i;
                    times._lastValue = i;
                    break;
                }
            }
        }

        var t0 = times[index - 1];
        var t1 = times[index];
        var weight = (n - t0) / (t1 - t0);
    
        var newValue = undefined;
        if(keyValueType === 10) // KEY_FLOAT_BEZIER_MMD
        {
            newValue = values[index - 1].value.clone();
            newValue.slerp(values[index].value, weight);
        }
        else if(keyValueType === 9)
        {
            newValue = values[index - 1].clone();
            newValue.slerp(values[index], weight);
        }
        else
        {
            console.error("Unsupported key frame data type.");
        }

        return newValue;
    },

    _interplolateValue: function(times, values, p)
    {
        var n = p;

        if(n <= times[0] || times.length === 1)
        {
            return values[0];
        }

        if(n >= times[times.length - 1])
        {
            return values[values.length - 1];
        }

        var index;
        for(var i = times._lastValue; i < times.length; i++)
        {
            if(n < times[i])
            {
                index = i;
                times._lastValue = i;
                break;
            }
        }

        var t0 = times[index - 1];
        var t1 = times[index];
        var weight = (n - t0) / (t1 - t0);

        //weight = Math.floor(weight + 0.5);//clamp to nearest keyframe
        var newValue = values[index - 1] * (1.0 - weight) + values[index] * weight;

        return newValue;
    },

    interplolateFloat: function(keyIndexType, keyValueType, times, values, duration, p)
    {
        if(keyIndexType === undefined)
        {
            keyIndexType = 0;
        }

        if(keyValueType === undefined)
        {
            keyValueType = 1;
        }
        //var n = Math.fmod(p, duration);
        //if(n == 0 && p > 0)
        //{
        //    n = duration;
        //}
        var n = p;

        if(n <= times[0] || times.length === 1)
        {
            if(keyValueType === 2) // KEY_FLOAT_BEZIER_MMD
            {
                return values[0].value;
            }
            else if(keyValueType === 1)
            {
                return values[0];
            }
            else
            {
                console.error("Unsupported key frame data type.");
            }
        }

        if(n >= times[times.length - 1])
        {
            if(keyValueType === 2) // KEY_FLOAT_BEZIER_MMD
            {
                return values[values.length - 1].value;
            }
            else if(keyValueType === 1)
            {
                return values[values.length - 1];
            }
            else
            {
                console.error("Unsupported key frame data type.");
            }
        }

        var index;
        if(n < times[times._lastValue])
        {
            for(var i = 0; i <= times._lastValue; i++)
            {
                if(n < times[i])
                {
                    index = i;
                    times._lastValue = i;
                    break;
                }
            }
        }
        else
        {
            for(var i = times._lastValue; i < times.length; i++)
            {
                if(n < times[i])
                {
                    index = i;
                    times._lastValue = i;
                    break;
                }
            }
        }

        var t0 = times[index - 1];
        var t1 = times[index];
        var weight = (n - t0) / (t1 - t0);

        //weight = Math.floor(weight + 0.5);//clamp to nearest keyframe
        var newValue = undefined;
        if(keyValueType === 2) // KEY_FLOAT_BEZIER_MMD
        {
            var newValue = values[index - 1].value * (1.0 - weight) + values[index].value * weight;
        }
        else if(keyValueType === 1)
        {
            var newValue = values[index - 1] * (1.0 - weight) + values[index] * weight;
        }
        else
        {
            console.error("Unsupported key frame data type.");
        }

        return newValue;
    },

    interplolateInt: function(times, values, duration, p)
    {
        //var n = Math.fmod(p, duration);
        //if(n == 0 && p > 0)
        //{
        //    n = duration;
        //}
        var n = p;

        if(n <= times[0] || times.length === 1)
        {
            return values[0];
        }

        if(n >= times[times.length - 1])
        {
            return values[values.length - 1];
        }

        var index;
        for(var i = 0; i < times.length; i++)
        {
            if(n < times[i])
            {
                index = i;
                break;
            }
        }

        var newValue = values[index - 1];

        return newValue;
    },
  
    absNumericalSort: function ( a, b ) 
    {
        return Math.abs( b[ 0 ] ) - Math.abs( a[ 0 ] );

    },    

    onAnimationLoadOver: function()
    {
    },
    
    detectEulerRotationTargets: function(kfs)
    {
        var hasEulerRotation = false;
        for(var i = 0; i < kfs.length; ++i)
        {
            if(kfs[i].target === GASEngine.KeyframeAnimation.TARGET_RX || 
                kfs[i].target === GASEngine.KeyframeAnimation.TARGET_RY || 
                kfs[i].target === GASEngine.KeyframeAnimation.TARGET_RZ)
            {
                hasEulerRotation = true;
                break;
            }
        }

        return hasEulerRotation;
    },

    //TODO: This function is really slow.
    eulerToQuaternionKFS: function(kfs, originalEX, originalEY, originalEZ, order)
    {
        var EX_T, EY_T, EZ_T;
        var EX_V, EY_V, EZ_V;
        var hasEulerRotation = false;

        var maxLength = 1;
        for(var i = 0; i < kfs.length; ++i)
        {
            if(kfs[i].target === GASEngine.KeyframeAnimation.TARGET_RX)
            {
                EX_T = kfs[i].t;
                EX_V = kfs[i].v;
                kfs[i].target = -1;

                if(EX_T.length > maxLength)
                {
                    maxLength = EX_T.length;
                }
                hasEulerRotation = true;
            }
            else if(kfs[i].target === GASEngine.KeyframeAnimation.TARGET_RY)
            {
                EY_T = kfs[i].t;
                EY_V = kfs[i].v;
                kfs[i].target = -1;

                if(EY_T.length > maxLength)
                {
                    maxLength = EY_T.length;
                }
                hasEulerRotation = true;
            }
            else if(kfs[i].target === GASEngine.KeyframeAnimation.TARGET_RZ)
            {
                EZ_T = kfs[i].t;
                EZ_V = kfs[i].v;
                kfs[i].target = -1;

                if(EZ_T.length > maxLength)
                {
                    maxLength = EZ_T.length;
                }
                hasEulerRotation = true;
            }
            else if(kfs[i].target === GASEngine.KeyframeAnimation.TARGET_RQ)
            {
                return;
            }
        }

        if(!hasEulerRotation)
        {
            return;
        }

        if(!EX_T)
        {
            EX_T = [0];
            EX_V = [originalEX];
        }

        if(!EY_T)
        {
            EY_T = [0];
            EY_V = [originalEY];
        }

        if(!EZ_T)
        {
            EZ_T = [0];
            EZ_V = [originalEZ];
        }

        var newt = [];
        var x, y, z, minT;
        var i0 = 0, i1 = 0, i2 = 0;
        while(i0 < EX_T.length || i1 < EY_T.length || i2 < EZ_T.length)
        {
            x = (i0 >= EX_T.length) ? Number.MAX_VALUE : EX_T[i0];
            y = (i1 >= EY_T.length) ? Number.MAX_VALUE : EY_T[i1];
            z = (i2 >= EZ_T.length) ? Number.MAX_VALUE : EZ_T[i2];

            minT = Math.min(x, Math.min(y, z));
            if(newt.length == 0 || (newt[newt.length - 1] < minT))
            {
                newt.push(minT);
            }

            if(minT === x)
                i0++;

            if(minT === y)
                i1++;

            if(minT === z)
                i2++;
        }
        newt._lastValue = 0;
        
        var newv = [];

        for(var k = 0; k < newt.length; ++k)
        {
            var index = newt[k];
            var x = this._interplolateValue(EX_T, EX_V, index);
            var y = this._interplolateValue(EY_T, EY_V, index);
            var z = this._interplolateValue(EZ_T, EZ_V, index);
            x = GASEngine.degToRad(x);
            y = GASEngine.degToRad(y);
            z = GASEngine.degToRad(z);

            var euler = new GASEngine.Euler(x, y, z, order);
            var quaternion = new GASEngine.Quaternion();
            quaternion.setFromEuler(euler, false);
            newv.push(quaternion);
        }

        kfs.push
        (
            {
                     "keyIndexType": 0,
                     "keyValueType": 9,
                     "t": newt,
                     "v": newv,
                     "target": 11,
                     "targetName": 'transform.rotation.quaternion'
            }
        );
    },    

    linkToObjectsAndProperties: function(entity)
    {
        for(var i = 0; i < this.animatedNodes.length; i++)
        {
            var targetNode = this.animatedNodes[i];

            var animatedObject = null;
            if(targetNode.uniqueID !== 0xffffffff)
            {
                animatedObject = entity.findObjectByID_r(targetNode.uniqueID);                                
            }
            else
            {
                animatedObject = entity.findChildEntityByName_r(targetNode.nodeName);
            }
           
            if(animatedObject === null)
            {
                console.error('GASEngine.KeyframeAnimation.linkToObjectsAndProperties: animation link failed. Cannot find the specified object!');
            }
            else
            {
                targetNode.node = animatedObject;
            }            
        }
    },

    animateTRS_V3: function()
    {
        for(var i = 0; i < this.animatedNodes.length; i++)
        {
            var animatedObject = this.animatedNodes[i];

            if(!animatedObject.node)
            {
                continue;
            }

            //TODO: THERE WILL BE ERROR WHEN CLIP SWITCHED!
            // var record = this._originValueDB_.get(animatedObject.node.uniqueID);
            // if(record === undefined)
            // {
            //     record = {};
            //     record.traditionalTarget = [];
            //     record.traditionalTarget.length = GASEngine.KeyframeAnimation.TARGET_TOTAL_COUNT;
            //     this._originValueDB_.set(animatedObject.node.uniqueID, record);

            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_TX] = animatedObject.node.translation.x;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_TY] = animatedObject.node.translation.y;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_TZ] = animatedObject.node.translation.z;

            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_RX] = animatedObject.node.rotation.x;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_RY] = animatedObject.node.rotation.y;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_RZ] = animatedObject.node.rotation.z;

            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_SX] = animatedObject.node.scale.x;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_SY] = animatedObject.node.scale.y;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_SZ] = animatedObject.node.scale.z;

            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_RQ_X] = animatedObject.node.quaternion.x;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_RQ_Y] = animatedObject.node.quaternion.y;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_RQ_Z] = animatedObject.node.quaternion.z;
            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_RQ_W] = animatedObject.node.quaternion.w;

            //     record.traditionalTarget[GASEngine.KeyframeAnimation.TARGET_VISIBILITY] = animatedObject.node.MB_PROPS.Visibility;
            // }            

            if(animatedObject._eulerToQuaternionFlag === undefined)
            {
                var hasEulerRotation = this.detectEulerRotationTargets(animatedObject.kfs);
                if(hasEulerRotation)
                {
                    this.eulerToQuaternionKFS
                    (
                        animatedObject.kfs, 
                        GASEngine.radToDeg(animatedObject.node.rotation.x), 
                        GASEngine.radToDeg(animatedObject.node.rotation.y), 
                        GASEngine.radToDeg(animatedObject.node.rotation.z), 
                        'ZYX'
                    );
                    animatedObject._eulerToQuaternionFlag = true;
                }
            }   
            
            var morphWeightIndex = 0;
            for(var m = 0; m < animatedObject.kfs.length; ++m)
            {
                var keyframeData = animatedObject.kfs[m];

                if(keyframeData != null)
                {                    
                    if(keyframeData.t.length > 0 && keyframeData.v.length == keyframeData.t.length)
                    {
                        var newValue = 0;
                        var frameIndex = this.localTime * this.fps;
                        if(keyframeData.target === GASEngine.KeyframeAnimation.TARGET_VISIBILITY)
                        {
                            newValue = this.interplolateInt
                            (
                                keyframeData.t, 
                                keyframeData.v, 
                                animatedObject.duration, 
                                frameIndex
                            );
                        }
                        else if(keyframeData.target === GASEngine.KeyframeAnimation.TARGET_RQ)
                        {
                            newValue = this.interplolateQuaternion
                            (
                                keyframeData.keyIndexType,
                                keyframeData.keyValueType,
                                keyframeData.t, 
                                keyframeData.v, 
                                animatedObject.duration, 
                                frameIndex
                            );
                        }
                        else
                        {
                            newValue = this.interplolateFloat
                            (
                                keyframeData.keyIndexType,
                                keyframeData.keyValueType,
                                keyframeData.t,
                                keyframeData.v,
                                animatedObject.duration,
                                frameIndex
                            );
                        }
                        
                        switch(keyframeData.target)
                        {
                            case GASEngine.KeyframeAnimation.TARGET_TX:
                            {
                                animatedObject.node.translation.setX(newValue);
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_TY:
                            {
                                animatedObject.node.translation.setY(newValue);
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_TZ:
                            {
                                animatedObject.node.translation.setZ(newValue);
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_RX:
                            {
                                animatedObject.node.rotation._x = GASEngine.degToRad(newValue);
                                animatedObject.node.rotation._order = 'ZYX';
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_RY:
                            {
                                animatedObject.node.rotation._y = GASEngine.degToRad(newValue);
                                animatedObject.node.rotation._order = 'ZYX';
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_RZ:
                            {
                                animatedObject.node.rotation._z = GASEngine.degToRad(newValue);
                                animatedObject.node.rotation._order = 'ZYX';
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_SX:
                            {
                                animatedObject.node.scale.setX(newValue);
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_SY:
                            {
                                animatedObject.node.scale.setY(newValue);
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_SZ:
                            {
                                animatedObject.node.scale.setZ(newValue);
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_RQ:
                            {
                                animatedObject.node.quaternion._x = newValue._x;
                                animatedObject.node.quaternion._y = newValue._y;
                                animatedObject.node.quaternion._z = newValue._z;
                                animatedObject.node.quaternion._w = newValue._w;
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_RQ_Y:
                            {
                                animatedObject.node.quaternion.y = newValue;
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_RQ_Z:
                            {
                                animatedObject.node.quaternion.z = newValue;
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_RQ_W:
                            {
                                animatedObject.node.quaternion.w = newValue;
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_MORPH_WEIGHT:
                            {
                                //TODO: SET MORPH WITH TARGET NAME
                                var mesh = animatedObject.node.getMesh();
                                if(mesh !== null)
                                {
                                    mesh.setMorphWeight(morphWeightIndex, newValue);
                                }
                                ++morphWeightIndex;
                                break;
                            }
                            case GASEngine.KeyframeAnimation.TARGET_VISIBILITY:
                            {
                                //console.error('Visibility animated target does not implented.');
                                break;
                            }
                            case -1:
                            {
                                break;
                            }
                            default:
                                console.error('Unknown animated target.');
                        }                        
                    }
                    else
                    {
                        console.error("Animation data is illegal.");
                    }                    
                }
                //<                
            }
        }        
    },    

    update: function(delta)
    {
        if(!this.updateEnable)
        {
            delta = 0.0;
        }

        this.animateTRS_V3();

        var startTime = this.startFrame / this.fps;
        if(this._enableClamp)
        {
            startTime = this.clampedSF / this.fps;
        }

        if(this._clipLocalTime <= startTime)
        {
            this.localTime = startTime;
            if(!this._clipStarted || this.forceUpdate)
            {
                this._clipStarted = true;
                this._clipEnded = false;
                this.onAnimationStartCallback(this);
            }
        }

        if((delta > 0.0 && this.onAnimationUpdateCallback) || this.forceUpdate)
        {
            this.forceUpdate = false;
            this.onAnimationUpdateCallback(this);
        }

        this.localTime += (delta * this.timeMultiplier);

        var endTime = this.endFrame / this.fps;
        if(this._enableClamp)
        {
            endTime = this.clampedEF / this.fps;
        }

        if(this._clipLocalTime >= endTime)
        {
            this.localTime = endTime;
            if(!this._clipEnded)
            {
                this._clipEnded = true;
                this.onAnimationEndCallback(this);
            }
        }
    },

    unlinkNodes: function() {
        this.animatedNodes.forEach(node => {
            node.node = null;
        });
    },

    destroy: function()
    {
        this.unlinkNodes();
    }
};