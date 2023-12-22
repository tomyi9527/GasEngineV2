GASEngine.WebGLRenderStates = function()
{
    this._gl_ = GASEngine.WebGLDevice.Instance.gl;

    this._newStates_ = new Map();
    this._appliedStates_ = new Map();

    GASEngine.WebGLRenderStates.Instance = this;
};

GASEngine.WebGLRenderStates.prototype =
{
    constructor: GASEngine.WebGLRenderStates,

    reset: function()
    {
        this._newStates_.clear();
        this._appliedStates_.clear();

        //var gl = this._gl_;
        // //gl.getParameter(gl.DEPTH_WRITEMASK);
        // //<
        // this._appliedStates_.set('depth_write_enable', 1);
         //gl.depthMask(true);

        // this._appliedStates_.set('depth_test_enable', 1);
        //gl.enable(gl.DEPTH_TEST);
        
        // this._appliedStates_.set('depth_test_mode', 0); //0:LEQUAL   1:EQUAL
        //gl.depthFunc(gl.LEQUAL);

        // //<
        // this._appliedStates_.set('stencil_test_enable', 0);
        //gl.disable(gl.STENCIL_TEST);
        //gl.stencilMask(0xFFFFFFFF); //to control which bits to write 
        //gl.stencilFunc(gl.NEVER, 0, 0xFFFFFFFF); // ((RefValue & Mask) Op (StencilBufferValue & Mask)) & StencilWriteMask
        //gl.stencilOp(gl.KEEP, gl.KEEP, gl.KEEP); //Stencil -> Depth

        // //<
        // this._appliedStates_.set('alpha_blend_enable', 0);
        //gl.disable(gl.BLEND);
        //gl.blendEquation(gl.FUNC_ADD);

        // this._appliedStates_.set('alpha_blend_mode', 0); //0: 1, 2, 3
        //gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
   
        // this._appliedStates_.set('color_write_mask', 0x0F);
        //gl.colorMask(true, true, true, true);

        // //<
        // this._appliedStates_.set('front_face', 0); // 0:CCW  1:CW
        //gl.frontFace(gl.CCW);

        // this._appliedStates_.set('culling_face', 0); // 0:BACK  1:FRONT
        //gl.cullFace(gl.BACK);

        // this._appliedStates_.set('culling_face_enable', 1);
        //gl.enable(gl.CULL_FACE);
    },

    //Depth related states
    setDepthWriteEnable: function(val) // 0, 1
    {
        this._newStates_.set('depth_write_enable', val);
    },

    setDepthTestEnable: function(val) // 0, 1
    {
        this._newStates_.set('depth_test_enable', val);
    },

    setDepthTestMode: function(val)
    {
        // 0: LEQUAL
        this._newStates_.set('depth_test_mode', val);
    },

    //Stencil related states
    setStencilTestEnable: function(val)
    {
        this._newStates_.set('stencil_test_enable', val);
    },

    setStencilMode: function(val)
    {
        this._newStates_.set('stencil_func_mode', val);
    },

    setStencilMask: function(val)
    {
        this._newStates_.set('stencil_mask', val);
    },

    setStencilOp: function(val)
    {
        this._newStates_.set('stencil_op', val);
    },

    //Alpha blend related states
    setAlphaBlendEnable: function(val)
    {
        this._newStates_.set('alpha_blend_enable', val);
    },

    setAlphaBlendMode: function(val)
    {
        //0: AlphaBlending, ONE, ZERO
        //1: NormalBlending, SRC_ALPHA, ONE_MINUS_SRC_ALPHA, 
        //2: AdditiveBlending, ONE, ONE
        //3: SubtractiveBlending, ONE, ONE_MINUS_SRC_ALPHA
        this._newStates_.set('alpha_blend_mode', val); 
    },

    //Color write related states
    setColorWriteMask: function(val)
    {
        this._newStates_.set('color_write_mask', val);
    },

    //0:Both	Renders both the inner and outer polygons for the selected model.
    //1:Outer (Counter-Clockwise)	Renders only the polygons that compose the outside of the model.
    //2:Inner (Clockwise)	Renders only the polygons that compose the inside of the selected model.
    //Culling, right hand coordination like opengl webgl follow ccw front rule.
    //TODO: this value affect the shader pre-defined variable gl_FrontFacing, so as to affect the direction of the final normal.
    setFrontFace: function(val)
    {
        this._newStates_.set('front_face', val);
    },

    setCullingFace: function(val)
    {
        this._newStates_.set('culling_face', val);
    },

    setCullingFaceEnable: function(val)
    {
        this._newStates_.set('culling_face_enable', val ? 1 : 0);
    },

    //<
    applyRenderStates: function()
    {
        this._applyDepthRelatedStates();
        this._applyStencilRelatedStates();
        this._applyAlphaBlendRelatedStates();
        this._applyCullingFaceRelatedStates();
    },

    _applyDepthRelatedStates: function()
    {
        var gl = this._gl_;
        var newValue = 0;
        var oldValue = 0;
        //<
        newValue = this._newStates_.get('depth_test_enable');
        oldValue = this._appliedStates_.get('depth_test_enable');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.disable(gl.DEPTH_TEST);
            }
            else
            {
                gl.enable(gl.DEPTH_TEST);
            }
            //this._appliedStates_.set('depth_test_enable', newValue);
        }
        //<
        newValue = this._newStates_.get('depth_write_enable');
        oldValue = this._appliedStates_.get('depth_write_enable');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.depthMask(false);
            }
            else
            {
                gl.depthMask(true);
            }
            //this._appliedStates_.set('depth_write_enable', newValue);
        }
        //<
        newValue = this._newStates_.get('depth_test_mode');
        oldValue = this._appliedStates_.get('depth_test_mode');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.depthFunc(gl.LEQUAL);
            }
            else if(newValue === 1)
            {
                gl.depthFunc(gl.EQUAL);
            }
            else if(newValue === 2)
            {
                gl.depthFunc(gl.ALWAYS);
            }
            //this._appliedStates_.set('depth_test_mode', newValue);
        }
    },

    _applyStencilRelatedStates: function()
    {
        /*
        gl.stencil​Func()
        gl.stencil​Func​Separate()
        gl.stencil​Mask()
        gl.stencil​Mask​Separate()
        gl.stencilOp()
        gl.stencil​OpSeparate()
        
        status = stencilfunc.func((stencilbuf[x,y] & stencilfunc.mask), (stencilfunc.ref & stencilfunc.mask));
        status |= depth_test_result;
        if (status == stencil_test_fail) stencilop = sfailop;
        else if (status == stencil_test_pass & depth_test_fail) stencilop = dpfailop;
        else if (status == stencil_test_pass & depth_test_pass) stencilop = dppassop;
        // stencil test结束后的操作不需要mask
        stencil_new_value = stencilop(stencilbuf[x,y]);
        // 写入stencil buffer的时候需要另一个mask
        stencilbuf[x,y] = (stencil_new_value & stencilmask.mask) | (stencilbuf[x,y] & (~stencilmask.mask));
        */
        var gl = this._gl_;
        var newValue = 0;
        var oldValue = 0;

        //<
        newValue = this._newStates_.get('stencil_test_enable');
        oldValue = this._appliedStates_.get('stencil_test_enable');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.disable(gl.STENCIL_TEST);
            }
            else if(newValue === 1)
            {
                gl.enable(gl.STENCIL_TEST);
            }
            //this._appliedStates_.set('stencil_test_enable', newValue);
        }
        
        newValue = this._newStates_.get('stencil_func_mode');
        oldValue = this._appliedStates_.get('stencil_func_mode');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.stencilFunc(gl.NEVER, 0, 0xFFFFFFFF); // ((RefValue & Mask) Op (StencilBufferValue & Mask)) & StencilWriteMask
            }
            else
            {
                gl.stencilFunc(gl.ALWAYS, newValue, 0xff);
            }
            //this._appliedStates_.set('stencil_func_mode', newValue);
        }

        newValue = this._newStates_.get('stencil_mask');
        oldValue = this._appliedStates_.get('stencil_mask');
        if(newValue !== undefined && newValue !== oldValue)
        {
            gl.stencilMask(newValue);
            //this._appliedStates_.set('stencil_mask', newValue);
        }

        newValue = this._newStates_.get('stencil_op');
        oldValue = this._appliedStates_.get('stencil_op');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.stencilOp(gl.KEEP, gl.KEEP, gl.REPLACE);
            }
            else
            {
                gl.stencilOp(gl.KEEP, gl.REPLACE, gl.REPLACE); //for highlight without occluding.
            }
            //this._appliedStates_.set('stencil_op', newValue);
        }
    },

    _applyAlphaBlendRelatedStates: function()
    {
        var gl = this._gl_;
        var newValue = 0;
        var oldValue = 0;
        //<
        newValue = this._newStates_.get('alpha_blend_enable');
        oldValue = this._appliedStates_.get('alpha_blend_enable');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.disable(gl.BLEND);
            }
            else if(newValue === 1)
            {
                gl.enable(gl.BLEND);
                gl.blendEquation(gl.FUNC_ADD);
            }
            //this._appliedStates_.set('alpha_blend_enable', newValue);
        }
        //<
        newValue = this._newStates_.get('alpha_blend_mode');
        oldValue = this._appliedStates_.get('alpha_blend_mode');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.blendFuncSeparate(gl.ONE, gl.ZERO, gl.ONE, gl.ZERO);
            }
            else if(newValue === 1)
            {
                gl.blendFuncSeparate(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA, gl.ONE, gl.ONE);
            }
            else if(newValue === 2)
            {
                gl.blendFunc(gl.ONE, gl.ONE);                
            }
            else if(newValue === 3)
            {
                gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
            }

            //this._appliedStates_.set('alpha_blend_mode', newValue);
        }
        //<
        newValue = this._newStates_.get('color_write_mask');
        oldValue = this._appliedStates_.get('color_write_mask');
        if(newValue !== undefined && newValue !== oldValue)
        {
            var writeR = ((newValue & 0x01) === 1);
            var writeG = ((newValue & 0x02) === 2);
            var writeB = ((newValue & 0x04) === 4);
            var writeA = ((newValue & 0x08) === 8);
            gl.colorMask(writeR, writeG, writeB, writeA);

            //this._appliedStates_.set('color_write_mask', newValue);
        }
    },

    _applyCullingFaceRelatedStates: function()
    {
        var gl = this._gl_;
        var newValue = 0;
        var oldValue = 0;
        //<
        newValue = this._newStates_.get('front_face');
        oldValue = this._appliedStates_.get('front_face');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.frontFace(gl.CCW);
            }
            else
            {
                gl.frontFace(gl.CW);
            }
            //this._appliedStates_.set('front_face', newValue);
        }
        //<
        newValue = this._newStates_.get('culling_face');
        oldValue = this._appliedStates_.get('culling_face');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.cullFace(gl.BACK);
            }
            else if(newValue === 1)
            {
                gl.cullFace(gl.FRONT);
            }
            else if(newValue === 3)
            {
                gl.cullFace(gl.FRONT_AND_BACK);
            }
            //this._appliedStates_.set('culling_face', newValue);
        }
        //<
        newValue = this._newStates_.get('culling_face_enable');
        oldValue = this._appliedStates_.get('culling_face_enable');
        if(newValue !== undefined && newValue !== oldValue)
        {
            if(newValue === 0)
            {
                gl.disable(gl.CULL_FACE);
            }
            else
            {
                gl.enable(gl.CULL_FACE);
            }
            //this._appliedStates_.set('culling_face_enable', newValue);
        }
        //<
    }
};