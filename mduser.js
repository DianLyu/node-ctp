var pack = require('./package.json');
var ctp = require('./' + [
    'build',
    'node-ctp',
    'v' + pack.version, ['node', 'v' + process.versions.modules, process.platform, process.arch].join('-'),
    'node-ctp.node'
].join('/'));

ctp.settings({ log: true });
var mduser = ctp.createMdUser({
    connect() {
        console.log("on connected");
        this.reqUserLogin('046798', 'dexger', '9999', function(result) {});
    },
    rspUserLogin(requestId, isLast, field, info) {
        console.log("on rspUserLogin", arguments)
        this.subscribeMarketData(['rb1801'], function(result) {
            console.log('subscribeMarketData result:' + result);
        });
    },
    rspSubMarketData(requestId, isLast, field, info) {
        console.log("on rspSubMarketData", arguments)
    },
    rspUnSubMarketData(requestId, isLast, field, info) {
        console.log("on rspUnSubMarketData", arguments)
    },
    rtnDepthMarketData(field) {
        console.log("on rtnDepthMarketData", field);
    },
    rspError(requestId, isLast, info) {
        console.log(requestId, isLast, info)
    }
});

mduser.connect('tcp://180.168.146.187:10010', 'conn/tmp', function(f) {
    console.log(f);
});