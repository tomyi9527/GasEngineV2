//gas1
ArtHubGas1Loader = function () {
  GASEngine.GAS1Loader.call(this);
  this._callBack_ = null;
}
ArtHubGas1Loader.prototype = Object.create(GASEngine.GAS1Loader.prototype);
ArtHubGas1Loader.prototype.constructor = ArtHubGas1Loader;

ArtHubGas1Loader.prototype.loadConvertedFiles = function (modelInfo, callback) {
  this._callBack_ = callback;
  let convertedFiles = modelInfo.files;
  let id = modelInfo.id;
  let assetHub = modelInfo.assetHub;

  this._rawDisplayFileList_ = convertedFiles;

  if (this._callBack_) {
    this._callBack_({
      files: this._rawDisplayFileList_
    })
  }
}

// // 暂时不需要一把拉取所有的文件
// ArtHubGas1Loader.prototype.getModelFilesSignature = function(files, modelId, assetHub)
// {
//     if(!files || !Array.isArray(files) || modelId === -1) return Promise.resolve([]);
//     let payload = [], params;
//     files.forEach(
//         v =>
//         {
//             params = { object_id: modelId, object_meta: "display_url", file_name: v};
//             if(v.indexOf(".") !== -1 && v.split(".").pop().indexOf("gz") !== -1)
//             {
//                 params.content_encoding = "gzip";
//             }
//             payload.push(params);
//         }
//     );

//     return ArthubLoader.Instance.getSignature(assetHub, payload)
//         .then(res => {
//             let resList = res.items;
//             let signedFiles = [];
//             if(resList && Array.isArray(resList)) 
//             {
//                 resList.forEach(v =>
//                 {
//                     signedFiles.push(v.signed_url);
//                 });
//             }

//             if(this._callBack_) {
//                 this._callBack_({files: signedFiles})
//             }
//         })
//         .catch(error => {
//             console.log(error);
//         });
// }