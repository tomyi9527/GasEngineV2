import { color2RGB, RGB2Color } from './Util.js';
import ArtHubBase from './ArtHubBase.js';

const RenderModeList = [
    { id: 0, name: '正常' },
    { id: 1, name: '基色' },
    { id: 2, name: '法线' },
    { id: 3, name: '光照' },
    { id: 4, name: '金属' },
];

const SkeletonSale = [0.1, 10];

class ArtHubRenderer extends ArtHubBase{
    constructor(options) {
        super(options);
    }

    _getScaleByPercent(percent) {
        return (SkeletonSale[1] - SkeletonSale[0]) * percent / 100 + SkeletonSale[0];
    }

    _getPercentByScale(scale) {
        return (scale - SkeletonSale[0]) * 100 / (SkeletonSale[1] - SkeletonSale[0]);
    }
    // apis
    showTopology(flag) {
        return this._invokeIFrame('showTopology', flag);
    }

    setTopologyColor(color) {
        const colorArray = Array.isArray(color) ? color : color2RGB(color);
        return this._invokeIFrame('setTopologyColor', colorArray);
    }

    setTopologyAlpha(alpha) {
        const showTopology = alpha > 0;
        this.showTopology(showTopology);
        return this._invokeIFrame('setTopologyAlpha', alpha / 100);
    }

    getTopologyColor() {
        return this._invokeIFrame('getTopologyColor');
    }

    getTopologyAlpha() {
        return this._invokeIFrame('getTopologyAlpha');
    }

    showTPose(flag) {
        return this._invokeIFrame('showTPose', flag);
    }

    showSkeleton(flag) {
        return this._invokeIFrame('showSkeleton', flag);
    }

    set skeletonScale(percent) {
        const scaleValue = this._getScaleByPercent(percent);
        return this._invokeIFrame('setSkeletonScale', scaleValue);
    }

    setSkeletonScale(value) {
        return this._invokeIFrame('setSkeletonScale', value);
    }

    getSkeletonScale() {
        return this._invokeIFrame('getSkeletonScale');
    }

    changeRenderMode(mode) {
        return this._invokeIFrame('changeRenderMode', mode);
    }

    getRendererInfo() {
        return this._invokeIFrame('getRendererInfo')
            .then(res => {
                res.renderModeObj = RenderModeList.find(e => e.id === res.renderMode);
                res.wireframeColor = RGB2Color(res.wireframeColor);
                res.wireframeOpacity = res.wireframeEnable ? Math.round(res.wireframeOpacity * 100) : 0;
                res.skelentorScale = this._getPercentByScale(res.skelentorScale);
                return res;
            });
    }
}

export default ArtHubRenderer;