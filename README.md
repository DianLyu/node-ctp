node-ctp
========

高性能期货程序化交易框架

##Why?

Shif发布的CTP接口是基于C++语言开发的，我们使用CTP开发交易策略软件时，一般也使用C++语言。
我本人觉得这样不是很方便，封装成Node.js模块，我考虑基于以下两点：

    1. 使用Javascript极大的方便了交易策略的编写。
    2. 提供了一个高性能的并发框架，并支持多账户交易。

##Demo

我们可以这样调用CTP接口
行情订阅调用示例

```javascript

var ctp = require('node-ctp');
ctp.settings({log:true});
var mduser = ctp.createMduser({
    connect() {
        console.log("on connected");
        this.reqUserLogin('', '', '', function(result) {});
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

mduser.connect('ctp url', 'conn/tmp', function (result){
    console.log(result);

});


```
交易接口示例

```javascript
//confirm

ctp = require('node-ctp');
ctp.settings({ log: true});
var trader = ctp.createTrader({
    connect() {
        console.log("on connected");
        this.reqUserLogin('', '', '', function(result, iRequestID) {
            console.log('login return val is ' + result);
        });
    },
    rspUserLogin(requestId, isLast, field, info) {

        console.log(JSON.stringify(field));
        console.log(info);
        this.reqQrySettlementInfo('', '', '', function(result, iRequestID) {
            console.log('settlementinfo return val is ' + result);

        });
        var tradingDay = this.getTradingDay();
        console.log(tradingDay);
    },
    rspInfoconfirm(requestId, isLast, field, info) {

        console.log()

    },
    rqSettlementInfo(requestId, isLast, field, info) {
        console.log('rqsettlementinfo callback');
        console.log(field);
        console.log(info);

    },
    rtnOrder(field) {
        console.log(field);
    },
    rspError(requestId, isLast, field) {
        console.log(JSON.stringify(field));

    }
});

trader.connect('',undefined,0,1,function(result){
    console.log('connect return val is '+result);
});


```







