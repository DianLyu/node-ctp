var pack = require('./package.json');
var ctp =  require('./' + [
    'build',
    'shifctp',
    'v' + pack.version,
    ['node', 'v' + process.versions.modules, process.platform, process.arch].join('-'),
    'shifctp.node'
  ].join('/'));
ctp.settings({ log: true});
var mduser = ctp.createMdUser();

mduser.on("connect",function(result){
    console.log("on connected");
    mduser.reqUserLogin('046798','dexger','9999',function(result){
    });

});

mduser.on("rspUserLogin", function (requestId, isLast, field, info) {
 
    mduser.subscribeMarketData(['IF1503'], function (result) {
             console.log('subscribeMarketData result:' + result);
    });
});

mduser.on('rspSubMarketData', function (requestId, isLast, field, info) {
    
        console.log("rspSubMarketData");
    });

mduser.on('rspUnSubMarketData', function (requestId, isLast, field, info) {
    
    mduser.disconnect();

});

mduser.on('rtnDepthMarketData', function (field) {
    
    console.log(JSON.stringify(field));

});

mduser.on('rspError', function (requestId, isLast, info) {
    
    console.log("repError");

});

mduser.connect('tcp://180.168.146.187:10010', 'conn/tmp', function (f) {

    console.log(f);
});
