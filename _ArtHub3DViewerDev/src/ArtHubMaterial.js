import ArtHubBase from './ArtHubBase.js';
import { color2RGB, RGB2Color, animate } from './Util.js';

const additionalDielectricMaterialFields = [
    'albedoMapName',
    'metalnessMapName',
    'specularF0MapName',
    'roughnessMapName',
    'glossinessMapName',
    'normalMapName',
    'aoMapName',
    'cavityMapName',
    'opacityMapName',
    'emissiveMapName',
];

const additionalElectricMaterialFields = [
    'albedoMapName',
    'specularMapName',
    'roughnessMapName',
    'glossinessMapName',
    'normalMapName',
    'aoMapName',
    'cavityMapName',
    'opacityMapName',
    'emissiveMapName',
];

const FirstLetterToUpperCase = function(str) {
    return str.charAt(0).toUpperCase() + str.slice(1);
}

const allowedChoices = ['dielectric', 'electric'];

const cullingChoice = {
    off: 0, // show both side
    onCCW: 1, // cull back
    onCW: 2, // cull front
    onBoth: 3 // cull double side (show nothing)
};

const allMapFields = ['albedoMap','specularMap','metalnessMap','specularF0Map','roughnessMap','glossinessMap','normalMap','aoMap','cavityMap','opacityMap','emissiveMap'];

class ArtHubMaterial  extends ArtHubBase{
    constructor(options) {
        super(options);
        this._type = options.type;
        this._invokeFunctionField('enableMaterialBasicProperties');
        this._materialContexts = new Map();
    }

    init() {
        return new Promise((rs, rj)=>{
            let type = this.type;
            if (this.type === 'compound') {
                this.activeMaterialType.then(type=>{
                    this.setMaterialContext(type);
                    rs();
                });
            } else {
                this.setMaterialContext(type);
                rs();
            }
        });
    }

    _invokeGetField(field) {
        return this._invokeFunctionField('get' + FirstLetterToUpperCase(field));
    }

    _invokeSetField(field, value) {
        return this._invokeFunctionField('set' + FirstLetterToUpperCase(field), value);
    }

    _invokeFunctionField(field, value) {
        return this._invokeIFrame('invokeMaterialField', {field: field, id: this.id, value: value}).catch(e=>console.log(e));
    }

    get type() {
        return this._type;
    }

    get materialTypes() {
        return this._invokeGetField('materialTypes');
    }

    get activeMaterialType() {
        return this._invokeGetField('activeMaterialType');
    }

    set activeMaterialType(value) {
        this.setMaterialContext(value);
        return this._invokeSetField('activeMaterialType', value);
    }

    get culling() {
        return this._invokeGetField('culling');
    }
    set culling(value) {
        return this._invokeSetField('culling', value); 
    }

    get albedoEnable() { return this._invokeGetField('albedoEnable'); }
    get albedoColor() { return this._invokeGetField('albedoColor'); }
    get albedoFactor() { return this._invokeGetField('albedoFactor'); }
    get albedoMapPath() { return this._invokeGetField('albedoMapPath'); }
    get albedoMapImage() { return this._invokeGetField('albedoMapImage'); }
    get specularEnable() { return this._invokeGetField('specularEnable'); }
    get specularColor() { return this._invokeGetField('specularColor'); }
    get specularFactor() { return this._invokeGetField('specularFactor'); }
    get specularMapPath() { return this._invokeGetField('specularMapPath'); }
    get specularMapImage() { return this._invokeGetField('specularMapImage'); }
    get metalnessEnable() { return this._invokeGetField('metalnessEnable'); }
    get metalnessFactor() { return this._invokeGetField('metalnessFactor'); }
    get metalnessMapPath() { return this._invokeGetField('metalnessMapPath'); }
    get metalnessMapImage() { return this._invokeGetField('metalnessMapImage'); }
    get specularF0Enable() { return this._invokeGetField('specularF0Enable'); }
    get specularF0Factor() { return this._invokeGetField('specularF0Factor'); }
    get specularF0MapPath() { return this._invokeGetField('specularF0MapPath'); }
    get specularF0MapImage() { return this._invokeGetField('specularF0MapImage'); }
    get roughnessEnable() { return this._invokeGetField('roughnessEnable'); }
    get roughnessFactor() { return this._invokeGetField('roughnessFactor'); }
    get roughnessMapPath() { return this._invokeGetField('roughnessMapPath'); }
    get roughnessMapImage() { return this._invokeGetField('roughnessMapImage'); }
    get glossinessEnable() { return this._invokeGetField('glossinessEnable'); }
    get glossinessFactor() { return this._invokeGetField('glossinessFactor'); }
    get glossinessMapPath() { return this._invokeGetField('glossinessMapPath'); }
    get glossinessMapImage() { return this._invokeGetField('glossinessMapImage'); }
    get normalEnable() { return this._invokeGetField('normalEnable'); }
    get normalFactor() { return this._invokeGetField('normalFactor'); }
    get normalFlipY() { return this._invokeGetField('normalFlipY'); }
    get normalMapPath() { return this._invokeGetField('normalMapPath'); }
    get normalMapImage() { return this._invokeGetField('normalMapImage'); }
    get aoEnable() { return this._invokeGetField('aoEnable'); }
    get aoFactor() { return this._invokeGetField('aoFactor'); }
    get aoMapPath() { return this._invokeGetField('aoMapPath'); }
    get aoMapImage() { return this._invokeGetField('aoMapImage'); }
    get aoOccludeSpecular() { return this._invokeGetField('aoOccludeSpecular'); }
    get cavityEnable() { return this._invokeGetField('cavityEnable'); }
    get cavityFactor() { return this._invokeGetField('cavityFactor'); }
    get cavityMapPath() { return this._invokeGetField('cavityMapPath'); }
    get cavityMapImage() { return this._invokeGetField('cavityMapImage'); }
    get opacityEnable() { return this._invokeGetField('opacityEnable'); }
    get opacityFactor() { return this._invokeGetField('opacityFactor'); }
    get opacityMapPath() { return this._invokeGetField('opacityMapPath'); }
    get opacityMapImage() { return this._invokeGetField('opacityMapImage'); }
    get opacityAlphaInvert() { return this._invokeGetField('opacityAlphaInvert'); }
    get emissiveEnable() { return this._invokeGetField('emissiveEnable'); }
    get emissiveColor() { return this._invokeGetField('emissiveColor'); }
    get emissiveMapPath() { return this._invokeGetField('emissiveMapPath'); }
    get emissiveMapImage() { return this._invokeGetField('emissiveMapImage'); }
    get emissiveFactor() { return this._invokeGetField('emissiveFactor'); }

    set albedoEnable(value) { this._invokeSetField('albedoEnable', value); }
    set albedoColor(value) { 
        this._invokeSetField('albedoColor', value);
    }
    set albedoFactor(value) { value = value / 100;  this._invokeSetField('albedoFactor', value); }
    set specularEnable(value) { this._invokeSetField('specularEnable', value); }
    set specularColor(value) { 
        this._invokeSetField('specularColor', value); 
    }
    set specularFactor(value) { value = value / 100;  this._invokeSetField('specularFactor', value); }
    set metalnessEnable(value) { this._invokeSetField('metalnessEnable', value); }
    set metalnessFactor(value) { 
        value = value / 100;  
        this._invokeSetField('metalnessFactor', value); 
    }
    set specularF0Enable(value) { this._invokeSetField('specularF0Enable', value); }
    set specularF0Factor(value) { 
        value = value / 100;  
        this._invokeSetField('specularF0Factor', value); 
    }
    set roughnessEnable(value) { this._invokeSetField('roughnessEnable', value); }
    set roughnessFactor(value) { value = value / 100;  this._invokeSetField('roughnessFactor', value); }
    set glossinessEnable(value) { this._invokeSetField('glossinessEnable', value); }
    set glossinessFactor(value) { value = value / 100;  this._invokeSetField('glossinessFactor', value); }
    set normalEnable(value) { this._invokeSetField('normalEnable', value); }
    set normalFactor(value) { value = value / 100;  this._invokeSetField('normalFactor', value); }
    set normalFlipY(value) { this._invokeSetField('normalFlipY', value); }
    set aoEnable(value) { this._invokeSetField('aoEnable', value); }
    set aoFactor(value) { value = value / 100;  this._invokeSetField('aoFactor', value); }
    set aoOccludeSpecular(value) { this._invokeSetField('aoOccludeSpecular', value); }
    set cavityEnable(value) { this._invokeSetField('cavityEnable', value); }
    set cavityFactor(value) { value = value / 100;  this._invokeSetField('cavityFactor', value); }
    set opacityEnable(value) { this._invokeSetField('opacityEnable', value); }
    set opacityFactor(value) { value = value / 100;  this._invokeSetField('opacityFactor', value); }
    set opacityAlphaInvert(value) { this._invokeSetField('opacityAlphaInvert', value); }
    set emissiveEnable(value) { this._invokeSetField('emissiveEnable', value); }
    set emissiveColor(value) { this._invokeSetField('emissiveColor', value); }
    set emissiveFactor(value) { this._invokeSetField('emissiveFactor', value); }

    get highlightMaskEnable() { return this._invokeGetField('highlightMaskEnable'); }
    set highlightMaskEnable(value) { this._invokeSetField('highlightMaskEnable', value); }
    get highlightMaskColor() { return this._invokeGetField('highlightMaskColor'); }
    set highlightMaskColor(value) { this._invokeSetField('highlightMaskColor', value); }
    get highlightMaskAlpha() { return this._invokeGetField('highlightMaskAlpha'); }
    set highlightMaskAlpha(value) { this._invokeSetField('highlightMaskAlpha', value); }

    set albedoMapPath(value) { this._invokeSetField('albedoMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.albedoMapName = name[0]); }
    set specularMapPath(value) { this._invokeSetField('specularMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.specularMapName = name[0]);    }
    set metalnessMapPath(value) { this._invokeSetField('metalnessMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.metalnessMapName = name[0]); }
    set specularF0MapPath(value) { this._invokeSetField('specularF0MapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.specularF0MapName = name[0]); }
    set roughnessMapPath(value) { this._invokeSetField('roughnessMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.roughnessMapName = name[0]); }
    set glossinessMapPath(value) { this._invokeSetField('roughnessMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.glossinessMapName = name[0]); }
    set normalMapPath(value) { this._invokeSetField('normalMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.normalMapName = name[0]); }
    set aoMapPath(value) { this._invokeSetField('aoMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.aoMapName = name[0]); }
    set cavityMapPath(value) { this._invokeSetField('cavityMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.cavityMapName = name[0]); }
    set opacityMapPath(value) { this._invokeSetField('opacityMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.opacityMapName = name[0]); }
    set emissiveMapPath(value) { this._invokeSetField('emissiveMapPath', value); this._invokeIFrame('getTextureName', [value]).then(name=>this.emissiveMapName = name[0]); }

    set albedoMapImage(value) { this._invokeSetField('albedoMapImage', value); }
    set specularMapImage(value) { this._invokeSetField('specularMapImage', value); }
    set metalnessMapImage(value) { this._invokeSetField('metalnessMapImage', value); }
    set specularF0MapImage(value) { this._invokeSetField('specularF0MapImage', value); }
    set roughnessMapImage(value) { this._invokeSetField('roughnessMapImage', value); }
    set glossinessMapImage(value) { this._invokeSetField('roughnessMapImage', value); }
    set normalMapImage(value) { this._invokeSetField('normalMapImage', value); }
    set aoMapImage(value) { this._invokeSetField('aoMapImage', value); }
    set cavityMapImage(value) { this._invokeSetField('cavityMapImage', value); }
    set opacityMapImage(value) { this._invokeSetField('opacityMapImage', value); }
    set emissiveMapImage(value) { this._invokeSetField('emissiveMapImage', value); }

    setAlbedoMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.albedoMapName = name[0]), this._invokeFunctionField('setAlbedoMapPath', value)]); }
    setSpecularMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.specularMapName = name[0]), this._invokeFunctionField('setSpecularMapPath', value)]); }
    setMetalnessMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.metalnessMapName = name[0]), this._invokeFunctionField('setMetalnessMapPath', value)]); }
    setSpecularF0MapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.specularF0MapName = name[0]), this._invokeFunctionField('setSpecularF0MapPath', value)]); }
    setRoughnessMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.roughnessMapName = name[0]), this._invokeFunctionField('setRoughnessMapPath', value)]); }
    setGlossinessMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.glossinessMapName = name[0]), this._invokeFunctionField('setGlossinessMapPath', value)]); }
    setNormalMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.normalMapName = name[0]), this._invokeFunctionField('setNormalMapPath', value)]); }
    setAoMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.aoMapName = name[0]), this._invokeFunctionField('setAoMapPath', value)]); }
    setCavityMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.cavityMapName = name[0]), this._invokeFunctionField('setCavityMapPath', value)]); }
    setOpacityMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.opacityMapName = name[0]), this._invokeFunctionField('setOpacityMapPath', value)]); }
    setEmissiveMapPath(value) { return Promise.all([this._invokeIFrame('getTextureName', [value]).then(name=>this.emissiveMapName = name[0]), this._invokeFunctionField('setEmissiveMapPath', value)]); }

    setAlbedoMapImage(value) { return this._invokeFunctionField('setAlbedoMapImage', value); }
    setSpecularMapImage(value) { return this._invokeFunctionField('setSpecularMapImage', value); }
    setMetalnessMapImage(value) { return this._invokeFunctionField('setMetalnessMapImage', value); }
    setSpecularF0MapImage(value) { return this._invokeFunctionField('setSpecularF0MapImage', value); }
    setRoughnessMapImage(value) { return this._invokeFunctionField('setRoughnessMapImage', value); }
    setGlossinessMapImage(value) { return this._invokeFunctionField('setGlossinessMapImage', value); }
    setNormalMapImage(value) { return this._invokeFunctionField('setNormalMapImage', value); }
    setAoMapImage(value) { return this._invokeFunctionField('setAoMapImage', value); }
    setCavityMapImage(value) { return this._invokeFunctionField('setCavityMapImage', value); }
    setOpacityMapImage(value) { return this._invokeFunctionField('setOpacityMapImage', value); }
    setEmissiveMapImage(value) { return this._invokeFunctionField('setEmissiveMapImage', value); }

    setMaterialContext(typeName) {
        if (!this._materialContexts.get(typeName)) {
            this._materialContexts.set(typeName, {});
        }
        this._activeContext = this._materialContexts.get(typeName);
    }
    // name getter
    get albedoMapName() { return this._activeContext._albedoMapName; }
    get specularMapName() { return this._activeContext._specularMapName; }
    get metalnessMapName() { return this._activeContext._metalnessMapName; }
    get specularF0MapName() { return this._activeContext._specularF0MapName; }
    get roughnessMapName() { return this._activeContext._roughnessMapName; }
    get glossinessMapName() { return this._activeContext._roughnessMapName; }
    get normalMapName() { return this._activeContext._normalMapName; }
    get aoMapName() { return this._activeContext._aoMapName; }
    get cavityMapName() { return this._activeContext._cavityMapName; }
    get opacityMapName() { return this._activeContext._opacityMapName; }
    get emissiveMapName() { return this._activeContext._emissiveMapName; }
    // name setter
    set albedoMapName(value) { this._activeContext._albedoMapName = value; }
    set specularMapName(value) { this._activeContext._specularMapName = value; }
    set metalnessMapName(value) { this._activeContext._metalnessMapName = value; }
    set specularF0MapName(value) { this._activeContext._specularF0MapName = value; }
    set roughnessMapName(value) { this._activeContext._roughnessMapName = value; }
    set glossinessMapName(value) { this._activeContext._roughnessMapName = value; }
    set normalMapName(value) { this._activeContext._normalMapName = value; }
    set aoMapName(value) { this._activeContext._aoMapName = value; }
    set cavityMapName(value) { this._activeContext._cavityMapName = value; }
    set opacityMapName(value) { this._activeContext._opacityMapName = value; }
    set emissiveMapName(value) { this._activeContext._emissiveMapName = value; }

    getMaterialInfo(type) {
        return this._invokeFunctionField('getHelperMaterialInfo', type).then(info=>{
            return this.updateNames(info).then(()=>{
                this._appendAdditionalFieldsToInfo(info);
                return info;
            });
        });
    }
    get activeMaterialInfo() {
        return this.activeMaterialType.then(type=>this.getMaterialInfo(type));
    }
    get info() {
        return this._invokeGetField('helperInfo').then(info=>{
            return this.updateNames(info.materialInfo).then(()=>{
                this._appendAdditionalFieldsToInfo(info.materialInfo);
                return info;
            });
        });
    }

    _appendAdditionalFieldsToInfo(info) {
        if (info.type === 'electric') {
            additionalElectricMaterialFields.forEach(field=>info[field] = this[field]);
        } else if (info.type === 'dielectric') {
            additionalDielectricMaterialFields.forEach(field=>info[field] = this[field]);
        }
    }

    updateNames(materialInfo) {
        var fieldsTable = new Map();
        var appendFields = function(key, field) {
            let callbacks = fieldsTable.get(key);
            if (callbacks) {
                callbacks.push(field);
            } else {
                fieldsTable.set(key, [field]);
            }
        }
        // append fields
        allMapFields.forEach(field=>{
            if (materialInfo[`${field}Path`] && !this[`${field}Name`]) { appendFields( materialInfo[`${field}Path`], `${field}Name`); };
        })
        let pathes = [];
        fieldsTable.forEach((value, key)=>{
            pathes.push(key);
        });
        // update fields
        return this._invokeIFrame('getTextureName', pathes).then(result=>{
            for(let i = 0;i<pathes.length;++i) {
                fieldsTable.get(pathes[i]).forEach(field=>{
                    this[field] = result[i];
                })
            }
        });
    }

    getInfo() {
        return Promise.resolve(this.info);
    }

    setActiveMaterialType(type) {
        this.activeMaterialType = type;
    }

    triggerHighlightMask(duration = 2500, startFraction = 0.9, startValue = 0.6) {
        this.highlightMaskEnable = true;
        var that = this;
        animate({
            duration: duration,
            timing(timeFraction) {
                let x = Math.min(Math.max(0.0, startFraction), 1.0 - 0.000001);
                let y = 1.0 - Math.min(Math.max(0.0, startValue), 1.0);
                if (timeFraction < x) {
                    return y;
                } else {
                    let a = (y-1) / (x-1);
                    return (timeFraction - 1) * a + 1;
                }
            },
            draw(progress) {
                that.highlightMaskAlpha = 1.0 - progress;
            }
        });
    }

    getTextureLinkByPath(path) {
        var that = this;
        var extractTextureLinkFromInfo = function(info) {
            var matchedFields = [];
            allMapFields.forEach(field=>{
                if (info[`${field}Path`] === path) {
                    matchedFields.push({id: that.id, type: info.type, map: field});
                }
            });
            return matchedFields;
        };

        if (this.type !== 'compound') {
            return this.getMaterialInfo().then(info=>extractTextureLinkFromInfo(info));
        } else {
            return this.materialTypes.then(types=>{
                let promises = types.map(type=>this.getMaterialInfo(type).then(info=>extractTextureLinkFromInfo(info)));
                return Promise.all(promises);
            }).then(values=>{
                var ret = [];
                values.forEach(value=>{ret = ret.concat(value);});
                return ret;
            });
        }
    }
}

export default ArtHubMaterial;