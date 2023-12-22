
class GifCapture 
{
    constructor(options = {}) 
    {
        this.option = {};
        Object.assign(this.option, 
        {
            fps: 10,
            width: 320,
            height: 200,
            maxFrame: 100,
            workers: 4,
        }, options);
        this.gifWorkerPath = this.option.path;
    }

    onFinishedCallback(data)
    {
        console.log(data);
    }

    start(engineCanvas) 
    {
        this.frame_captured = 0;
        this.frame_countdown = this.option.maxFrame;
        this.delay = 1000.0 / this.option.fps;

        if(!this.canvas) 
        {
            this.canvas = document.createElement('canvas');
        }
        this.canvas.width = this.option.width;
        this.canvas.height = this.option.height;
        this.context = this.canvas.getContext('2d');

        this.encoder = new GIF({
            workers: this.option.workers,
            workerScript: this.gifWorkerPath,
            width: this.option.width,
            height: this.option.height,
        });

        this.encoder.on('finished', res => 
        {
            this.onFinishedCallback({
                type: 'finish',
                blob: res,
            });
            // Terminate workers.
            this.encoder.abort();
            this.encoder = null;
        });

        // Calc canvas scale param.
        let sx, sy, sw, sh;
        if (this.option.width / this.option.height > engineCanvas.width / engineCanvas.height) 
        {
            // Capture size is wilder than glcanvas.
            sx = 0;
            sy = (engineCanvas.height - this.option.height / this.option.width * engineCanvas.width) / 2.0;
            sw = engineCanvas.width;
            sh = sw * this.option.height / this.option.width;
        } 
        else 
        {
            // Capture size is taller than glcanvas.
            sy = 0;
            sx = (engineCanvas.width - this.option.width / this.option.height * engineCanvas.height) / 2.0;
            sh = engineCanvas.height;
            sw = sh * this.option.width / this.option.height;
        }

        if (!this.captureFrame) 
        {
            this.captureFrame = () => 
            {
                this._timeoutID = setTimeout(() => 
                {
                    if (this.frame_countdown > 0) 
                    {
                        this.context.drawImage(engineCanvas, sx, sy, sw, sh, 0, 0, this.option.width, this.option.height);
                        this.encoder.addFrame(this.context, 
                        {
                            copy: true,
                            delay: this.delay,
                        });

                        if (this.frame_captured % this.option.fps === 0) 
                        {
                            this.onFinishedCallback({
                                type: 'capture',
                                time: this.frame_captured / this.option.fps,
                            });
                        }

                        --this.frame_countdown;
                        ++this.frame_captured;
                        this.captureFrame();
                    } 
                    else 
                    {
                        this.onFinishedCallback({
                            type: 'render'
                        });

                        this._timeoutID = null;

                        this.finish();
                    }
                }, this.delay);
            }
        }

        this.doCapture();
    }

    doCapture()
    {
        this._timeoutID = null;
        this.captureFrame();
    }

    stop()
    {
        this.frame_countdown = 0; // Next frame was the last frame.
    }

    finish() 
    {
        if(this.frame_captured > 0)
        {
            this.frame_captured = 0;
            this.encoder.render();
        }
    }
}
