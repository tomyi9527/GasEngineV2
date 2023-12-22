import { color2RGB, RGB2Color } from './Util.js';
import ArtHubBase from './ArtHubBase.js';

const BackgroundImageList = [
    {id: 'SYSTEM_DARK_1.jpg', name: '磨砂黑', url: 'system/backgroundImages/SYSTEM_DARK_1.jpg'},
    {id: 'SYSTEM_DARK_2.jpg', name: '经典黑', url: 'system/backgroundImages/SYSTEM_DARK_2.jpg'},
    {id: 'SYSTEM_DARK_3.jpg', name: '金属黑', url: 'system/backgroundImages/SYSTEM_DARK_3.jpg'},
    {id: 'SYSTEM_GRAY_1.jpg', name: '磨砂灰', url: 'system/backgroundImages/SYSTEM_GRAY_1.jpg'},
    {id: 'SYSTEM_GRAY_2.jpg', name: '经典灰', url: 'system/backgroundImages/SYSTEM_GRAY_2.jpg'},
    {id: 'SYSTEM_LIGHT_GRAY.jpg', name: '浅灰', url: 'system/backgroundImages/SYSTEM_LIGHT_GRAY.jpg'},
    {id: 'SYSTEM_METAL.jpg', name: '金属银', url: 'system/backgroundImages/SYSTEM_METAL.jpg'},
    {id: 'SYSTEM_LIGHT_BLUE.jpg', name: '深蓝', url: 'system/backgroundImages/SYSTEM_LIGHT_BLUE.jpg'},
];

const EnvironmentBrightness = [0, 2];

class ArtHubSkybox  extends ArtHubBase{
    constructor(options) {
        super(options);
    }
    
    _setBackground(params) {
        return this._invokeIFrame('setBackground', params);
    }

    _getEnvironmentBrightnessValueByPercent(percent) {
        return (EnvironmentBrightness[1] - EnvironmentBrightness[0]) * percent / 100 + EnvironmentBrightness[0];
    }

    _getPercentByEnvironmentBrightnessValue(value) {
        return (value - EnvironmentBrightness[0]) * 100 / (EnvironmentBrightness[1] - EnvironmentBrightness[0]);
    }
    
    // apis
    /** 
     * @param
     * @param {String} type: 'SOLIDCOLOR','IMAGE','CUBEMAP',
     * @param {Object} params
     * e.g:
     * {type: 'SOLIDCOLOR', backgroundColor: {r: 1.0, g: 1.0, b: 1.0}}
     * {type: 'IMAGE', backgroundImage: 'SYSTEM_GRAY_2.jpg'}
     * {type: 'CUBEMAP', cubeMapName: '01_attic_room_with_windows', lightEnable: false, environmentBlur: 1, environmentBrightness: 0.6}
     * */
    getBackgroundInfo() {
        return this._invokeIFrame('getBackgroundInfo')
            .then(res => {
                res.environmentBrightness = this._getPercentByEnvironmentBrightnessValue(res.environmentBrightness);
                res.backgroundImageObj = BackgroundImageList.find(e => e.id === res.backgroundImage);
                res.backgroundColor = RGB2Color(res.backgroundColor);
                return res;
            })
    }

    set type(value) {
        this._setBackground({type: value});
    }
    
    set backgroundColor(value) {
        const colorArray = color2RGB(value);
        this._setBackground({backgroundColor: {r: colorArray[0], g: colorArray[1], b: colorArray[2]}});
    }

    set backgroundImage(value) {
        this._setBackground({backgroundImage: value});
    }

    set cubeMapName(value) {
        this._setBackground({cubeMapName: value});
    }

    set lightEnable(value) {
        this._setBackground({lightEnable: value});
    }

    set environmentBrightness(value) {
        const brightnessValue = this._getEnvironmentBrightnessValueByPercent(value);
        this._setBackground({environmentBrightness: brightnessValue});
    }

    set orientation(value) {
        const orientValue = Math.round(360 * value / 100);
        this._setBackground({orientation: orientValue});
    }

    set environmentBlur(value) {
        this._setBackground({environmentBlur: value});
    }

    getBackgroundImageList() {
        return this._invokeIFrame('getRootDirectory')
            .then((rootDirectory) => {
                const imageList = [...BackgroundImageList];
                imageList.forEach(e => e.url = `${rootDirectory}${e.url}`);
                return Promise.resolve(imageList);
            });
    }
}

export default ArtHubSkybox;