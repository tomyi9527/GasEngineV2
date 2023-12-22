// 此部分(color2RGB等)在util出现过。
function color2RGB(color) {
    var sColor = color.toLowerCase();
    var reg = /^#([0-9a-fA-f]{3}|[0-9a-fA-f]{6})$/;
    if (sColor && reg.test(sColor)) {
        if (sColor.length === 4) {
            var sColorNew = "#";
            for (var i=1; i<4; i+=1) {
                sColorNew += sColor.slice(i, i+1).concat(sColor.slice(i, i+1));    
            }
            sColor = sColorNew;
        }
        var sColorChange = [];
        for (var i=1; i<7; i+=2) {
            sColorChange.push(parseInt("0x"+sColor.slice(i, i+2))/255);    
        }
        return sColorChange;
    }
    return sColor;
}

function RGB2Color(colorObj) {
    if(!colorObj) return '';
    const {r, g, b} = colorObj;
    let color = [r, g, b];
    let hex = "#";

    for (var i = 0; i < 3; i++) {
        hex += ("0" + Math.round(Number(color[i]) * 255).toString(16)).slice(-2);
    }
    return hex;
}

const dielectricMaterialFields = [
    "AlbedoEnable",
    "AlbedoColor",
    "AlbedoFactor",
    "AlbedoMapPath",
    "AlbedoMapImage",
    "MetalnessEnable",
    "MetalnessFactor",
    "MetalnessMapPath",
    "MetalnessMapImage",
    "SpecularF0Enable",
    "SpecularF0Factor",
    "SpecularF0MapPath",
    "SpecularF0MapImage",
    "RoughnessEnable",
    "RoughnessFactor",
    "RoughnessMapPath",
    "RoughnessMapImage",
    "GlossinessEnable",
    "GlossinessFactor",
    "GlossinessMapPath",
    "GlossinessMapImage",
    "NormalEnable",
    "NormalFactor",
    "NormalMapPath",
    "NormalMapImage",
    "NormalFlipY",
    "AoEnable",
    "AoFactor",
    "AoMapPath",
    "AoMapImage",
    "AoOccludeSpecular",
    "CavityEnable",
    "CavityFactor",
    "CavityMapPath",
    "CavityMapImage",
    "OpacityEnable",
    "OpacityFactor",
    "OpacityAlphaInvert",
    "OpacityMapPath",
    "OpacityMapImage",
    "EmissiveEnable",
    "EmissiveColor",
    "EmissiveFactor", 
    "EmissiveMapPath",
    "EmissiveMapImage",
    "Culling",
    "HighlightMaskEnable",
    "HighlightMaskColor",
    "HighlightMaskAlpha",
];

const MapName = {
    "albedoMap": "albedoMap",
    "metalnessMap": "metalnessMap",
    "specularF0Map": "specularF0Map",
    "specularMap": "specularMap",
    "roughnessMap": "roughnessMap",
    "glossinessMap": "roughnessMap",
    "normalMap": "normalMap",
    "aoMap": "aoMap",
    "cavityMap": "cavityMap",
    "opacityMap": "transparencyMap",
    "emissiveMap": "emissiveMap",
};

const electricMaterialFields = [
    "AlbedoEnable",
    "AlbedoColor",
    "AlbedoFactor",
    "AlbedoMapPath",
    "AlbedoMapImage",
    "SpecularEnable",
    "SpecularColor",
    "SpecularFactor",
    "SpecularMapPath",
    "SpecularMapImage",
    "RoughnessEnable",
    "RoughnessFactor",
    "RoughnessMapPath",
    "RoughnessMapImage",
    "GlossinessEnable",
    "GlossinessFactor",
    "GlossinessMapPath",
    "GlossinessMapImage",
    "NormalEnable",
    "NormalFactor",
    "NormalMapPath",
    "NormalMapImage",
    "NormalFlipY",
    "AoEnable",
    "AoFactor",
    "AoMapPath",
    "AoMapImage",
    "AoOccludeSpecular",
    "CavityEnable",
    "CavityFactor",
    "CavityMapPath",
    "CavityMapImage",
    "OpacityEnable",
    "OpacityFactor",
    "OpacityAlphaInvert",
    "OpacityMapPath",
    "OpacityMapImage",
    "EmissiveEnable",
    "EmissiveColor",
    "EmissiveFactor",
    "EmissiveMapPath",
    "EmissiveMapImage",
    "Culling",
    "HighlightMaskEnable",
    "HighlightMaskColor",
    "HighlightMaskAlpha",
];

const ConvertFieldNameToMapName = function(field) {
    var indexOfMap = field.indexOf('Map');
    if (indexOfMap !== -1) {
        return field.charAt(0).toLowerCase() + field.slice(1, indexOfMap + 3);
    } else {
        return field;
    }
}

const allowedChoices = ['dielectric', 'electric'];

const cullingChoice = {
    off: 0, // show both side
    onCCW: 1, // cull back
    onCW: 2, // cull front
    onBoth: 3 // cull double side (show nothing)
};

Promise.allSettled = Promise.allSettled || function(promises) {
    return new Promise(function(resolve, reject) {
        if (!Array.isArray(promises)) {
            return reject(
                new TypeError("arguments must be an array")
            );
        }
        let resolvedCounter = 0;
        const promiseNum = promises.length;
        // 统计所有的promise结果并最后返回
        const resolvedResults = new Array(promiseNum);
        for (let i = 0; i < promiseNum; i++) {
            Promise.resolve(promises[i]).then(
                function(value) {
                    resolvedCounter++;
                    resolvedResults[i] = value;
                    if (resolvedCounter == promiseNum) {
                        return resolve(resolvedResults);
                    }
                },
                function(reason) {
                    resolvedCounter++;
                    resolvedResults[i] = reason;
                    if (resolvedCounter == promiseNum) {
                        return resolve(reason);
                    }
                }
            );
        }
    });
};

function encodeImageFileAsURL(image, targetSize = 200) {
    if (image) {
        if (image instanceof Promise) {
            return image.then(res=>encodeImageFileAsURL(res, targetSize));
        }
        var canvas = document.createElement('canvas');
        let ratio = 1.0;
        if (targetSize) {
            ratio = Math.max(1.0, Math.max(image.naturalWidth, image.naturalHeight) / targetSize);
        } 
        canvas.width = image.naturalWidth / ratio;
        canvas.height = image.naturalHeight / ratio;
        var w = canvas.width;
        var h = canvas.height;
        context = canvas.getContext('2d');
        context.drawImage(image, 0, 0, w, h);
    
        var imageData = canvas.toDataURL('image/png');
        return imageData;
    } else {
        return null;
    }
}

// 处理除compound外的material
class ArtHubSingleMaterialAdapter {
    constructor(material, ctx) {
        this._material = material;
        this._iframeContext = ctx;
        this._registerAllFunctions();
    }

    _registerGetFunction(fieldName) {
        var getName = `get${fieldName}`;
        if (!Reflect.has(this, getName) && Reflect.has(this._material, getName)) {
            var targetFunction = Reflect.get(this._material, getName).bind(this._material);
            var getter = function(){return targetFunction();};
            // special case
            // get image
            if (fieldName.endsWith('MapImage'))
                getter = function(){return encodeImageFileAsURL(targetFunction());};
            // get color
            else if (fieldName.endsWith('Color'))
                getter = function(){
                    var color = targetFunction();
                    return RGB2Color({r:color[0], g:color[1], b:color[2]});
                };
            Reflect.set(this, getName, getter);
        }
    }
    _registerSetFunction(fieldName) {
        var setName = `set${fieldName}`;
        if (!Reflect.has(this, setName)) {
            var targetFunction = Reflect.get(this._material, setName);
            if (targetFunction) {
                targetFunction = targetFunction.bind(this._material);
            }
            var setter = function(value){return targetFunction(value);};
            // special case
            // set Map by image
            if (fieldName.endsWith('MapImage'))
                setter = function(imageBase64) {
                    return this._setSomeMapByImage(ConvertFieldNameToMapName(fieldName), imageBase64);
                }
            // set Map by path
            else if (fieldName.endsWith('MapPath'))
                setter = function(imageBase64) {
                    var targetMap = MapName[ConvertFieldNameToMapName(fieldName)];
                    if (targetMap)
                        return this._setSomeMapByPath(targetMap, imageBase64);
                }
            // set color
            else if (targetFunction && fieldName.endsWith('Color'))
                setter = function(color){
                    var colorArray = color2RGB(color);
                    return targetFunction(colorArray[0], colorArray[1], colorArray[2]);
                };
            Reflect.set(this, setName, setter);
        }
    }
    _registerOneField(fieldName) {
        this._registerGetFunction(fieldName);
        this._registerSetFunction(fieldName);
    }

    // generate get and set functions
    _registerAllFunctions() {
        var fields = [];
        if (this.getType() === 'dielectric') {
            fields = dielectricMaterialFields;
        } else if (this.getType() === 'electric') {
            fields = electricMaterialFields;
        }
        fields.forEach(field=>this._registerOneField(field));
    }

    getType() {
        return this._material.typeName;
    }

    // (TODO beanpliu): initializatoin for editor   should move to editor
    enableMaterialBasicProperties() {
        if ('dielectric' === this.getType()) {
            this.setMetalnessEnable(true);
            this.setSpecularF0Enable(true);
            this.setAlbedoEnable(true);
        } else if ('electric' === this.getType()) {
            this.setSpecularEnable(true);
            this.setAlbedoEnable(true);
        }
    }

    // map & texture utility
    _unloadMaterialMap(mapName) {
        var material = this._material;
        if (material[mapName]) {
            GASEngine.Resources.Instance.unloadTextureOnMaterial(material[mapName]);
            return true;
        } else {
            return false;
        }
    }

    _changeMaterialMap(mapName, newPath) {
        var material = this._material;
        // unloadTexture
        if (material[mapName]){
            GASEngine.Resources.Instance.unloadTextureOnMaterial(material[mapName]);
        } else {
            material[mapName] = GASEngine.MaterialMapFactory.Instance.create();
        }
        // loadTexture
        material[mapName].texture = newPath;
        // GASEngine.Resources.Instance.loadTextureOnMaterial(material[mapName]);
        return new Promise((resolve, reject)=>{
            GASEngine.Resources.Instance.loadTextureOnMaterial(material[mapName], resolve, reject);
        });
    }

    _changeMaterialMapByBase64Image(mapName, base64Image) {
        var material = this._material;
        // unloadTexture
        if (material[mapName]){
            GASEngine.Resources.Instance.unloadTextureOnMaterial(material[mapName]);
        } else {
            material[mapName] = GASEngine.MaterialMapFactory.Instance.create();
        }
        // loadTexture
        material[mapName].texture = `base64Image_${GASEngine.generateUUID()}`; // virtual name(?)
        // GASEngine.Resources.Instance.loadTextureOnMaterial(material[mapName]);
        return new Promise((rs,rj)=>{
            var image = new Image();
            image.onload = function(){
                material[mapName].webglTexture = GASEngine.WebGLTextureManager.Instance.createTextureFromImage(image, false); // dontNeedPot = false
                rs();
            }
            image.onabort = rj;
            image.src = base64Image;
        });
    }

    _setSomeMapByPath(mapName, value) {
        // value should be empty(null) or '@@depot/id/meta'
        if (value)
            return this._changeMaterialMap(mapName, value);
        else
            return Promise.resolve(this._unloadMaterialMap(mapName));
    }

    _setSomeMapByImage(mapName, value) {
        if (value)
            return this._changeMaterialMapByBase64Image(mapName, value);
        else
            return Promise.resolve(this._unloadMaterialMap(mapName));
    }

    // helper
    getHelperMaterialInfo() {
        // takeout fields
        var setFieldValue = function(obj, field, value) {
            if(value instanceof Promise) {
                return value.then((res) => {
                    obj[field] = res;
                }).catch(() => {
                    obj[field] = null;
                })
            } else {
                obj[field] = value;
                return null;
            }
        }
        var promises = [];
        var materialInfoObject = {};

        // data source material: mat
        var actualType = this.getType();
        var targetFields;
        if (actualType === 'dielectric') {
            targetFields = dielectricMaterialFields;
        } else if (actualType === 'electric') {
            targetFields = electricMaterialFields;
        }
        targetFields.forEach(field=>{
            var objField = field.charAt(0).toLowerCase() + field.slice(1)
            var thisField = `get${field}`;
            var promise = setFieldValue(materialInfoObject, objField, Reflect.get(this, thisField)());
            if (promise)
                promises.push(promise);
        });
        materialInfoObject['type'] = actualType;
        if (promises.length > 0) {
            materialInfoObject['_waitMessage'] = MessageType.MaterialLoaded;
            var ctx = this._iframeContext;
            var matId = this._material.uniqueID;
            Promise.allSettled(promises).then(()=>{
                ctx._returnParent(-1, MessageType.MaterialLoaded, matId);
            });
        }
        return materialInfoObject;
    }

    getHelperInfo() {
        const name = this._material.name;
        const uniqueId = this._material.uniqueID;
        const type = this.getType();
        const materialInfo = this.getHelperMaterialInfo();

        return {
            name,
            uniqueId,
            type,
            activeMaterialType,
            materialTypes,
            materialInfo
        };
    }
}

class ArtHubCompoundMaterialAdapter {
    constructor(material, ctx) {
        this._material = material;
        this._iframeContext = ctx;
        this._refreshActiveMaterial();
        this._registerAllFunctions();
    }

    _registerGetFunction(fieldName) {
        var getName = `get${fieldName}`;
        var activeMaterial = this.getActiveMaterial();
        if (!Reflect.has(this, getName) && Reflect.has(activeMaterial, getName)) {
            var targetFunction = Reflect.get(activeMaterial, getName).bind(activeMaterial);
            var getter = function(){ return targetFunction();};
            // no special case
            Reflect.set(this, getName, getter);
        }
    }
    _registerSetFunction(fieldName) {
        var setName = `set${fieldName}`;
        var activeMaterial = this.getActiveMaterial();
        if (!Reflect.has(this, setName) && Reflect.has(activeMaterial, setName)) {
            var targetFunction = Reflect.get(activeMaterial, setName).bind(activeMaterial);
            var setter = function(value){return targetFunction(value);};
            // no special case
            Reflect.set(this, setName, setter);
        }
    }
    _registerOneField(fieldName) {
        this._registerGetFunction(fieldName);
        this._registerSetFunction(fieldName);
    }

    // generate get and set functions
    _registerAllFunctions() {
        var fields = dielectricMaterialFields.concat(
            electricMaterialFields.filter(x => !dielectricMaterialFields.includes(x))
        );
        fields.forEach(field=>this._registerOneField(field));
    }

    getType() {
        return this._material.typeName;
    }

    getActiveMaterialType() {
        return this._material.getActiveMaterial().typeName;
    }

    getMaterialTypes() {
        var choices = [];
        this._material.materials.forEach(mat=>choices.push(mat.typeName));
        // 目前仅允许使用allowedChoices里的这两种
        return choices.filter(x => allowedChoices.includes(x));
    }

    // editable properties
    getActiveMaterial() {
        // type check
        return this._activeMaterial;
    }

    setActiveMaterialType(type) {
        this._material.setActiveMaterial(type);
        this._refreshActiveMaterial();
    }

    _refreshActiveMaterial() {
        var activeMaterial = this._material.getActiveMaterial();
        if (!activeMaterial) {
            return false;
        } else {
            this._activeMaterial = new ArtHubSingleMaterialAdapter(activeMaterial, this._iframeContext);
            return true;
        }
    }

    // (TODO beanpliu): initializatoin for editor   should move to editor
    enableMaterialBasicProperties() {
        this._material.materials.forEach(v=>new ArtHubMaterialAdapter(v, this._iframeContext).getAdapter().enableMaterialBasicProperties());
    }

    // helper
    getHelperMaterialInfo(type) {
        var oldType = this.getActiveMaterialType();
        this.setActiveMaterialType(type);
        var targetMaterial = this.getActiveMaterial();
        this.setActiveMaterialType(oldType);

        return targetMaterial.getHelperMaterialInfo();
    }

    getHelperActiveMaterialInfo() {
        return this.getHelperMaterialInfo(this.getActiveMaterialType());
    }

    getHelperInfo() {
        const name = this._material.name;
        const uniqueId = this._material.uniqueID;
        const type = this.getType();
        const materialTypes = this.getMaterialTypes();
        const activeMaterialType = this.getActiveMaterialType();
        const materialInfo = this.getHelperActiveMaterialInfo();

        return {
            name,
            uniqueId,
            type,
            activeMaterialType,
            materialTypes,
            materialInfo
        };
    }
}


class ArtHubMaterialAdapter {
    constructor(material, ctx) {
        this._iframeContext = ctx;
        if (material.typeName === 'compound') {
            this._adapter = new ArtHubCompoundMaterialAdapter(material, ctx);
        } else {
            this._adapter = new ArtHubSingleMaterialAdapter(material, ctx);
        }
    }

    getAdapter() {
        return this._adapter;
    }
}