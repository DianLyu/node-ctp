'use strict';
var pack = require('./package.json');
var ctp =  require('./' + [
    'build',
    'node-ctp',
    'v' + pack.version,
    ['node', 'v' + process.versions.modules, process.platform, process.arch].join('-'),
    'node-ctp.node'
  ].join('/'));
module.exports = ctp;