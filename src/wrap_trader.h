#ifndef N_TRADER_H_
#define N_TRADER_H_

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <node.h>
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include <uv.h>
#include "uv_trader.h"
#include <node_object_wrap.h>

using namespace v8;

extern bool islog;
extern void logger_cout(const char* content);
extern std::string to_string(int val);
extern std::string charto_string(char val);

class WrapTrader : public node::ObjectWrap {
public:
	WrapTrader(const FunctionCallbackInfo<Value>& args);
	~WrapTrader(void);

	///连接前置机
	static void Connect(const FunctionCallbackInfo<Value>& args);	
	///注册事件
	static void On(const FunctionCallbackInfo<Value>& args);
	///用户登录请求
	static void ReqUserLogin(const FunctionCallbackInfo<Value>& args);
	///登出请求 
	static void ReqUserLogout(const FunctionCallbackInfo<Value>& args);
	///投资者结算结果确认
	static void ReqSettlementInfoConfirm(const FunctionCallbackInfo<Value>& args);
	///请求查询合约
	static void ReqQryInstrument(const FunctionCallbackInfo<Value>& args);
	///请求查询资金账户
	static void ReqQryTradingAccount(const FunctionCallbackInfo<Value>& args);
	///请求查询投资者持仓
	static void ReqQryInvestorPosition(const FunctionCallbackInfo<Value>& args);
	///持仓明细
	static void ReqQryInvestorPositionDetail(const FunctionCallbackInfo<Value>& args);
	///报单录入请求
	static void ReqOrderInsert(const FunctionCallbackInfo<Value>& args);
	///报单操作请求
	static void ReqOrderAction(const FunctionCallbackInfo<Value>& args);
	///请求查询合约保证金率 
	static void ReqQryInstrumentMarginRate(const FunctionCallbackInfo<Value>& args);
	///请求查询行情 
	static void ReqQryDepthMarketData(const FunctionCallbackInfo<Value>& args);
	///请求查询投资者结算结果 
	static void ReqQrySettlementInfo(const FunctionCallbackInfo<Value>& args);
	///删除接口对象
	static void Disposed(const FunctionCallbackInfo<Value>& args);
	//对象初始化
	static void Init(Handle<Object> args);
    static void GetTradingDay(const FunctionCallbackInfo<Value>& args);
	static Persistent<Function> constructor;
	void FunCallback(CbRtnField *data);
	void FunRtnCallback(int result, void* baton);
private:
	static void initEventMap();	
	void pkg_cb_userlogin(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_userlogout(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_confirm(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_orderinsert(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_errorderinsert(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_orderaction(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_errorderaction(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rspqryorder(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rtnorder(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rqtrade(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rtntrade(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rqinvestorposition(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rqinvestorpositiondetail(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rqtradingaccount(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rqinstrument(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rqdepthmarketdata(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rqsettlementinfo(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	void pkg_cb_rsperror(Isolate*isolate,CbRtnField* data, Local<Value>cbArray[]);
	Local<Value> pkg_rspinfo(Isolate*isolate, void *vpRspInfo);
	uv_trader* uvTrader;
	int s_uuid=0;
	static std::map<const char*, int,ptrCmp> event_map;
	std::map<int, Persistent<Function, CopyablePersistentTraits<Function>> > callback_map;
	std::map<int, Persistent<Function, CopyablePersistentTraits<Function>> > fun_rtncb_map;
};
#endif
