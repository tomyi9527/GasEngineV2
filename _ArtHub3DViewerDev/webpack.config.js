const path = require('path');

module.exports = env => {
  const isProduction = env ? env.production === 'true' : true;
  console.log('Production: ', isProduction);

  return {
    entry: './src/index.js',
    mode: isProduction ? 'production' : 'development',
    devtool: isProduction ? 'hidden-source-map' : 'inline-source-map',
    output: {
      path: isProduction ? path.resolve(__dirname, './../_ArtHub3DViewer/') : path.resolve(__dirname, './dist'),
      filename: 'arthub-3d-viewer.js',
      library: 'GasEngine',
      libraryExport: 'default',
    },
  }
};