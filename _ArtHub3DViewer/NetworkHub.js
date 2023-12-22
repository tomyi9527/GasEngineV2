/**
 * API 路径规范
 * 常规路径拼接方式:
 * http(s)://host/depotSchema/serviceModule/apiPath
 * ws(s)://host/depotSchema/serviceModule/apiPath
 * client proxy拼接方式:
 * http(s)://clientProxyOA(QQ)Host/depotSchema/serviceModule/apiPath
 * 注意：admin 服务的API不遵守相关规则
 */

function convertToLink(linkString) {
  if (!linkString || typeof linkString !== 'string') {
    return null;
  }
  const link = document.createElement('a');
  link.href = linkString;
  return link;
}

/**
 * assume there will be a domains variable on window
 */
function getServiceDomain() {
  const serviceDomain = {
    default: window.location.host,
    apis: window.location.host,
    clientProxy: window.location.host,
  };
  // search an entry where current host exists
  if (window.domains && typeof window.domains === 'object') {
    Object.keys(window.domains).forEach((env) => {
      const config = window.domains[env];
      if (!config) return;
      const link = config.finder ? convertToLink(config.finder.default) : null;
      if (link && link.host === window.location.host) {
        serviceDomain.default = config.default.default || serviceDomain.default;
        serviceDomain.apis = config.apis.default || serviceDomain.apis;
        serviceDomain.clientProxy = config.client_proxy.default || serviceDomain.clientProxy;
        return;
      }
    });
  }
  // always convert to domain without protocol
  Object.keys(serviceDomain).forEach((k) => {
    if (serviceDomain[k].indexOf('http') === 0) {
      serviceDomain[k] = convertToLink(serviceDomain[k]).host;
    }
  });
  return serviceDomain;
}

const domains = getServiceDomain();
const WBP_DEPOT_SCHEMA = 'outsourcing';

const ServiceModule = {
  DATA_SERVICE: 'data',
  EXPLORER_SERVICE: 'explorer',
  GATEWAY_SERVICE: 'gateway',
  EDITOR_SERVICE: 'editor',
  CLIENT_PROXY_SERVICE: 'client_proxy',
  MESSAGE_SERVICE: 'message',
  ADMIN_SERVICE: 'admin',
  CONVARTTING_SERVICE: 'converting',
  AI_SERVICE: 'ai',
  TSA_SERVICE: 'tsa',
  ACCOUNT_SERVICE: 'account',
  WBP_SERVICE: 'wbp',
};

const ApiMapConfig = {};

// explorer api path
ApiMapConfig[ServiceModule.EXPLORER_SERVICE] = [
  'openapi/v2/core/get-exploration-set-item-count',
  'openapi/v2/core/get-exploration-set-directory-count',
  'openapi/v2/core/get-exploration-item-id-in-range',
  'openapi/v2/core/get-pan-search-item-count',
  'openapi/v2/core/get-pan-search-item-id-in-range',
];

// client proxy api path
ApiMapConfig[ServiceModule.CLIENT_PROXY_SERVICE] = [
  'openapi/v2/client_proxy/get-upload-signature',
  'openapi/v2/client_proxy/unzip-and-upload',
  'openapi/v2/client_proxy/zip-and-download',
  'openapi/v2/core/download-file-by-hash',
];

// gateway api path
ApiMapConfig[ServiceModule.GATEWAY_SERVICE] = [
  'openapi/v2/core/login',
  'openapi/v2/core/logout',
  'openapi/v2/core/retrieve-password',
  'openapi/v2/core/get-depot-count',
  'openapi/v2/core/get-depot-unique-id-in-range',
  'openapi/v2/core/get-depot-detail-by-unique-id',
  'openapi/v2/core/update-depot-by-unique-id',
  'openapi/v2/core/update-account-by-account-name',
  'openapi/v2/core/update-account-password-by-account-name',
  'openapi/v2/core/get-object-meta-upload-signature',
  'openapi/v2/core/retrieve-password-through-mail',
  'openapi/v2/core/get-captcha',
  'openapi/v2/core/get-ticket',
  'openapi/v2/core/register-account',
];

// account service
ApiMapConfig[ServiceModule.ACCOUNT_SERVICE] = [
  'openapi/v3/core/logout',
  'openapi/v3/core/get-account-detail-by-account-name',
  'openapi/v3/core/update-account-by-account-name',
  'openapi/v3/core/get-last-access-location-by-account',
  'openapi/v3/core/set-last-access-location-by-account',
];

// admin api path
ApiMapConfig[ServiceModule.ADMIN_SERVICE] = [
  'openapi/v1/core/send-feedback',
  'openapi/v1/core/report-page-router',
];

ApiMapConfig[ServiceModule.MESSAGE_SERVICE] = [
  'ws',
];

ApiMapConfig[ServiceModule.EDITOR_SERVICE] = [
  'openapi/v2/core/update-json-value-by-path',
  'openapi/v2/core/create-json-files',
];

ApiMapConfig[ServiceModule.TSA_SERVICE] = [
  'openapi/v2/core/get-tsa-task',
  'openapi/v2/core/add-tsa-task',
];

ApiMapConfig[ServiceModule.WBP_SERVICE] = [
  'openapi/v1/core/get-download-sign',
  'openapi/v1/core/get-file-list-by-id-meta',
  'openapi/v1/core/get-node-brief-by-id',
];

// ai service and converting service not used
ApiMapConfig[ServiceModule.AI_SERVICE] = [];
ApiMapConfig[ServiceModule.CONVARTTING_SERVICE] = [];

// default data service api
ApiMapConfig[ServiceModule.DATA_SERVICE] = [];


NetworkHub = function () {
  if (NetworkHub.Instance) {
    return NetworkHub.Instance;
  }

  this._tokenKey_ = "publictoken";
  this._token_ = "";
  this._macAddress_ = '';
  this._host_ = '';
  this._assetHub_ = '';
  this._wbpThingCode_ = '';
  NetworkHub.Instance = this;
}

NetworkHub.prototype.constructor = NetworkHub;

NetworkHub.prototype.init = function (options = {}) {
  this._host_ = options.host || window.location.host;
  this._token_ = options.token || '';
  this._macAddress_ = options.macAddress || '';
  this._assetHub_ = options.assetHub || '';
  this._wbpThingCode_ = options.thingCode || '';
}

//util
NetworkHub.prototype._setHeader = function (request, headerArr, withCredentials = true) {
  let headers = [];
  headers.push({
    key: "X-Requested-With",
    value: "XMLHttpRequest"
  });
  if (this._token_ !== "") {
    headers.push({
      key: this._tokenKey_,
      value: this._token_
    });
  }
  if (this._macAddress_ !== "") {
    headers.push({
      key: "MacAddress",
      value: this._macAddress_
    });
  }
  if (headerArr && headerArr.length > 0) {
    headers = [
      ...headers,
      ...headerArr
    ];
  }

  request.withCredentials = withCredentials;

  for (var i = 0, len = headers.length; i < len; i++) {
    request.setRequestHeader(headers[i].key, headers[i].value);
  }
  return request;
};

NetworkHub.prototype._generateCompleteAPIPath = function (path, schema, protocol = null) {
  const proto = protocol || window.location.protocol;

  const apisHost = domains.apis;
  const clientProxyHost = domains.clientProxy;
  // gateway api (schema is gateway)
  if (ApiMapConfig[ServiceModule.GATEWAY_SERVICE].indexOf(path) !== -1 || schema === ServiceModule.GATEWAY_SERVICE) {
    return `${proto}//${apisHost}/${ServiceModule.GATEWAY_SERVICE}/${ServiceModule.GATEWAY_SERVICE}/${path}`;
  }
  // account api
  if (ApiMapConfig[ServiceModule.ACCOUNT_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${apisHost}/${ServiceModule.ACCOUNT_SERVICE}/${ServiceModule.ACCOUNT_SERVICE}/${path}`;
  }
  // explorer api
  if (ApiMapConfig[ServiceModule.EXPLORER_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${apisHost}/${schema}/${ServiceModule.EXPLORER_SERVICE}/${path}`;
  }
  // message api
  if (ApiMapConfig[ServiceModule.MESSAGE_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${apisHost}/${schema}/${ServiceModule.MESSAGE_SERVICE}/${path}`;
  }
  // client proxy api
  if (ApiMapConfig[ServiceModule.CLIENT_PROXY_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${clientProxyHost}/${schema}/${ServiceModule.CLIENT_PROXY_SERVICE}/${path}`;
  }
  // editor api
  if (ApiMapConfig[ServiceModule.EDITOR_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${apisHost}/${schema}/${ServiceModule.EDITOR_SERVICE}/${path}`;
  }
  // tsa api
  if (ApiMapConfig[ServiceModule.TSA_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${apisHost}/${schema}/${ServiceModule.TSA_SERVICE}/${path}`;
  }
  // wbp api
  if (ApiMapConfig[ServiceModule.WBP_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${apisHost}/${ServiceModule.WBP_SERVICE}/${ServiceModule.WBP_SERVICE}/${path}`;
  }
  // admin api 不遵守相关规则，不分库，直接拼接
  if (ApiMapConfig[ServiceModule.ADMIN_SERVICE].indexOf(path) !== -1) {
    return `${proto}//${apisHost}/${ServiceModule.ADMIN_SERVICE}/${ServiceModule.ADMIN_SERVICE}/${path}`;
  }
  // default data api, replace /v1/ with /v2/
  return `${proto}//${apisHost}/${schema}/${ServiceModule.DATA_SERVICE}/${path}`;
}

NetworkHub.prototype.httpRequest = function (url, method, data = null, headers = {}, withCredentials = true) {
  return new Promise((resolve, reject) => {
    var xhttp = new XMLHttpRequest();
    xhttp.open(method, url, true);
    const headersArr = [];
    for (let key in headers) {
      headersArr.push({
        key: key,
        value: headers[key]
      });
    }
    this._setHeader(xhttp, headersArr, withCredentials);
    xhttp.onreadystatechange = function () {
      if (this.readyState === 4) {
        if (this.status === 200) {
          try {
            let result = JSON.parse(this.responseText);
            resolve(result);
          } catch {
            reject({
              message: 'json parse error'
            })
          }
        } else {
          reject({
            message: `网络请求错误：${this.status}`
          });
        }
      }
    };
    xhttp.send(data);
  });
};
NetworkHub.prototype.defaultHandler = function (resultInfo) {
  if (resultInfo.code === 0) {
    return resultInfo.result || {};
  } else {
    return null;
  }
};

//apis
NetworkHub.prototype.getFile = function (url) {
  return this.httpRequest(url, 'GET', null, {}, false);
};

NetworkHub.prototype.getSignature = function (assetHub, payload) {
  if (assetHub === WBP_DEPOT_SCHEMA) {
    return this.getWbpSignature(assetHub, payload);
  }
  let baseURL = this._generateCompleteAPIPath('openapi/v2/core/get-download-signature', assetHub);
  payload.forEach(v => {
    let filename = v.file_name;
    if (filename && filename.indexOf('.') !== -1 && filename.split('.').pop().indexOf('gz') !== -1) {
      v.content_encoding = 'gzip';
    }
  });
  payload = JSON.stringify(payload);
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "Application/json"
    })
    .then(this.defaultHandler)
    .then(resInfo => {
      if (!resInfo) {
        return Promise.reject({
          message: '完整路径获取错误'
        });
      } else {
        let resList = resInfo.items;
        let res = resList[0];
        let signedUrl = res.signed_url;
        return signedUrl;
      }
    });
};

NetworkHub.prototype.getFileListByIDMeta = function (assetHub, assetId) {
  if (assetHub === WBP_DEPOT_SCHEMA) {
    return this.getWbpFileListByIDMeta(assetHub, assetId);
  }
  let baseURL = this._generateCompleteAPIPath('openapi/v2/core/get-file-list-by-id-meta', assetHub);
  let payload = {
    "object_id": assetId,
    "object_meta": "display_url"
  }
  payload = JSON.stringify([payload]);
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "Application/json"
    })
    .then(this.defaultHandler)
    .then(resInfo => {
      let resList = resInfo ? resInfo.items : null;
      let res = resList ? resList[0] : {};
      let files = res.files || [];
      return files;
    })
};

NetworkHub.prototype.getAssetDetailByID = function (assetHub, idArray, assetMetaArray) {
  if (assetHub === WBP_DEPOT_SCHEMA) {
    return this.getWbpAssetDetailByID(assetHub, idArray);
  }
  let baseURL = this._generateCompleteAPIPath('openapi/v2/core/get-asset-detail-by-id', assetHub);
  let payload = {
    ids: idArray,
    meta: assetMetaArray
  };
  payload = JSON.stringify(payload)
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "Application/json"
    })
    .then(this.defaultHandler)
    .then(resInfo => {
      if (!resInfo) {
        return [];
      }
      let resList = resInfo.items;
      if (!resList || !Array.isArray(resList)) {
        return [];
      }
    });
};

NetworkHub.prototype.updateJsonValueByPath = function (params, assetHub) {
  let baseURL = this._generateCompleteAPIPath('openapi/v2/core/update-json-value-by-path', assetHub);
  let payload = JSON.stringify(params);
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "application/json"
    })
    .then(this.defaultHandler);
}

NetworkHub.prototype.createFile = function (params, assetHub) {
  let baseURL = this._generateCompleteAPIPath('openapi/v2/core/create-json-files', assetHub);
  let payload = JSON.stringify(params)
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "application/json"
    })
    .then(this.defaultHandler);
}

//wbp
NetworkHub.prototype.getWbpSignature = function (assetHub, payload) {
  let baseURL = this._generateCompleteAPIPath('openapi/v1/core/get-download-sign', assetHub);
  payload.forEach(v => {
    let filename = v.file_name;
    if (filename && filename.indexOf('.') !== -1 && filename.split('.').pop().indexOf('gz') !== -1) {
      v.content_encoding = 'gzip';
    }
  });
  payload = {
    thing_code: this._wbpThingCode_,
    items: payload
  };
  payload = JSON.stringify(payload);
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "application/json"
    })
    .then(this.defaultHandler)
    .then(resInfo => {
      if (!resInfo) {
        return Promise.reject({
          message: '完整路径获取错误'
        });
      } else {
        let res = resInfo[0];
        let signedUrl = res.signed_url;
        return signedUrl;
      }
    });
};

NetworkHub.prototype.getWbpFileListByIDMeta = function (assetHub, assetId) {
  let baseURL = this._generateCompleteAPIPath('openapi/v1/core/get-file-list-by-id-meta', assetHub);
  let payload = {
    thing_code: this._wbpThingCode_,
    asset_id: assetId,
    meta: "display_url",
  }
  payload = JSON.stringify(payload);
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "application/json"
    })
    .then(this.defaultHandler)
    .then(res => {
      return res ? res.files || [] : [];
    });
};

NetworkHub.prototype.getWbpAssetDetailByID = function (assetHub, idArray) {
  let baseURL = this._generateCompleteAPIPath('openapi/v1/core/get-node-brief-by-id', assetHub);
  let payload = {
    thing_code: this._wbpThingCode_,
    node_ids: idArray,
  };
  payload = JSON.stringify(payload);
  return this.httpRequest(baseURL, 'POST', payload, {
      "content-type": "application/json"
    })
    .then(this.defaultHandler);
};