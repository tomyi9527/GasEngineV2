import ArtHubBase from './ArtHubBase.js';

const BackgroundList = [
    {id: '01_attic_room_with_windows', name: '01_attic_room_with_windows', url: 'system/backgroundCubes/01_attic_room_with_windows/thumbnail.jpg'},
    {id: '02_Ferry_To_Stockholm', name: '02_Ferry_To_Stockholm', url: 'system/backgroundCubes/02_Ferry_To_Stockholm/thumbnail.jpg'},
    {id: '03_HDRI_Sky_1553', name: '03_HDRI_Sky_1553', url: 'system/backgroundCubes/03_HDRI_Sky_1553/thumbnail.jpg'},
    {id: '04_Empty_Night_Side_Walk_In_Small_Town', name: '04_Empty_Night_Side_Walk_In_Small_Town', url: 'system/backgroundCubes/04_Empty_Night_Side_Walk_In_Small_Town/thumbnail.jpg'},
    {id: '05_Stairs_At_Church', name: '05_Stairs_At_Church', url: 'system/backgroundCubes/05_Stairs_At_Church/thumbnail.jpg'},
    {id: '06_The_Interior_Of_The_Romanesque_Rotunda', name: '06_The_Interior_Of_The_Romanesque_Rotunda', url: 'system/backgroundCubes/06_The_Interior_Of_The_Romanesque_Rotunda/thumbnail.jpg'},
    {id: '07_Wooden_Pier_At_Morning', name: '07_Wooden_Pier_At_Morning', url: 'system/backgroundCubes/07_Wooden_Pier_At_Morning/thumbnail.jpg'},
    {id: '08_Winter_Night_Hill', name: '08_Winter_Night_Hill', url: 'system/backgroundCubes/08_Winter_Night_Hill/thumbnail.jpg'},
    {id: '09_Glazed_Patio', name: '09_Glazed_Patio', url: 'system/backgroundCubes/09_Glazed_Patio/thumbnail.jpg'},
    {id: '10_Small_Waterfall_In_Park', name: '10_Small_Waterfall_In_Park', url: 'system/backgroundCubes/10_Small_Waterfall_In_Park/thumbnail.jpg'},
    {id: '11_Morning_On_The_SquareIn_Resort_Town', name: '11_Morning_On_The_SquareIn_Resort_Town', url: 'system/backgroundCubes/11_Morning_On_The_SquareIn_Resort_Town/thumbnail.jpg'},
    {id: '12_Small_Apartment', name: '12_Small_Apartment', url: 'system/backgroundCubes/12_Small_Apartment/thumbnail.jpg'},
    {id: '13_Abandoned_Sanatorium', name: '13_Abandoned_Sanatorium', url: 'system/backgroundCubes/13_Abandoned_Sanatorium/thumbnail.jpg'},
    {id: '14_Blue_Hour_At_Pier', name: '14_Blue_Hour_At_Pier', url: 'system/backgroundCubes/14_Blue_Hour_At_Pier/thumbnail.jpg'},
    {id: '15_Urban_Exploring_Interior', name: '15_Urban_Exploring_Interior', url: 'system/backgroundCubes/15_Urban_Exploring_Interior/thumbnail.jpg'},
];

const EnvironmentIntensity = [0, 50];

class ArtHubEnvironmetalLight extends ArtHubBase{
    constructor(options) {
        super(options);
    }

    _setEnvironment(params) {
        return this._invokeIFrame('setEnvironmentInfo', params);
    }

    _getIntensityValueByPercent(percent) {
        return (EnvironmentIntensity[1] - EnvironmentIntensity[0]) * percent / 100 + EnvironmentIntensity[0];
    }

    _getPercentByIntensityValue(value) {
        return (value - EnvironmentIntensity[0]) * 100 / (EnvironmentIntensity[1] - EnvironmentIntensity[0]);
    }

    _getOrientationValueByPercent(percent) {
        return  Math.round(360 * percent / 100);
    }

    _getPercentByOrientationValue(value) {
        return Math.round(value/360 * 100);
    }
    
    _setBackground(params) {
        return this._invokeIFrame('setBackground', params);
    }

    //apis
    /** 
     * @param
     * @param {String} environmentName
     * @param {Number} orientation
     * @param {Number} intensity
     * */
    getEnvironmentInfo() {
        return this._invokeIFrame('getEnvironmentInfo')
            .then(res => {
                res.intensity = this._getPercentByIntensityValue(res.intensity);
                res.environmentObj = BackgroundList.find(e => e.id === res.environmentName);
                res.orientation = this._getPercentByOrientationValue(res.orientation);
                return res;
            });
    }

    set environmentName(value) {
        this._setEnvironment({environmentName: value});
    }

    set intensity(value) {
        const intensityValue = this._getIntensityValueByPercent(value);
        this._setEnvironment({intensity: intensityValue});
        this._setBackground({lightBrightness: intensityValue});
    }

    set orientation(value) {
        const orientValue = this._getOrientationValueByPercent(value);
        this._setEnvironment({orientation: orientValue});
    }

    getEnvironmentList() {
        return this._invokeIFrame('getRootDirectory')
            .then((rootDirectory) => {
                const backgroundList = [...BackgroundList];
                backgroundList.forEach(e => e.url = `${rootDirectory}${e.url}`);
                return Promise.resolve(backgroundList);
            });
    }
}

export default ArtHubEnvironmetalLight;