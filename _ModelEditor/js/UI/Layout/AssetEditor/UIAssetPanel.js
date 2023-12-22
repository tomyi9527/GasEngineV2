(function ()
{
    var style = getComputedStyle(document.body);
    let itemColor = style.getPropertyValue('--item-color');

    let UIAssetPanel = function (options) 
    {
        mgs.UIPanel.call(this, options);

        options = options ? options : {};
    
        this.assetTree = new mgs.UIAssetTree
        (
            {
                id:'assetTree',
                styleOptions:
                {
                    width: '25%', // width:'256px',
                    'border-right-width':'1px',
                },
                handlerOption:
                {
                    direction:'right',
                    min:100,
                    max:400,
                },
                childStyleOptions:
                {
                    header:
                    {
                        height : '0px',
                    },
                    content:
                    {
                        margin: '0px',
                        background: itemColor,
                    },
                },
            }
        );
        this.content.append(this.assetTree);

        // this.assetGrid = new mgs.UIAssetGrid
        // (
        //     {
        //         id:'assetGrid',
        //         childStyleOptions:
        //         {
        //             header:
        //             {
        //                 height : '0px',
        //             },
        //             content:
        //             {
        //                 margin: '0px',
        //                 overflow: 'hidden',
        //             },
        //         },
        //     }
        // );
        // this.content.append(this.assetGrid);
    };
    mgs.classInherit(UIAssetPanel, mgs.UIPanel);
}());