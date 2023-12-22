//Animation UI
SceneEditor.AnimationUtils = 
{
    _htmlToElement: function(html)
    {
        var template = document.createElement('template');
        template.innerHTML = html;
        if(template.content)
        {
            return template.content.firstChild;
        }
        else
        {
            var node = template.childNodes[0];
            return node;
        }
    },

    _addZero: function(str, length)
    {
        return new Array(length - str.length + 1).join("0") + str;
    },

    updateAnimationProgress: function(clip)
    {
        if(SceneEditor.Editor.Instance.globalAnimationBar)
        {
            var animationProgress = clip.progress;
            SceneEditor.Editor.Instance.globalAnimationBar.style.transform = 'scaleX(' + animationProgress + ')';
        }
    
        if(SceneEditor.Editor.Instance.globalAnimationTimeText)
        {
            var animationLocalTime = clip.localTime;
            var sec = Math.floor(animationLocalTime);
            var ms = Math.floor((animationLocalTime - sec) * 100);
            SceneEditor.Editor.Instance.globalAnimationTimeText.innerHTML = SceneEditor.AnimationUtils._addZero(sec.toString(), 2) + ':' + SceneEditor.AnimationUtils._addZero(ms.toString(), 2);
        }
    },

    addAnimationPanel: function()
    {
        // var webglContainer = document.getElementById("viewport")
        var webglContainer = SceneEditor.Editor.Instance.container;

        var timelineHtml = '<div class="timeline" style="position:absolute; left:60px; bottom:50px; height:30px; right:180px; z-index:2">' +
        '<div style="box-sizing: border-box; display:flex; transition:all 200ms ease-out; position:absolute; left:50%; transform:translate(-50%,0); width:100%; height:100%; background:rgba(0,0,0,.75); padding:3px; border-radius:30px; max-width:570px;">' +
            '<a id="ANI_PLAY_BTN" class="animationSwitch" style="padding:0 3px 0 9px; font-size:13px; line-height:23px; color:#efefef; cursor:pointer; text-decoration:none; font-family:fontawesome;">' + '&#61516' + '</a>' +
            '<div id="ANI_TRACK" class="track" style="flex:1; position:relative; top:8px; height:8px; cursor:pointer; margin: 0 5px; background:#434343; border-radius:8px; overflow:hidden">' +
                '<div id="ANI_BAR" class="bar" style="position:absolute; pointer-event:none; transform:scaleX(0.0); background:#1caad9; width:100%; height:8px; left:0; transform-origin:0 0;">' + '</div>' +
                '<div id="ANI_KNOB" class="knob" style="display:none; left:52%; position:absolute; background:#FFFFFF; width:2px; height:100%; border-right: 1px solid rgba(0,0,0,.8); top:-1px; pointer-events:none;">' + '</div>' +
            '</div>' +
            '<p id="ANI_TIME_TXT" class="progress12" style="display:block; color:#FFFFFF; font-family:KaiTi; margin:0 10px 0 5px; padding:0; font-size:12px; line-height:24px; text-align:right;">' + '00:00' + '</p>' +
            '<div class="chooseAnimation" style="display:block; position:relative; color:#EEEEEE; flex:0 0 20px; margin-right:5px;">' +
            '<span style="display:inline-block; line-height:24px; background:transparent; font-family:FontAwesome; font-size: 14px; cursor: pointer;">' + '&#61642' + '</span>' +
            '</div>' +
        '</div>';

        var elementNode = SceneEditor.AnimationUtils._htmlToElement(timelineHtml);
        webglContainer.appendChild(elementNode);

        SceneEditor.Editor.Instance.globalAnimationBar = document.getElementById("ANI_BAR");
        SceneEditor.Editor.Instance.globalAnimationTimeText = document.getElementById("ANI_TIME_TXT");

        var animationTrack = document.getElementById("ANI_TRACK");
        var animationPlayButton = document.getElementById("ANI_PLAY_BTN");

        animationPlayButton.addEventListener("click", function()
        {
            if(SceneEditor.Editor.Instance.globalAnimationClips.length > SceneEditor.Editor.Instance.currentClipIndex && SceneEditor.Editor.Instance.currentClipIndex >= 0)
            {
                var clip = SceneEditor.Editor.Instance.globalAnimationClips[SceneEditor.Editor.Instance.currentClipIndex];

                clip.enable = !clip.enable;
                if(clip.enable)
                {
                    animationPlayButton.innerHTML = '&#61516';
                }
                else
                {
                    animationPlayButton.innerHTML = '&#61515';
                }
            }
        });

        var animationMouseDown = function(event)
        {
            if(event.button == 0)
            {
                SceneEditor.Editor.Instance.globalAnimationMouseDownFlag = true;

                trackLeft = animationTrack.getBoundingClientRect().left;
                MouseLeft = event.clientX;
                trackWidth = animationTrack.getBoundingClientRect().width;
                trackWidth = trackWidth == 0 ? 1 : trackWidth;
                progress = (MouseLeft - trackLeft) / trackWidth;

                if(SceneEditor.Editor.Instance.globalAnimationClips.length > SceneEditor.Editor.Instance.currentClipIndex && SceneEditor.Editor.Instance.currentClipIndex >= 0)
                {
                    var clip = SceneEditor.Editor.Instance.globalAnimationClips[SceneEditor.Editor.Instance.currentClipIndex];
                    clip.progress = progress;

                    SceneEditor.Editor.Instance.globalPreviousAnimationStatus = clip.enable;
                    clip.enable = false;
                    window.addEventListener("mousemove", animationMouseMove);
                    window.addEventListener("mouseup", animationMouseUp);
                }
            }
        };

        var animationMouseMove = function(event)
        {
            event.preventDefault();
            var rect1 = animationTrack.getBoundingClientRect();

            if(SceneEditor.Editor.Instance.globalAnimationMouseDownFlag)
            {
                var trackLeft = rect1.left;
                var MouseLeft = event.clientX;
                var trackWidth = rect1.width;
                var trackWidth = trackWidth == 0 ? 1 : trackWidth;
                var deltaLength = MouseLeft - trackLeft;
                var deltaLength = Math.min(Math.max(deltaLength, 0), trackWidth);
                var progress = deltaLength / trackWidth;

                if(SceneEditor.Editor.Instance.globalAnimationClips.length > SceneEditor.Editor.Instance.currentClipIndex && SceneEditor.Editor.Instance.currentClipIndex >= 0)
                {
                    var clip = SceneEditor.Editor.Instance.globalAnimationClips[SceneEditor.Editor.Instance.currentClipIndex];
                    clip.progress = progress;
                }
            }
        };

        var animationMouseUp = function(event)
        {
            if(event.button == 0)
            {
                SceneEditor.Editor.Instance.globalAnimationMouseDownFlag = false;

                window.removeEventListener("mousemove", animationMouseMove);
                window.removeEventListener("mouseup", animationMouseUp);

                if(SceneEditor.Editor.Instance.globalAnimationClips.length > SceneEditor.Editor.Instance.currentClipIndex && SceneEditor.Editor.Instance.currentClipIndex >= 0)
                {
                    var clip = SceneEditor.Editor.Instance.globalAnimationClips[SceneEditor.Editor.Instance.currentClipIndex];
                    clip.enable = SceneEditor.Editor.Instance.globalPreviousAnimationStatus;
                }
            }
        };

        animationTrack.addEventListener("mousedown", animationMouseDown);
    }
}