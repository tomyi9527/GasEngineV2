(function ()
{
    let UIViewportPanel= function(options) 
    {
        mgs.UILayout.call(this, options);

        this.sceneEditor = options.sceneEditor;
    
        ///////////////////////////////toolbar////////////////////
        this.toolBar = new mgs.UILayout
        (
            {
                id:'toolBar',
                styleOptions:
                {
                    height:'30px',
                    'border-bottom-width':'1px',
                }
            }
        );

        let leftPanel = new mgs.UILayout
        (
            {
                id:'toolBar-leftPanel',
                styleOptions:
                {
                    height : '30px',
                    'border-right-width' : '1px',
                    'flex-grow' : '1',
                    'position' : 'relative',
                }
            }
        );
        let leftContentPanel = new mgs.UILayout
        (
            {
                id:'toolBar-leftContentPanel',
                styleOptions:
                {
                    height : '30px',
                    'display' : 'flex',
                    'align-items' : 'center',
                    'position' : 'absolute',
                    'overflow' : 'auto',
                    'left' : '0px',
                    'right' : '0px',
                    'top' : '0px',
                    'bottom' : '0px',
                    'white-space' : 'nowrap',
                }
            }
        );

        /*************************************** mutex transform buttons ***************************************/
        let toolBarStateBtnStyle = 
        {
            width:'25px',
            height:'25px',
            'line-height':'25px',
            'flex-grow':'0',
            'flex-shrink':'0',
            'padding':'0px',
            'margin' : '0px 1.5px',
            'font-family' : 'mgs-icon',
            'font-size' : '18px',
            'cursor' : 'pointer',
            'font-weight' : 'normal',
        };

        let mutexButtonInfos =
        [
            { selectID:'translate', text: mgs.ICONS.moveBtn },
            { selectID:'rotate', text: mgs.ICONS.rotateBtn },
            { selectID:'scale', text: mgs.ICONS.scaleBtn }
        ];
        let mutexButtons = {};
        let transformSelected = 'translate';

        let onStateChange = function(ui, selected)
        {
            if (selected)
            {
                for (let key in mutexButtons)
                {
                    let mutexButton = mutexButtons[key];
                    if (mutexButton !== ui)
                    {
                        if (mutexButton.selected)
                        {
                            mutexButton.setSelected(false, true);
                        }
                    }
                }
            }
            else
            {
                mutexButtons[ui.selectID].setSelected(true, true);
            }

            transformSelected = ui.selectID;
            mgsEditor.emit(mgsEditor.EVENTS.toolBar.onTransformSelect, transformSelected);
        };

        for (let i = 0;i < mutexButtonInfos.length;i ++)
        {
            let mutexButtonInfo = mutexButtonInfos[i];
            let btn = new mgs.UIStateButton(
                {
                    text:mutexButtonInfo.text,
                    styleOptions : toolBarStateBtnStyle,
                }
            );
            btn.selectID = mutexButtonInfo.selectID;
            btn.onStateChange = onStateChange;
            leftContentPanel.append(btn);
            mutexButtons[mutexButtonInfo.selectID] = btn;
        }

        mutexButtons[mutexButtonInfos[0].selectID].setSelected(true); 

        mgsEditor.method(mgsEditor.EVENTS.toolBar.getTransformSelected, function()
        {
            return transformSelected;
        });


        let checkbox = new mgs.UICheckbox();
        checkbox.onChange = function(value)
        {
            mgsEditor.emit(mgsEditor.EVENTS.toolBar.onCheckboxChanged, value);
        };

        leftContentPanel.append(checkbox);

        let slider = new mgs.UISlider
        (
            {
                styleOptions:
                {
                    'margin-left': '15px',
                },
                precision: 1,
                min: 0.1,
                max: 1,
            }
        );
        leftContentPanel.append(slider);
        this.slider = slider;
        this.slider.on('onValueChange', function(value)
        {
            mgsEditor.emit(mgsEditor.EVENTS.toolBar.navigateUniformStepValue, value);
        });

        // var options = {
        //     elementTag: 'span',
        //     innerHTML: 'world',
        //     styleOptions: {
        //         display: 'inline-block',
        //         color: 'white'
        //     }
        // }
        // let checkboxValue = new mgs.UIBase(options);
        // let root = checkboxValue.getRoot();
        // root.innerHTML = 'local';
        // leftContentPanel.append(checkboxValue);
        
        leftPanel.append(leftContentPanel);

        let saveBtn = new mgs.UIButton
        (
            {
                // styleOptions:
                // {
                //     width:'25px',
                //     height:'25px',
                //     'margin-left': '15px',
                //     'font-family' : 'mgs-icon',
                // },
                styleOptions: toolBarStateBtnStyle,
                text: mgs.ICONS.saveBtn,
            }
        );
        leftContentPanel.append(saveBtn);
        this.saveBtn = saveBtn;
        this.saveBtn.on('onClick', function(evt)
        {
            mgsEditor.emit(mgsEditor.EVENTS.toolBar.onSaveBtnClick, evt);
        });

        this.toolBar.append(leftPanel);

        this.append(this.toolBar);


        //////////////////////////canvas viewport////////////////////////////
        this.viewportPanel = new mgs.UILayout
        (
            {
                // id:'viewport'
            }
        );

        this.canvas3d = new mgs.UICanvas
        (
            {
                // id:'canvas3d'
                // id:'viewport-canvas',
                canFocus: true,
            }
        );
        this.viewportPanel.append(this.canvas3d);

        let viewportPanelRoot = this.viewportPanel.getRoot();
        let viewportPanelWidth = 0;
        let viewportPanelHeight = 0;
        let self = this;

        this.viewportResizeInterval = setInterval(() => {
            let viewportPanelRect = viewportPanelRoot.getBoundingClientRect();
            if (viewportPanelWidth !== viewportPanelRect.width ||
                viewportPanelHeight !== viewportPanelRect.height)
            {
                self.sceneEditor.emit('onCanvasResize');
            }

            viewportPanelWidth = viewportPanelRect.width;
            viewportPanelHeight = viewportPanelRect.height;
        }, 1000/60);

        this.fpsElement = new mgs.UIBase
        (
            {
                id:'FPS',
                styleOptions: {
                    position: 'absolute',
                    color: 'red',
                    margin: '5px'
                }
            }
        );
        this.viewportPanel.append(this.fpsElement);

        this.append(this.viewportPanel);
    };
    mgs.classInherit(UIViewportPanel, mgs.UILayout);

    UIViewportPanel.prototype.destroy = function()
    {
        clearInterval(this.viewportResizeInterval);
    };

    UIViewportPanel.prototype.getToolBar = function()
    {
        return this.toolBar;
    }

    UIViewportPanel.prototype.getFpsElement = function()
    {
        return this.fpsElement;
    }

    UIViewportPanel.prototype.getCanvas3d = function()
    {
        return this.canvas3d;
    }

    UIViewportPanel.prototype.getCanvasPanel = function()
    {
        return this.viewportPanel;
    }

}());