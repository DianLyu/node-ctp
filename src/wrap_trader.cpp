#include <node.h>
#include "wrap_trader.h"
#include "defines.h"
DECL_CONSTR(WrapTrader)
int WrapTrader::s_uuid;
std::map<const char*, int,ptrCmp> WrapTrader::event_map;
std::map<int, Persistent<Function> > WrapTrader::callback_map;
std::map<int, Persistent<Function> > WrapTrader::fun_rtncb_map;

WrapTrader::WrapTrader() {	
	logger_cout("wrap_trader------>object start init");
	uvTrader = new uv_trader();	
	logger_cout("wrap_trader------>object init successed");
}

WrapTrader::~WrapTrader(void) {
    if(uvTrader){
	    delete uvTrader;
    }
	logger_cout("wrap_trader------>object destroyed");
}

void WrapTrader::Init(Handle<Object> target) {
	NEW_CONSTR(WrapTrader);
	initEventMap();
	_WrapTrader.SetProtoTypeMethod("on", On);
	_WrapTrader.SetProtoTypeMethod("connect", Connect);
	_WrapTrader.SetProtoTypeMethod("reqUserLogin", ReqUserLogin);
	_WrapTrader.SetProtoTypeMethod("reqUserLogout", ReqUserLogout);
	_WrapTrader.SetProtoTypeMethod("reqSettlementInfoConfirm", ReqSettlementInfoConfirm);
	_WrapTrader.SetProtoTypeMethod("reqQryInstrument", ReqQryInstrument);
	_WrapTrader.SetProtoTypeMethod("reqQryTradingAccount", ReqQryTradingAccount);
	_WrapTrader.SetProtoTypeMethod("reqQryInvestorPosition", ReqQryInvestorPosition);
	_WrapTrader.SetProtoTypeMethod("reqQryInvestorPositionDetail", ReqQryInvestorPositionDetail);
	_WrapTrader.SetProtoTypeMethod("reqOrderInsert", ReqOrderInsert);
	_WrapTrader.SetProtoTypeMethod("reqOrderAction", ReqOrderAction);
	_WrapTrader.SetProtoTypeMethod("reqQryInstrumentMarginRate", ReqQryInstrumentMarginRate);
	_WrapTrader.SetProtoTypeMethod("reqQryDepthMarketData", ReqQryDepthMarketData);
	_WrapTrader.SetProtoTypeMethod("reqQrySettlementInfo", ReqQrySettlementInfo);
	_WrapTrader.SetProtoTypeMethod("disconnect", Disposed);
	_WrapTrader.SetProtoTypeMethod("getTradingDay", GetTradingDay);
	NODE_SET_METHOD(target, "createMdUser", CreateCObject<WrapTrader>);
}

void WrapTrader::initEventMap() {
	event_map["connect"] = T_ON_CONNECT;
	event_map["disconnected"] = T_ON_DISCONNECTED;
	event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
	event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
	event_map["rspInfoconfirm"] = T_ON_RSPINFOCONFIRM;
	event_map["rspInsert"] = T_ON_RSPINSERT;
	event_map["errInsert"] = T_ON_ERRINSERT;
	event_map["rspAction"] = T_ON_RSPACTION;
	event_map["errAction"] = T_ON_ERRACTION;
	event_map["rqOrder"] = T_ON_RQORDER;
	event_map["rtnOrder"] = T_ON_RTNORDER;
	event_map["rqTrade"] = T_ON_RQTRADE;
	event_map["rtnTrade"] = T_ON_RTNTRADE;
	event_map["rqInvestorPosition"] = T_ON_RQINVESTORPOSITION;
	event_map["rqInvestorPositionDetail"] = T_ON_RQINVESTORPOSITIONDETAIL;
	event_map["rqTradingAccount"] = T_ON_RQTRADINGACCOUNT;
	event_map["rqInstrument"] = T_ON_RQINSTRUMENT;
	event_map["rqDdpthmarketData"] = T_ON_RQDEPTHMARKETDATA;
	event_map["rqSettlementInfo"] = T_ON_RQSETTLEMENTINFO;
	event_map["rspError"] = T_ON_RSPERROR;
}

FUNCTIONCALLBACK(WrapTrader::On)
	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		logger_cout("Wrong arguments->event name or function");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->event name or function")));
		return;
	}
	
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	
	Local<String> eventName = args[0]->ToString();
	Local<Function> cb = Local<Function>::Cast(args[1]);
	String::Utf8Value eNameAscii(eventName);

	std::map<const char*, int>::iterator eIt = event_map.find(*eNameAscii);
	if (eIt == event_map.end()) {
		logger_cout("System has not register this event");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("System has no register this event")));
		return;
	}

	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(eIt->second);
	if (cIt != callback_map.end()) {
		logger_cout("Callback is defined before");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Callback is defined before")));
		return;
	}

	callback_map[eIt->second] = Persistent<Function>(isolate,cb);
	obj->uvTrader->On(*eNameAscii,eIt->second, FunCallback);
	args.GetReturnValue().Set(GETLOCAL(0));
}

FUNCTIONCALLBACK(WrapTrader::Connect)
		
	std::string log = "wrap_trader Connect------>";
	if (args[0]->IsUndefined()) {
		logger_cout("Wrong arguments->front addr");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->front addr")));
		return;
	}
	if (!args[2]->IsNumber() || !args[3]->IsNumber()) {
		logger_cout("Wrong arguments->public or private topic type");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->public or private topic type")));
		return;
	}  
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[4]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	//Local<String> frontAddr = args[0]->ToString();
	//Local<String> szPath = args[1]->IsUndefined() ? GETLOCAL("t") : args[1]->ToString();
	//String::AsciiValue addrAscii(frontAddr);
	//String::AsciiValue pathAscii(szPath);
	int publicTopicType = args[2]->Int32Value();
	int privateTopicType = args[3]->Int32Value();	 
	if (args[1]->IsUndefined()) {
		args[1] = GETLOCAL("t");
	}
	UVConnectField pConnectField; 
	memset(&pConnectField, 0, sizeof(pConnectField));		
	//strcpy(pConnectField.front_addr, ((std::string)*addrAscii).c_str());
	//strcpy(pConnectField.szPath, ((std::string)*pathAscii).c_str());
	ArgsToObject(args, pConnectField.front_addr, pConnectField.szPath);
	pConnectField.public_topic_type = publicTopicType;
	pConnectField.private_topic_type = privateTopicType;	
	logger_cout(log.append(" ").append(pConnectField.front_addr).append("|").append(pConnectField.szPath).append("|").append(to_string(publicTopicType)).append("|").append(to_string(privateTopicType)).c_str());
	obj->uvTrader->Connect(&pConnectField, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqUserLogin)
	
	std::string log = "wrap_trader ReqUserLogin------>";
	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}

	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	/*Local<String> broker = args[0]->ToString();
	Local<String> userId = args[1]->ToString();
	Local<String> pwd = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue userIdAscii(userId);
	String::AsciiValue pwdAscii(pwd);
*/
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.UserID, req.Password);
	//strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	//strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
	//strcpy(req.Password, ((std::string)*pwdAscii).c_str());	
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.UserID).append("|").append(req.Password).c_str());
	obj->uvTrader->ReqUserLogin(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqUserLogout)
	
	std::string log = "wrap_trader ReqUserLogout------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	/*Local<String> broker = args[0]->ToString();
	Local<String> userId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue userIdAscii(userId);*/

	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.UserID);
	/*strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.UserID, ((std::string)*userIdAscii).c_str());*/
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.UserID).c_str());
	obj->uvTrader->ReqUserLogout(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqSettlementInfoConfirm)
	
	std::string log = "wrap_trader ReqSettlementInfoConfirm------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;	
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}	 

	/*Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);*/

	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.InvestorID);
	/*strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());*/
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.InvestorID).c_str());
	obj->uvTrader->ReqSettlementInfoConfirm(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqQryInstrument)
	
	std::string log = "wrap_trader ReqQryInstrument------>";

	if (args[0]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	//Local<String> instrumentId = args[0]->ToString();
	//String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.InstrumentID);
	//strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	logger_cout(log.append(" ").append(req.InstrumentID).c_str());
	obj->uvTrader->ReqQryInstrument(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqQryTradingAccount)
	
	std::string log = "wrap_trader ReqQryTradingAccount------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	/*Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);*/

	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.InvestorID);
	/*strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());*/
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.InvestorID).c_str());
	obj->uvTrader->ReqQryTradingAccount(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqQryInvestorPosition)
	
	std::string log = "wrap_trader ReqQryInvestorPosition------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	/*Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);*/

	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	/*strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
*/
	ArgsToObject(args, req.BrokerID, req.InvestorID, req.InstrumentID);
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.InvestorID).append("|").append(req.InstrumentID).c_str());
	obj->uvTrader->ReqQryInvestorPosition(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqQryInvestorPositionDetail)
	
	std::string log = "wrap_trader ReqQryInvestorPositionDetail------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	/*Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);*/

	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	/*strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());*/
	ArgsToObject(args, req.BrokerID, req.InvestorID, req.InstrumentID);
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.InvestorID).append("|").append(req.InstrumentID).c_str());
	obj->uvTrader->ReqQryInvestorPositionDetail(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqOrderInsert)
	
	std::string log = "wrap_trader ReqOrderInsert------>";

	if (args[0]->IsUndefined() || !args[0]->IsObject()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<Object> jsonObj = args[0]->ToObject();
	Local<Value> brokerId = jsonObj->Get(GETLOCAL("brokerId"));
	if (brokerId->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->brokerId")));
		return;
	}
	//String::AsciiValue brokerId_(brokerId->ToString());
	Local<Value> investorId = jsonObj->Get(GETLOCAL("investorId"));
	if (investorId->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->investorId")));
		return;
	}
	//String::AsciiValue investorId_(investorId->ToString());
	Local<Value> instrumentId = jsonObj->Get(GETLOCAL("instrumentId"));
	if (instrumentId->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->instrumentId")));
		return;
	}
	//String::AsciiValue instrumentId_(instrumentId->ToString());
	Local<Value> priceType = jsonObj->Get(GETLOCAL("priceType"));
	if (priceType->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->priceType")));
		return;
	}
	//String::AsciiValue priceType_(priceType->ToString());
	Local<Value> direction = jsonObj->Get(GETLOCAL("direction"));
	if (direction->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->direction")));
		return;
	}
	//String::AsciiValue direction_(direction->ToString());
	Local<Value> combOffsetFlag = jsonObj->Get(GETLOCAL("combOffsetFlag"));
	if (combOffsetFlag->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->combOffsetFlag")));
		return;
	}
	//String::AsciiValue combOffsetFlag_(combOffsetFlag->ToString());
	Local<Value> combHedgeFlag = jsonObj->Get(GETLOCAL("combHedgeFlag"));
	if (combHedgeFlag->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->combHedgeFlag")));
		return;
	}
	//String::AsciiValue combHedgeFlag_(combHedgeFlag->ToString());
	Local<Value> vlimitPrice = jsonObj->Get(GETLOCAL("limitPrice"));
	if (vlimitPrice->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->limitPrice")));
		return;
	}
	double limitPrice = vlimitPrice->NumberValue();
	Local<Value> vvolumeTotalOriginal = jsonObj->Get(GETLOCAL("volumeTotalOriginal"));
	if (vvolumeTotalOriginal->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->volumeTotalOriginal")));
		return;
	}
	int32_t volumeTotalOriginal = vvolumeTotalOriginal->Int32Value();
	Local<Value> timeCondition = jsonObj->Get(GETLOCAL("timeCondition"));
	if (timeCondition->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->timeCondition")));
		return;
	}
	//String::AsciiValue timeCondition_(timeCondition->ToString());
	Local<Value> volumeCondition = jsonObj->Get(GETLOCAL("volumeCondition"));
	if (volumeCondition->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->volumeCondition")));
		return;
	}
	//String::AsciiValue volumeCondition_(volumeCondition->ToString());
	Local<Value> vminVolume = jsonObj->Get(GETLOCAL("minVolume"));
	if (vminVolume->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->minVolume")));
		return;
	}
	int32_t minVolume = vminVolume->Int32Value();
	Local<Value> forceCloseReason = jsonObj->Get(GETLOCAL("forceCloseReason"));
	if (forceCloseReason->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->forceCloseReason")));
		return;
	}
	//String::AsciiValue forceCloseReason_(forceCloseReason->ToString());
	Local<Value> visAutoSuspend = jsonObj->Get(GETLOCAL("isAutoSuspend"));
	if (visAutoSuspend->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->isAutoSuspend")));
		return;
	}
	int32_t isAutoSuspend = visAutoSuspend->Int32Value();
	Local<Value> vuserForceClose = jsonObj->Get(GETLOCAL("userForceClose"));
	if (vuserForceClose->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->userForceClose")));
		return;
	}
	int32_t userForceClose = vuserForceClose->Int32Value();

	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
    log.append(" ");

    Local<Value> orderRef = jsonObj->Get(GETLOCAL("orderRef"));
    if (!orderRef->IsUndefined()) {
        //String::AsciiValue orderRef_(orderRef->ToString());
        //strcpy(req.OrderRef, ((std::string)*orderRef_).c_str());  
		orderRef->ToString()->WriteOneByte((uint8_t*)req.OrderRef);
        log.append("orderRef:").append(req.OrderRef).append("|");
    }

    Local<Value> vstopPrice = jsonObj->Get(GETLOCAL("stopPrice"));
    if(!vstopPrice->IsUndefined()){
        double stopPrice = vstopPrice->NumberValue();
        req.StopPrice = stopPrice;
        log.append("stopPrice:").append(to_string(stopPrice)).append("|");
    }
    Local<Value> contingentCondition = jsonObj->Get(GETLOCAL("contingentCondition"));
    if (!contingentCondition->IsUndefined()) {
        //String::AsciiValue contingentCondition_(contingentCondition->ToString());
	   // req.ContingentCondition = ((std::string)*contingentCondition_)[0];
		char t[2] = { 0 };
		contingentCondition->ToString()->WriteOneByte((uint8_t*)t);
		req.ContingentCondition = t[0];
		log.append("contingentCondition:").append(t).append("|");
    }
	brokerId->ToString()->WriteOneByte((uint8_t*)req.BrokerID);
	investorId->ToString()->WriteOneByte((uint8_t*)req.InvestorID);
	instrumentId->ToString()->WriteOneByte((uint8_t*)req.InstrumentID);
	/*strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
	strcpy(req.InvestorID, ((std::string)*investorId_).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentId_).c_str());*/
	char t[5] = { 0 };
	uint8_t* _t = (uint8_t*)t;
	priceType->ToString()->WriteOneByte(_t);
	req.OrderPriceType = t[0];
	direction->ToString()->WriteOneByte(_t);
	req.Direction = t[0];
	combOffsetFlag->ToString()->WriteOneByte(_t);
	req.CombOffsetFlag[0] = t[0];
	combHedgeFlag->ToString()->WriteOneByte(_t);
	req.CombHedgeFlag[0] = t[0];
	req.LimitPrice = limitPrice;
	req.VolumeTotalOriginal = volumeTotalOriginal;
	timeCondition->ToString()->WriteOneByte(_t);
	req.TimeCondition = t[0];
	volumeCondition->ToString()->WriteOneByte(_t);
	req.VolumeCondition = t[0];
	req.MinVolume = minVolume;
	forceCloseReason->ToString()->WriteOneByte(_t);
	req.ForceCloseReason = t[0];
	req.IsAutoSuspend = isAutoSuspend;
	req.UserForceClose = userForceClose;
	t[0] = req.OrderPriceType;
	char t2[2] = { req.Direction ,0};
	char t3[2] = { req.TimeCondition ,0 };
	char t4[2] = { req.VolumeCondition ,0 };
	char t5[2] = { req.ForceCloseReason ,0 };
	logger_cout(log.
        append("brokerID:").append(req.BrokerID).append("|").
		append("investorID:").append(req.InvestorID).append("|").
		append("instrumentID:").append(req.InstrumentID).append("|").
		append("priceType:").append(t).append("|").
		append("direction:").append(t2).append("|").
		append("comboffsetFlag:").append(req.CombOffsetFlag).append("|").
		append("combHedgeFlag:").append(req.CombHedgeFlag).append("|").
		append("limitPrice:").append(to_string(limitPrice)).append("|").
		append("volumnTotalOriginal:").append(to_string(volumeTotalOriginal)).append("|").
		append("timeCondition:").append(t3).append("|").
		append("volumeCondition:").append(t4).append("|").
		append("minVolume:").append(to_string(minVolume)).append("|").
		append("forceCloseReason:").append(t5).append("|").
		append("isAutoSuspend:").append(to_string(isAutoSuspend)).append("|").
		append("useForceClose:").append(to_string(userForceClose)).append("|").c_str());
	obj->uvTrader->ReqOrderInsert(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqOrderAction)
	
	std::string log = "wrap_trader ReqOrderAction------>";

	if (args[0]->IsUndefined() || !args[0]->IsObject()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<Object> jsonObj = args[0]->ToObject();
	Local<Value> vbrokerId = jsonObj->Get(GETLOCAL("brokerId"));
	if (vbrokerId->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->brokerId")));
		return;
	}
	//String::AsciiValue brokerId_(vbrokerId->ToString());
	Local<Value> vinvestorId = jsonObj->Get(GETLOCAL("investorId"));
	if (vinvestorId->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->investorId")));
		return;
	}
	//String::AsciiValue investorId_(vinvestorId->ToString());
	Local<Value> vinstrumentId = jsonObj->Get(GETLOCAL("instrumentId"));
	if (vinstrumentId->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->instrumentId")));
		return;
	}
	//String::AsciiValue instrumentId_(vinstrumentId->ToString());
	Local<Value> vactionFlag = jsonObj->Get(GETLOCAL("actionFlag"));
	if (vactionFlag->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->actionFlag")));
		return;
	}
	int32_t actionFlag = vactionFlag->Int32Value();

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));

    log.append(" ");
	Local<Value> vorderRef = jsonObj->Get(GETLOCAL("orderRef"));
	if (!vorderRef->IsUndefined()) {
		vorderRef->ToString()->WriteOneByte((uint8_t*)req.OrderRef);
		log.append("orderRef:").append(req.OrderRef).append("|");
	}
	Local<Value> vfrontId = jsonObj->Get(GETLOCAL("frontId"));
	if (!vfrontId->IsUndefined()) {
	    int32_t frontId = vfrontId->Int32Value();
	    req.FrontID = frontId;
		log.append(to_string(frontId)).append("|");
	}
	Local<Value> vsessionId = jsonObj->Get(GETLOCAL("sessionId"));
	if (!vsessionId->IsUndefined()) {
	    int32_t sessionId = vsessionId->Int32Value();
	    req.SessionID = sessionId;
		log.append(to_string(sessionId)).append("|");
	}
	Local<Value> vexchangeID = jsonObj->Get(GETLOCAL("exchangeID"));
	if (!vexchangeID->IsUndefined()) {
	    //String::AsciiValue exchangeID_(vexchangeID->ToString());
	    //strcpy(req.ExchangeID, ((std::string)*exchangeID_).c_str());
		vexchangeID->ToString()->WriteOneByte((uint8_t*)req.ExchangeID);
		log.append(req.ExchangeID).append("|");
	}
	Local<Value> vorderSysID = jsonObj->Get(GETLOCAL("orderSysID"));
	if (vorderSysID->IsUndefined()) {
	    //String::AsciiValue orderSysID_(vorderSysID->ToString());
	    //strcpy(req.OrderSysID, ((std::string)*orderSysID_).c_str());
		vorderSysID->ToString()->WriteOneByte((uint8_t*)req.OrderSysID);
		log.append(req.OrderSysID).append("|");
	}

	/*strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
	strcpy(req.InvestorID, ((std::string)*investorId_).c_str());*/
	req.ActionFlag = actionFlag;
	//strcpy(req.InstrumentID, ((std::string)*instrumentId_).c_str());
	vbrokerId->ToString()->WriteOneByte((uint8_t*)req.BrokerID);
	vinvestorId->ToString()->WriteOneByte((uint8_t*)req.InvestorID);
	vinstrumentId->ToString()->WriteOneByte((uint8_t*)req.InstrumentID);
	logger_cout(log.
		append(req.BrokerID).append("|").
		append(req.InvestorID).append("|").
		append(req.InstrumentID).append("|").
		append(to_string(actionFlag)).append("|").c_str());

	obj->uvTrader->ReqOrderAction(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqQryInstrumentMarginRate)
	
	std::string log = "wrap_trader ReqQryInstrumentMarginRate------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || args[3]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

	if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[4]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	/*Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();*/
	int32_t hedgeFlag = args[3]->Int32Value();
	/*String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);*/

	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	/*strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());*/
	ArgsToObject(args, req.BrokerID, req.InvestorID, req.InstrumentID);
	req.HedgeFlag = hedgeFlag;
	logger_cout(log.append(" ").
		append(req.BrokerID).append("|").
		append(req.InvestorID).append("|").
		append(req.InstrumentID).append("|").
		append(to_string(hedgeFlag)).append("|").c_str());	 

	obj->uvTrader->ReqQryInstrumentMarginRate(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqQryDepthMarketData)
	
	std::string log = "wrap_trader ReqQryDepthMarketData------>";

	if (args[0]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	//Local<String> instrumentId = args[0]->ToString();
	//String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryDepthMarketDataField req;
	memset(&req, 0, sizeof(req));
	//strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	ArgsToObject(args, req.InstrumentID);
	logger_cout(log.append(" ").
		append(req.InstrumentID).append("|").c_str());
	obj->uvTrader->ReqQryDepthMarketData(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::ReqQrySettlementInfo)
	
	std::string log = "wrap_trader ReqQrySettlementInfo------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	/*Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> tradingDay = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue tradingDayAscii(tradingDay);*/

	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof(req));
	/*strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.TradingDay, ((std::string)*tradingDayAscii).c_str());*/
	ArgsToObject(args, req.BrokerID, req.InvestorID, req.TradingDay);
	logger_cout(log.append(" ").
		append(req.BrokerID).append("|").
		append(req.InvestorID).append("|").
		append(req.TradingDay).append("|").c_str());

	obj->uvTrader->ReqQrySettlementInfo(&req, FunRtnCallback, uuid);
	return;
}

FUNCTIONCALLBACK(WrapTrader::Disposed)
	
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	obj->uvTrader->Disconnect();	
	std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
	while (callback_it != callback_map.end()) {
		callback_it->second.Reset();
		callback_it++;
	}
	event_map.clear();
	callback_map.clear();
	fun_rtncb_map.clear();
	delete obj->uvTrader;
    obj->uvTrader = NULL;
	logger_cout("wrap_trader Disposed------>wrap disposed");
	return;
}
FUNCTIONCALLBACK(WrapTrader::GetTradingDay)
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	const char* tradingDay = obj->uvTrader->GetTradingDay();
	args.GetReturnValue().Set(GETLOCAL(tradingDay));
}

void WrapTrader::FunCallback(CbRtnField *data) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(data->eFlag);
	if (cIt == callback_map.end())
		return;
#define FunCallback_Switch(type,count,...) case type:{\
 Local<Value> argv[count]\
__VA_ARGS__\
 cIt->second.Get(isolate)->Call(isolate->GetCurrentContext()->Global(), count, argv);\
break;\
}
	switch (data->eFlag) {
		FunCallback_Switch(T_ON_CONNECT, 1, ={ Undefined(isolate) };)
			FunCallback_Switch(T_ON_DISCONNECTED, 1, ={ GETLOCAL(data->nReason) };)
			FunCallback_Switch(T_ON_RSPUSERLOGIN, 4, ; pkg_cb_userlogin(data, argv);)
	
	case T_ON_RSPUSERLOGOUT:
	{
							   Local<Value> argv[4];
							   pkg_cb_userlogout(data, argv);
							   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
							   break;
	}
	case T_ON_RSPINFOCONFIRM:
	{
								Local<Value> argv[4];
								pkg_cb_confirm(data, argv);
								cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
								break;
	}
	case T_ON_RSPINSERT:
	{
						   Local<Value> argv[4];
						   pkg_cb_orderinsert(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						   break;
	}
	case T_ON_ERRINSERT:
	{
						   Local<Value> argv[2];
						   pkg_cb_errorderinsert(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);
						   break;
	}
	case T_ON_RSPACTION:
	{
						   Local<Value> argv[4];
						   pkg_cb_orderaction(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						   break;
	}
	case T_ON_ERRACTION:
	{
						   Local<Value> argv[2];
						   pkg_cb_errorderaction(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);

						   break;
	}
	case T_ON_RQORDER:
	{
						 Local<Value> argv[4];
						 pkg_cb_rspqryorder(data, argv);
						 cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						 break;
	}
	case T_ON_RTNORDER:
	{
						  Local<Value> argv[1];
						  pkg_cb_rtnorder(data, argv);
						  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

						  break;
	}
	case T_ON_RQTRADE:
	{
						 Local<Value> argv[4];
						 pkg_cb_rqtrade(data, argv);
						 cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

						 break;
	}
	case T_ON_RTNTRADE:
	{
						  Local<Value> argv[1];
						  pkg_cb_rtntrade(data, argv);
						  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

						  break;
	}
	case T_ON_RQINVESTORPOSITION:
	{
									Local<Value> argv[4];
									pkg_cb_rqinvestorposition(data, argv);
									cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

									break;
	}
	case T_ON_RQINVESTORPOSITIONDETAIL:
	{
										  Local<Value> argv[4];
										  pkg_cb_rqinvestorpositiondetail(data, argv);
										  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

										  break;
	}
	case T_ON_RQTRADINGACCOUNT:
	{
								  Local<Value> argv[4];
								  pkg_cb_rqtradingaccount(data, argv);
								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

								  break;
	}
	case T_ON_RQINSTRUMENT:
	{
							  Local<Value> argv[4];
							  pkg_cb_rqinstrument(data, argv);
							  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

							  break;
	}
	case T_ON_RQDEPTHMARKETDATA:
	{
								   Local<Value> argv[4];
								   pkg_cb_rqdepthmarketdata(data, argv);
								   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

								   break;
	}
	case T_ON_RQSETTLEMENTINFO:
	{
								  Local<Value> argv[4];
								  pkg_cb_rqsettlementinfo(data, argv);
								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
								  break;
	}
	case T_ON_RSPERROR:
	{
						   Local<Value> argv[3];
						   pkg_cb_rsperror(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 3, argv);

						   break;
	}
	}
#undef FunCallback_Switch
}

void WrapTrader::FunRtnCallback(int result, void* baton) {
	
	LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);	 
	if (tmp->uuid != -1) {
		std::map<int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[2] = { Local<Value>::New(Int32::New(tmp->nResult)),Local<Value>::New(Int32::New(tmp->iRequestID)) };
		it->second->Call(Context::GetCurrent()->Global(), 2, argv);
		it->second.Dispose();
		fun_rtncb_map.erase(tmp->uuid);	  		
	}
	scope.Close(Undefined());
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WrapTrader::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	if (data->rtnField){  
	    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pRspUserLogin->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("LoginTime"), GETLOCAL(pRspUserLogin->LoginTime));
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pRspUserLogin->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pRspUserLogin->UserID));
		jsonRtn->Set(GETLOCALSymbol("SystemName"), GETLOCAL(pRspUserLogin->SystemName));
		jsonRtn->Set(GETLOCALSymbol("FrontID"), Int32::New(pRspUserLogin->FrontID));
		jsonRtn->Set(GETLOCALSymbol("SessionID"), Int32::New(pRspUserLogin->SessionID));
		jsonRtn->Set(GETLOCALSymbol("MaxOrderRef"), GETLOCAL(pRspUserLogin->MaxOrderRef));
		jsonRtn->Set(GETLOCALSymbol("SHFETime"), GETLOCAL(pRspUserLogin->SHFETime));
		jsonRtn->Set(GETLOCALSymbol("DCETime"), GETLOCAL(pRspUserLogin->DCETime));
		jsonRtn->Set(GETLOCALSymbol("CZCETime"), GETLOCAL(pRspUserLogin->CZCETime));
		jsonRtn->Set(GETLOCALSymbol("FFEXTime"), GETLOCAL(pRspUserLogin->FFEXTime));
		jsonRtn->Set(GETLOCALSymbol("INETime"), GETLOCAL(pRspUserLogin->INETime));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pRspUserLogin->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pRspUserLogin->UserID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm = static_cast<CThostFtdcSettlementInfoConfirmField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pSettlementInfoConfirm->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pSettlementInfoConfirm->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("ConfirmDate"), GETLOCAL(pSettlementInfoConfirm->ConfirmDate));
		jsonRtn->Set(GETLOCALSymbol("ConfirmTime"), GETLOCAL(pSettlementInfoConfirm->ConfirmTime));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pInputOrder->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pInputOrder->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pInputOrder->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pInputOrder->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pInputOrder->UserID));
		jsonRtn->Set(GETLOCALSymbol("OrderPriceType"), GETLOCAL(charto_string(pInputOrder->OrderPriceType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("Direction"), GETLOCAL(charto_string(pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(GETLOCALSymbol("CombOffsetFlag"), GETLOCAL(pInputOrder->CombOffsetFlag));
		jsonRtn->Set(GETLOCALSymbol("CombHedgeFlag"), GETLOCAL(pInputOrder->CombHedgeFlag));
		jsonRtn->Set(GETLOCALSymbol("LimitPrice"), Number::New(pInputOrder->LimitPrice));
		jsonRtn->Set(GETLOCALSymbol("VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
		jsonRtn->Set(GETLOCALSymbol("TimeCondition"), GETLOCAL(charto_string(pInputOrder->TimeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("GTDDate"), GETLOCAL(pInputOrder->GTDDate));
		jsonRtn->Set(GETLOCALSymbol("VolumeCondition"), GETLOCAL(charto_string(pInputOrder->VolumeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("MinVolume"), Int32::New(pInputOrder->MinVolume));
		jsonRtn->Set(GETLOCALSymbol("ContingentCondition"), GETLOCAL(charto_string(pInputOrder->ContingentCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("StopPrice"), Number::New(pInputOrder->StopPrice));
		jsonRtn->Set(GETLOCALSymbol("ForceCloseReason"), GETLOCAL(charto_string(pInputOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(GETLOCALSymbol("IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
		jsonRtn->Set(GETLOCALSymbol("BusinessUnit"), GETLOCAL(pInputOrder->BusinessUnit));
		jsonRtn->Set(GETLOCALSymbol("RequestID"), Int32::New(pInputOrder->RequestID));
		jsonRtn->Set(GETLOCALSymbol("UserForceClose"), Int32::New(pInputOrder->UserForceClose));
		jsonRtn->Set(GETLOCALSymbol("IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pInputOrder->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pInputOrder->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pInputOrder->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pInputOrder->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pInputOrder->UserID));
		jsonRtn->Set(GETLOCALSymbol("OrderPriceType"), GETLOCAL(charto_string(pInputOrder->OrderPriceType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("Direction"), GETLOCAL(charto_string(pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(GETLOCALSymbol("CombOffsetFlag"), GETLOCAL(pInputOrder->CombOffsetFlag));
		jsonRtn->Set(GETLOCALSymbol("CombHedgeFlag"), GETLOCAL(pInputOrder->CombHedgeFlag));
		jsonRtn->Set(GETLOCALSymbol("LimitPrice"), Number::New(pInputOrder->LimitPrice));
		jsonRtn->Set(GETLOCALSymbol("VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
		jsonRtn->Set(GETLOCALSymbol("TimeCondition"), GETLOCAL(charto_string(pInputOrder->TimeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("GTDDate"), GETLOCAL(pInputOrder->GTDDate));
		jsonRtn->Set(GETLOCALSymbol("VolumeCondition"), GETLOCAL(charto_string(pInputOrder->VolumeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("MinVolume"), Int32::New(pInputOrder->MinVolume));
		jsonRtn->Set(GETLOCALSymbol("ContingentCondition"), GETLOCAL(charto_string(pInputOrder->ContingentCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("StopPrice"), Number::New(pInputOrder->StopPrice));
		jsonRtn->Set(GETLOCALSymbol("ForceCloseReason"), GETLOCAL(charto_string(pInputOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(GETLOCALSymbol("IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
		jsonRtn->Set(GETLOCALSymbol("BusinessUnit"), GETLOCAL(pInputOrder->BusinessUnit));
		jsonRtn->Set(GETLOCALSymbol("RequestID"), Int32::New(pInputOrder->RequestID));
		jsonRtn->Set(GETLOCALSymbol("UserForceClose"), Int32::New(pInputOrder->UserForceClose));
		jsonRtn->Set(GETLOCALSymbol("IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	*(cbArray + 1) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInputOrderActionField* pInputOrderAction = static_cast<CThostFtdcInputOrderActionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pInputOrderAction->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pInputOrderAction->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("OrderActionRef"), Int32::New(pInputOrderAction->OrderActionRef));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pInputOrderAction->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("RequestID"), Int32::New(pInputOrderAction->RequestID));
		jsonRtn->Set(GETLOCALSymbol("FrontID"), Int32::New(pInputOrderAction->FrontID));
		jsonRtn->Set(GETLOCALSymbol("SessionID"), Int32::New(pInputOrderAction->SessionID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pInputOrderAction->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("OrderSysID"), GETLOCAL(pInputOrderAction->OrderSysID));
		jsonRtn->Set(GETLOCALSymbol("ActionFlag"), GETLOCAL(charto_string(pInputOrderAction->ActionFlag).c_str()));
		jsonRtn->Set(GETLOCALSymbol("LimitPrice"), Number::New(pInputOrderAction->LimitPrice));
		jsonRtn->Set(GETLOCALSymbol("VolumeChange"), Int32::New(pInputOrderAction->VolumeChange));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pInputOrderAction->UserID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pInputOrderAction->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcOrderActionField* pOrderAction = static_cast<CThostFtdcOrderActionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pOrderAction->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pOrderAction->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("OrderActionRef"), Int32::New(pOrderAction->OrderActionRef));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pOrderAction->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("RequestID"), Int32::New(pOrderAction->RequestID));
		jsonRtn->Set(GETLOCALSymbol("FrontID"), Int32::New(pOrderAction->FrontID));
		jsonRtn->Set(GETLOCALSymbol("SessionID"), Int32::New(pOrderAction->SessionID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pOrderAction->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("OrderSysID"), GETLOCAL(pOrderAction->OrderSysID));
		jsonRtn->Set(GETLOCALSymbol("ActionFlag"), GETLOCAL(charto_string(pOrderAction->ActionFlag).c_str()));
		jsonRtn->Set(GETLOCALSymbol("LimitPrice"), Number::New(pOrderAction->LimitPrice));
		jsonRtn->Set(GETLOCALSymbol("VolumeChange"), Int32::New(pOrderAction->VolumeChange));
		jsonRtn->Set(GETLOCALSymbol("ActionDate"), GETLOCAL(pOrderAction->ActionDate));
		jsonRtn->Set(GETLOCALSymbol("TraderID"), GETLOCAL(pOrderAction->TraderID));
		jsonRtn->Set(GETLOCALSymbol("InstallID"), Int32::New(pOrderAction->InstallID));
		jsonRtn->Set(GETLOCALSymbol("OrderLocalID"), GETLOCAL(pOrderAction->OrderLocalID));
		jsonRtn->Set(GETLOCALSymbol("ActionLocalID"), GETLOCAL(pOrderAction->ActionLocalID));
		jsonRtn->Set(GETLOCALSymbol("ParticipantID"), GETLOCAL(pOrderAction->ParticipantID));
		jsonRtn->Set(GETLOCALSymbol("ClientID"), GETLOCAL(pOrderAction->ClientID));
		jsonRtn->Set(GETLOCALSymbol("BusinessUnit"), GETLOCAL(pOrderAction->BusinessUnit));
		jsonRtn->Set(GETLOCALSymbol("OrderActionStatus"), GETLOCAL(charto_string(pOrderAction->OrderActionStatus).c_str()));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pOrderAction->UserID));
		jsonRtn->Set(GETLOCALSymbol("StatusMsg"), GETLOCAL(pOrderAction->StatusMsg));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pOrderAction->InstrumentID));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	*(cbArray + 1) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pOrder->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pOrder->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pOrder->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pOrder->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pOrder->UserID));
		jsonRtn->Set(GETLOCALSymbol("OrderPriceType"), GETLOCAL(charto_string(pOrder->OrderPriceType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("Direction"), GETLOCAL(charto_string(pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(GETLOCALSymbol("CombOffsetFlag"), GETLOCAL(pOrder->CombOffsetFlag));
		jsonRtn->Set(GETLOCALSymbol("CombHedgeFlag"), GETLOCAL(pOrder->CombHedgeFlag));
		jsonRtn->Set(GETLOCALSymbol("LimitPrice"), Number::New(pOrder->LimitPrice));
		jsonRtn->Set(GETLOCALSymbol("VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
		jsonRtn->Set(GETLOCALSymbol("TimeCondition"), GETLOCAL(charto_string(pOrder->TimeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("GTDDate"), GETLOCAL(pOrder->GTDDate));
		jsonRtn->Set(GETLOCALSymbol("VolumeCondition"), GETLOCAL(charto_string(pOrder->VolumeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("MinVolume"), Int32::New(pOrder->MinVolume));
		jsonRtn->Set(GETLOCALSymbol("ContingentCondition"), GETLOCAL(charto_string(pOrder->ContingentCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("StopPrice"), Number::New(pOrder->StopPrice));
		jsonRtn->Set(GETLOCALSymbol("ForceCloseReason"), GETLOCAL(charto_string(pOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(GETLOCALSymbol("IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
		jsonRtn->Set(GETLOCALSymbol("BusinessUnit"), GETLOCAL(pOrder->BusinessUnit));
		jsonRtn->Set(GETLOCALSymbol("RequestID"), Int32::New(pOrder->RequestID));
		jsonRtn->Set(GETLOCALSymbol("OrderLocalID"), GETLOCAL(pOrder->OrderLocalID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pOrder->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("ParticipantID"), GETLOCAL(pOrder->ParticipantID));
		jsonRtn->Set(GETLOCALSymbol("ClientID"), GETLOCAL(pOrder->ClientID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeInstID"), GETLOCAL(pOrder->ExchangeInstID));
		jsonRtn->Set(GETLOCALSymbol("TraderID"), GETLOCAL(pOrder->TraderID));
		jsonRtn->Set(GETLOCALSymbol("InstallID"), Int32::New(pOrder->InstallID));
		jsonRtn->Set(GETLOCALSymbol("OrderSubmitStatus"), GETLOCAL(charto_string(pOrder->OrderSubmitStatus).c_str()));
		jsonRtn->Set(GETLOCALSymbol("NotifySequence"), Int32::New(pOrder->NotifySequence));
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pOrder->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(pOrder->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("OrderSysID"), GETLOCAL(pOrder->OrderSysID));
		jsonRtn->Set(GETLOCALSymbol("OrderSource"), Int32::New(pOrder->OrderSource));
		jsonRtn->Set(GETLOCALSymbol("OrderStatus"), GETLOCAL(charto_string(pOrder->OrderStatus).c_str()));
		jsonRtn->Set(GETLOCALSymbol("OrderType"), GETLOCAL(charto_string(pOrder->OrderType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("VolumeTraded"), Int32::New(pOrder->VolumeTraded));
		jsonRtn->Set(GETLOCALSymbol("VolumeTotal"), Int32::New(pOrder->VolumeTotal));
		jsonRtn->Set(GETLOCALSymbol("InsertDate"), GETLOCAL(pOrder->InsertDate));
		jsonRtn->Set(GETLOCALSymbol("InsertTime"), GETLOCAL(pOrder->InsertTime));
		jsonRtn->Set(GETLOCALSymbol("ActiveTime"), GETLOCAL(pOrder->ActiveTime));
		jsonRtn->Set(GETLOCALSymbol("SuspendTime"), GETLOCAL(pOrder->SuspendTime));
		jsonRtn->Set(GETLOCALSymbol("UpdateTime"), GETLOCAL(pOrder->UpdateTime));
		jsonRtn->Set(GETLOCALSymbol("CancelTime"), GETLOCAL(pOrder->CancelTime));
		jsonRtn->Set(GETLOCALSymbol("ActiveTraderID"), GETLOCAL(pOrder->ActiveTraderID));
		jsonRtn->Set(GETLOCALSymbol("ClearingPartID"), GETLOCAL(pOrder->ClearingPartID));
		jsonRtn->Set(GETLOCALSymbol("SequenceNo"), Int32::New(pOrder->SequenceNo));
		jsonRtn->Set(GETLOCALSymbol("FrontID"), Int32::New(pOrder->FrontID));
		jsonRtn->Set(GETLOCALSymbol("SessionID"), Int32::New(pOrder->SessionID));
		jsonRtn->Set(GETLOCALSymbol("UserProductInfo"), GETLOCAL(pOrder->UserProductInfo));
		jsonRtn->Set(GETLOCALSymbol("StatusMsg"), GETLOCAL(pOrder->StatusMsg));
		jsonRtn->Set(GETLOCALSymbol("UserForceClose"), Int32::New(pOrder->UserForceClose));
		jsonRtn->Set(GETLOCALSymbol("ActiveUserID"), GETLOCAL(pOrder->ActiveUserID));
		jsonRtn->Set(GETLOCALSymbol("BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
		jsonRtn->Set(GETLOCALSymbol("RelativeOrderSysID"), GETLOCAL(pOrder->RelativeOrderSysID));
		jsonRtn->Set(GETLOCALSymbol("ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
		jsonRtn->Set(GETLOCALSymbol("IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pOrder->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pOrder->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pOrder->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pOrder->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pOrder->UserID));
		jsonRtn->Set(GETLOCALSymbol("OrderPriceType"), GETLOCAL(charto_string(pOrder->OrderPriceType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("Direction"), GETLOCAL(charto_string(pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(GETLOCALSymbol("CombOffsetFlag"), GETLOCAL(pOrder->CombOffsetFlag));
		jsonRtn->Set(GETLOCALSymbol("CombHedgeFlag"), GETLOCAL(pOrder->CombHedgeFlag));
		jsonRtn->Set(GETLOCALSymbol("LimitPrice"), Number::New(pOrder->LimitPrice));
		jsonRtn->Set(GETLOCALSymbol("VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
		jsonRtn->Set(GETLOCALSymbol("TimeCondition"), GETLOCAL(charto_string(pOrder->TimeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("GTDDate"), GETLOCAL(pOrder->GTDDate));
		jsonRtn->Set(GETLOCALSymbol("VolumeCondition"), GETLOCAL(charto_string(pOrder->VolumeCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("MinVolume"), Int32::New(pOrder->MinVolume));
		jsonRtn->Set(GETLOCALSymbol("ContingentCondition"), GETLOCAL(charto_string(pOrder->ContingentCondition).c_str()));
		jsonRtn->Set(GETLOCALSymbol("StopPrice"), Number::New(pOrder->StopPrice));
		jsonRtn->Set(GETLOCALSymbol("ForceCloseReason"), GETLOCAL(charto_string(pOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(GETLOCALSymbol("IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
		jsonRtn->Set(GETLOCALSymbol("BusinessUnit"), GETLOCAL(pOrder->BusinessUnit));
		jsonRtn->Set(GETLOCALSymbol("RequestID"), Int32::New(pOrder->RequestID));
		jsonRtn->Set(GETLOCALSymbol("OrderLocalID"), GETLOCAL(pOrder->OrderLocalID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pOrder->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("ParticipantID"), GETLOCAL(pOrder->ParticipantID));
		jsonRtn->Set(GETLOCALSymbol("ClientID"), GETLOCAL(pOrder->ClientID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeInstID"), GETLOCAL(pOrder->ExchangeInstID));
		jsonRtn->Set(GETLOCALSymbol("TraderID"), GETLOCAL(pOrder->TraderID));
		jsonRtn->Set(GETLOCALSymbol("InstallID"), Int32::New(pOrder->InstallID));
		jsonRtn->Set(GETLOCALSymbol("OrderSubmitStatus"), GETLOCAL(charto_string(pOrder->OrderSubmitStatus).c_str()));
		jsonRtn->Set(GETLOCALSymbol("NotifySequence"), Int32::New(pOrder->NotifySequence));
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pOrder->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(pOrder->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("OrderSysID"), GETLOCAL(pOrder->OrderSysID));
		jsonRtn->Set(GETLOCALSymbol("OrderSource"), Int32::New(pOrder->OrderSource));
		jsonRtn->Set(GETLOCALSymbol("OrderStatus"), GETLOCAL(charto_string(pOrder->OrderStatus).c_str()));
		jsonRtn->Set(GETLOCALSymbol("OrderType"), GETLOCAL(charto_string(pOrder->OrderType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("VolumeTraded"), Int32::New(pOrder->VolumeTraded));
		jsonRtn->Set(GETLOCALSymbol("VolumeTotal"), Int32::New(pOrder->VolumeTotal));
		jsonRtn->Set(GETLOCALSymbol("InsertDate"), GETLOCAL(pOrder->InsertDate));
		jsonRtn->Set(GETLOCALSymbol("InsertTime"), GETLOCAL(pOrder->InsertTime));
		jsonRtn->Set(GETLOCALSymbol("ActiveTime"), GETLOCAL(pOrder->ActiveTime));
		jsonRtn->Set(GETLOCALSymbol("SuspendTime"), GETLOCAL(pOrder->SuspendTime));
		jsonRtn->Set(GETLOCALSymbol("UpdateTime"), GETLOCAL(pOrder->UpdateTime));
		jsonRtn->Set(GETLOCALSymbol("CancelTime"), GETLOCAL(pOrder->CancelTime));
		jsonRtn->Set(GETLOCALSymbol("ActiveTraderID"), GETLOCAL(pOrder->ActiveTraderID));
		jsonRtn->Set(GETLOCALSymbol("ClearingPartID"), GETLOCAL(pOrder->ClearingPartID));
		jsonRtn->Set(GETLOCALSymbol("SequenceNo"), Int32::New(pOrder->SequenceNo));
		jsonRtn->Set(GETLOCALSymbol("FrontID"), Int32::New(pOrder->FrontID));
		jsonRtn->Set(GETLOCALSymbol("SessionID"), Int32::New(pOrder->SessionID));
		jsonRtn->Set(GETLOCALSymbol("UserProductInfo"), GETLOCAL(pOrder->UserProductInfo));
		jsonRtn->Set(GETLOCALSymbol("StatusMsg"), GETLOCAL(pOrder->StatusMsg));
		jsonRtn->Set(GETLOCALSymbol("UserForceClose"), Int32::New(pOrder->UserForceClose));
		jsonRtn->Set(GETLOCALSymbol("ActiveUserID"), GETLOCAL(pOrder->ActiveUserID));
		jsonRtn->Set(GETLOCALSymbol("BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
		jsonRtn->Set(GETLOCALSymbol("RelativeOrderSysID"), GETLOCAL(pOrder->RelativeOrderSysID));
		jsonRtn->Set(GETLOCALSymbol("ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
		jsonRtn->Set(GETLOCALSymbol("IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	return;
}
void WrapTrader::pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pTrade->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pTrade->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pTrade->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pTrade->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pTrade->UserID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pTrade->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("TradeID"), GETLOCAL(pTrade->TradeID));
		jsonRtn->Set(GETLOCALSymbol("Direction"), GETLOCAL(charto_string(pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(GETLOCALSymbol("OrderSysID"), GETLOCAL(pTrade->OrderSysID));
		jsonRtn->Set(GETLOCALSymbol("ParticipantID"), GETLOCAL(pTrade->ParticipantID));
		jsonRtn->Set(GETLOCALSymbol("ClientID"), GETLOCAL(pTrade->ClientID));
		jsonRtn->Set(GETLOCALSymbol("TradingRole"), GETLOCAL(charto_string(pTrade->TradingRole).c_str()));
		jsonRtn->Set(GETLOCALSymbol("ExchangeInstID"), GETLOCAL(pTrade->ExchangeInstID));
		jsonRtn->Set(GETLOCALSymbol("OffsetFlag"), GETLOCAL(charto_string(pTrade->OffsetFlag).c_str()));
		jsonRtn->Set(GETLOCALSymbol("HedgeFlag"), GETLOCAL(charto_string(pTrade->HedgeFlag).c_str()));
		jsonRtn->Set(GETLOCALSymbol("Price"), Number::New(pTrade->Price));
		jsonRtn->Set(GETLOCALSymbol("Volume"), Int32::New(pTrade->Volume));
		jsonRtn->Set(GETLOCALSymbol("TradeDate"), GETLOCAL(pTrade->TradeDate));
		jsonRtn->Set(GETLOCALSymbol("TradeTime"), GETLOCAL(pTrade->TradeTime));
		jsonRtn->Set(GETLOCALSymbol("TradeType"), GETLOCAL(charto_string(pTrade->TradeType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("PriceSource"), GETLOCAL(charto_string(pTrade->PriceSource).c_str()));
		jsonRtn->Set(GETLOCALSymbol("TraderID"), GETLOCAL(pTrade->TraderID));
		jsonRtn->Set(GETLOCALSymbol("OrderLocalID"), GETLOCAL(pTrade->OrderLocalID));
		jsonRtn->Set(GETLOCALSymbol("ClearingPartID"), GETLOCAL(pTrade->ClearingPartID));
		jsonRtn->Set(GETLOCALSymbol("BusinessUnit"), GETLOCAL(pTrade->BusinessUnit));
		jsonRtn->Set(GETLOCALSymbol("SequenceNo"), Int32::New(pTrade->SequenceNo));
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pTrade->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(pTrade->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
		jsonRtn->Set(GETLOCALSymbol("TradeSource"), GETLOCAL(charto_string(pTrade->TradeSource).c_str()));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pTrade->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pTrade->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pTrade->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("OrderRef"), GETLOCAL(pTrade->OrderRef));
		jsonRtn->Set(GETLOCALSymbol("UserID"), GETLOCAL(pTrade->UserID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pTrade->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("TradeID"), GETLOCAL(pTrade->TradeID));
		jsonRtn->Set(GETLOCALSymbol("Direction"), GETLOCAL(charto_string(pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(GETLOCALSymbol("OrderSysID"), GETLOCAL(pTrade->OrderSysID));
		jsonRtn->Set(GETLOCALSymbol("ParticipantID"), GETLOCAL(pTrade->ParticipantID));
		jsonRtn->Set(GETLOCALSymbol("ClientID"), GETLOCAL(pTrade->ClientID));
		jsonRtn->Set(GETLOCALSymbol("TradingRole"), Int32::New(pTrade->TradingRole));
		jsonRtn->Set(GETLOCALSymbol("ExchangeInstID"), GETLOCAL(pTrade->ExchangeInstID));
		jsonRtn->Set(GETLOCALSymbol("OffsetFlag"), Int32::New(pTrade->OffsetFlag));
		jsonRtn->Set(GETLOCALSymbol("HedgeFlag"), Int32::New(pTrade->HedgeFlag));
		jsonRtn->Set(GETLOCALSymbol("Price"), Number::New(pTrade->Price));
		jsonRtn->Set(GETLOCALSymbol("Volume"), Int32::New(pTrade->Volume));
		jsonRtn->Set(GETLOCALSymbol("TradeDate"), GETLOCAL(pTrade->TradeDate));
		jsonRtn->Set(GETLOCALSymbol("TradeTime"), GETLOCAL(pTrade->TradeTime));
		jsonRtn->Set(GETLOCALSymbol("TradeType"), Int32::New(pTrade->TradeType));
		jsonRtn->Set(GETLOCALSymbol("PriceSource"), GETLOCAL(charto_string(pTrade->PriceSource).c_str()));
		jsonRtn->Set(GETLOCALSymbol("TraderID"), GETLOCAL(pTrade->TraderID));
		jsonRtn->Set(GETLOCALSymbol("OrderLocalID"), GETLOCAL(pTrade->OrderLocalID));
		jsonRtn->Set(GETLOCALSymbol("ClearingPartID"), GETLOCAL(pTrade->ClearingPartID));
		jsonRtn->Set(GETLOCALSymbol("BusinessUnit"), GETLOCAL(pTrade->BusinessUnit));
		jsonRtn->Set(GETLOCALSymbol("SequenceNo"), Int32::New(pTrade->SequenceNo));
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pTrade->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(pTrade->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
		jsonRtn->Set(GETLOCALSymbol("TradeSource"), GETLOCAL(charto_string(pTrade->TradeSource).c_str()));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	 
	return;
}
void WrapTrader::pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInvestorPositionField* _pInvestorPosition = static_cast<CThostFtdcInvestorPositionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(_pInvestorPosition->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(_pInvestorPosition->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(_pInvestorPosition->InvestorID)); 
		jsonRtn->Set(GETLOCALSymbol("PosiDirection"), GETLOCAL(charto_string(_pInvestorPosition->PosiDirection).c_str()));
		jsonRtn->Set(GETLOCALSymbol("HedgeFlag"), GETLOCAL(charto_string(_pInvestorPosition->HedgeFlag).c_str()));
		jsonRtn->Set(GETLOCALSymbol("PositionDate"), Int32::New(_pInvestorPosition->PositionDate));
		jsonRtn->Set(GETLOCALSymbol("YdPosition"), Int32::New(_pInvestorPosition->YdPosition));
		jsonRtn->Set(GETLOCALSymbol("Position"), Int32::New(_pInvestorPosition->Position));
		jsonRtn->Set(GETLOCALSymbol("LongFrozen"), Int32::New(_pInvestorPosition->LongFrozen));
		jsonRtn->Set(GETLOCALSymbol("ShortFrozen"), Int32::New(_pInvestorPosition->ShortFrozen));
		jsonRtn->Set(GETLOCALSymbol("LongFrozenAmount"), Number::New(_pInvestorPosition->LongFrozenAmount));
		jsonRtn->Set(GETLOCALSymbol("ShortFrozenAmount"), Number::New(_pInvestorPosition->ShortFrozenAmount));
		jsonRtn->Set(GETLOCALSymbol("OpenVolume"), Int32::New(_pInvestorPosition->OpenVolume));
		jsonRtn->Set(GETLOCALSymbol("CloseVolume"), Int32::New(_pInvestorPosition->CloseVolume));
		jsonRtn->Set(GETLOCALSymbol("OpenAmount"), Number::New(_pInvestorPosition->OpenAmount));
		jsonRtn->Set(GETLOCALSymbol("CloseAmount"), Number::New(_pInvestorPosition->CloseAmount));
		jsonRtn->Set(GETLOCALSymbol("PositionCost"), Number::New(_pInvestorPosition->PositionCost));
		jsonRtn->Set(GETLOCALSymbol("PreMargin"), Number::New(_pInvestorPosition->PreMargin));
		jsonRtn->Set(GETLOCALSymbol("UseMargin"), Number::New(_pInvestorPosition->UseMargin));
		jsonRtn->Set(GETLOCALSymbol("FrozenMargin"), Number::New(_pInvestorPosition->FrozenMargin));
		jsonRtn->Set(GETLOCALSymbol("FrozenCash"), Number::New(_pInvestorPosition->FrozenCash));
		jsonRtn->Set(GETLOCALSymbol("FrozenCommission"), Number::New(_pInvestorPosition->FrozenCommission));
		jsonRtn->Set(GETLOCALSymbol("CashIn"), Number::New(_pInvestorPosition->CashIn));
		jsonRtn->Set(GETLOCALSymbol("Commission"), Number::New(_pInvestorPosition->Commission));
		jsonRtn->Set(GETLOCALSymbol("CloseProfit"), Number::New(_pInvestorPosition->CloseProfit));
		jsonRtn->Set(GETLOCALSymbol("PositionProfit"), Number::New(_pInvestorPosition->PositionProfit));
		jsonRtn->Set(GETLOCALSymbol("PreSettlementPrice"), Number::New(_pInvestorPosition->PreSettlementPrice));
		jsonRtn->Set(GETLOCALSymbol("SettlementPrice"), Number::New(_pInvestorPosition->SettlementPrice));
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(_pInvestorPosition->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(_pInvestorPosition->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("OpenCost"), Number::New(_pInvestorPosition->OpenCost));
		jsonRtn->Set(GETLOCALSymbol("ExchangeMargin"), Number::New(_pInvestorPosition->ExchangeMargin));
		jsonRtn->Set(GETLOCALSymbol("CombPosition"), Int32::New(_pInvestorPosition->CombPosition));
		jsonRtn->Set(GETLOCALSymbol("CombLongFrozen"), Int32::New(_pInvestorPosition->CombLongFrozen));
		jsonRtn->Set(GETLOCALSymbol("CombShortFrozen"), Int32::New(_pInvestorPosition->CombShortFrozen));
		jsonRtn->Set(GETLOCALSymbol("CloseProfitByDate"), Number::New(_pInvestorPosition->CloseProfitByDate));
		jsonRtn->Set(GETLOCALSymbol("CloseProfitByTrade"), Number::New(_pInvestorPosition->CloseProfitByTrade));
		jsonRtn->Set(GETLOCALSymbol("TodayPosition"), Int32::New(_pInvestorPosition->TodayPosition));
		jsonRtn->Set(GETLOCALSymbol("MarginRateByMoney"), Number::New(_pInvestorPosition->MarginRateByMoney));
		jsonRtn->Set(GETLOCALSymbol("MarginRateByVolume"), Number::New(_pInvestorPosition->MarginRateByVolume));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail = static_cast<CThostFtdcInvestorPositionDetailField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pInvestorPositionDetail->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pInvestorPositionDetail->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pInvestorPositionDetail->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("HedgeFlag"), GETLOCAL(charto_string(pInvestorPositionDetail->HedgeFlag).c_str()));
		jsonRtn->Set(GETLOCALSymbol("Direction"), GETLOCAL(charto_string(pInvestorPositionDetail->Direction).c_str()));
		jsonRtn->Set(GETLOCALSymbol("OpenDate"), GETLOCAL(pInvestorPositionDetail->OpenDate));
		jsonRtn->Set(GETLOCALSymbol("TradeID"), GETLOCAL(pInvestorPositionDetail->TradeID));
		jsonRtn->Set(GETLOCALSymbol("Volume"), Int32::New(pInvestorPositionDetail->Volume));
		jsonRtn->Set(GETLOCALSymbol("OpenPrice"), Number::New(pInvestorPositionDetail->OpenPrice));
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pInvestorPositionDetail->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(pInvestorPositionDetail->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("TradeType"), GETLOCAL(charto_string(pInvestorPositionDetail->TradeType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("CombInstrumentID"), GETLOCAL(pInvestorPositionDetail->CombInstrumentID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pInvestorPositionDetail->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("CloseProfitByDate"), Number::New(pInvestorPositionDetail->CloseProfitByDate));
		jsonRtn->Set(GETLOCALSymbol("CloseProfitByTrade"), Number::New(pInvestorPositionDetail->CloseProfitByTrade));
		jsonRtn->Set(GETLOCALSymbol("PositionProfitByDate"), Number::New(pInvestorPositionDetail->PositionProfitByDate));
		jsonRtn->Set(GETLOCALSymbol("PositionProfitByTrade"), Number::New(pInvestorPositionDetail->PositionProfitByTrade));
		jsonRtn->Set(GETLOCALSymbol("Margin"), Number::New(pInvestorPositionDetail->Margin));
		jsonRtn->Set(GETLOCALSymbol("ExchMargin"), Number::New(pInvestorPositionDetail->ExchMargin));
		jsonRtn->Set(GETLOCALSymbol("MarginRateByMoney"), Number::New(pInvestorPositionDetail->MarginRateByMoney));
		jsonRtn->Set(GETLOCALSymbol("MarginRateByVolume"), Number::New(pInvestorPositionDetail->MarginRateByVolume));
		jsonRtn->Set(GETLOCALSymbol("LastSettlementPrice"), Number::New(pInvestorPositionDetail->LastSettlementPrice));
		jsonRtn->Set(GETLOCALSymbol("SettlementPrice"), Number::New(pInvestorPositionDetail->SettlementPrice));
		jsonRtn->Set(GETLOCALSymbol("CloseVolume"), Int32::New(pInvestorPositionDetail->CloseVolume));
		jsonRtn->Set(GETLOCALSymbol("CloseAmount"), Number::New(pInvestorPositionDetail->CloseAmount));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcTradingAccountField *pTradingAccount = static_cast<CThostFtdcTradingAccountField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pTradingAccount->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("AccountID"), GETLOCAL(pTradingAccount->AccountID));
		jsonRtn->Set(GETLOCALSymbol("PreMortgage"), Number::New(pTradingAccount->PreMortgage));
		jsonRtn->Set(GETLOCALSymbol("PreCredit"), Number::New(pTradingAccount->PreCredit));
		jsonRtn->Set(GETLOCALSymbol("PreDeposit"), Number::New(pTradingAccount->PreDeposit));
		jsonRtn->Set(GETLOCALSymbol("PreBalance"), Number::New(pTradingAccount->PreBalance));
		jsonRtn->Set(GETLOCALSymbol("PreMargin"), Number::New(pTradingAccount->PreMargin));
		jsonRtn->Set(GETLOCALSymbol("InterestBase"), Number::New(pTradingAccount->InterestBase));
		jsonRtn->Set(GETLOCALSymbol("Interest"), Number::New(pTradingAccount->Interest));
		jsonRtn->Set(GETLOCALSymbol("Deposit"), Number::New(pTradingAccount->Deposit));
		jsonRtn->Set(GETLOCALSymbol("Withdraw"), Number::New(pTradingAccount->Withdraw));
		jsonRtn->Set(GETLOCALSymbol("FrozenMargin"), Number::New(pTradingAccount->FrozenMargin));
		jsonRtn->Set(GETLOCALSymbol("FrozenCash"), Number::New(pTradingAccount->FrozenCash));
		jsonRtn->Set(GETLOCALSymbol("FrozenCommission"), Number::New(pTradingAccount->FrozenCommission));
		jsonRtn->Set(GETLOCALSymbol("CurrMargin"), Number::New(pTradingAccount->CurrMargin));
		jsonRtn->Set(GETLOCALSymbol("CashIn"), Number::New(pTradingAccount->CashIn));
		jsonRtn->Set(GETLOCALSymbol("Commission"), Number::New(pTradingAccount->Commission));
		jsonRtn->Set(GETLOCALSymbol("CloseProfit"), Number::New(pTradingAccount->CloseProfit));
		jsonRtn->Set(GETLOCALSymbol("PositionProfit"), Number::New(pTradingAccount->PositionProfit));
		jsonRtn->Set(GETLOCALSymbol("Balance"), Number::New(pTradingAccount->Balance));
		jsonRtn->Set(GETLOCALSymbol("Available"), Number::New(pTradingAccount->Available));
		jsonRtn->Set(GETLOCALSymbol("WithdrawQuota"), Number::New(pTradingAccount->WithdrawQuota));
		jsonRtn->Set(GETLOCALSymbol("Reserve"), Number::New(pTradingAccount->Reserve));
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pTradingAccount->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(pTradingAccount->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("Credit"), Number::New(pTradingAccount->Credit));
		jsonRtn->Set(GETLOCALSymbol("Mortgage"), Number::New(pTradingAccount->Mortgage));
		jsonRtn->Set(GETLOCALSymbol("ExchangeMargin"), Number::New(pTradingAccount->ExchangeMargin));
		jsonRtn->Set(GETLOCALSymbol("DeliveryMargin"), Number::New(pTradingAccount->DeliveryMargin));
		jsonRtn->Set(GETLOCALSymbol("ExchangeDeliveryMargin"), Number::New(pTradingAccount->ExchangeDeliveryMargin));
		jsonRtn->Set(GETLOCALSymbol("ReserveBalance"), Number::New(pTradingAccount->ReserveBalance));
		jsonRtn->Set(GETLOCALSymbol("CurrencyID"), GETLOCAL(pTradingAccount->CurrencyID));
		jsonRtn->Set(GETLOCALSymbol("PreFundMortgageIn"), Number::New(pTradingAccount->PreFundMortgageIn));
		jsonRtn->Set(GETLOCALSymbol("PreFundMortgageOut"), Number::New(pTradingAccount->PreFundMortgageOut));
		jsonRtn->Set(GETLOCALSymbol("FundMortgageIn"), Number::New(pTradingAccount->FundMortgageIn));
		jsonRtn->Set(GETLOCALSymbol("FundMortgageOut"), Number::New(pTradingAccount->FundMortgageOut));
		jsonRtn->Set(GETLOCALSymbol("FundMortgageAvailable"), Number::New(pTradingAccount->FundMortgageAvailable));
		jsonRtn->Set(GETLOCALSymbol("MortgageableFund"), Number::New(pTradingAccount->MortgageableFund));
		jsonRtn->Set(GETLOCALSymbol("SpecProductMargin"), Number::New(pTradingAccount->SpecProductMargin));
		jsonRtn->Set(GETLOCALSymbol("SpecProductFrozenMargin"), Number::New(pTradingAccount->SpecProductFrozenMargin));
		jsonRtn->Set(GETLOCALSymbol("SpecProductCommission"), Number::New(pTradingAccount->SpecProductCommission));
		jsonRtn->Set(GETLOCALSymbol("SpecProductFrozenCommission"), Number::New(pTradingAccount->SpecProductFrozenCommission));
		jsonRtn->Set(GETLOCALSymbol("SpecProductPositionProfit"), Number::New(pTradingAccount->SpecProductPositionProfit));
		jsonRtn->Set(GETLOCALSymbol("SpecProductCloseProfit"), Number::New(pTradingAccount->SpecProductCloseProfit));
		jsonRtn->Set(GETLOCALSymbol("SpecProductPositionProfitByAlg"), Number::New(pTradingAccount->SpecProductPositionProfitByAlg));
		jsonRtn->Set(GETLOCALSymbol("SpecProductExchangeMargin"), Number::New(pTradingAccount->SpecProductExchangeMargin));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInstrumentField *pInstrument = static_cast<CThostFtdcInstrumentField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pInstrument->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pInstrument->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("InstrumentName"), GETLOCAL(pInstrument->InstrumentName));
		jsonRtn->Set(GETLOCALSymbol("ExchangeInstID"), GETLOCAL(pInstrument->ExchangeInstID));
		jsonRtn->Set(GETLOCALSymbol("ProductID"), GETLOCAL(pInstrument->ProductID));
		jsonRtn->Set(GETLOCALSymbol("ProductClass"), GETLOCAL(charto_string(pInstrument->ProductClass).c_str()));
		jsonRtn->Set(GETLOCALSymbol("DeliveryYear"), Int32::New(pInstrument->DeliveryYear));
		jsonRtn->Set(GETLOCALSymbol("DeliveryMonth"), Int32::New(pInstrument->DeliveryMonth));
		jsonRtn->Set(GETLOCALSymbol("MaxMarketOrderVolume"), Int32::New(pInstrument->MaxMarketOrderVolume));
		jsonRtn->Set(GETLOCALSymbol("MinMarketOrderVolume"), Int32::New(pInstrument->MinMarketOrderVolume));
		jsonRtn->Set(GETLOCALSymbol("MaxLimitOrderVolume"), Int32::New(pInstrument->MaxLimitOrderVolume));
		jsonRtn->Set(GETLOCALSymbol("MinLimitOrderVolume"), Int32::New(pInstrument->MinLimitOrderVolume));
		jsonRtn->Set(GETLOCALSymbol("VolumeMultiple"), Int32::New(pInstrument->VolumeMultiple));
		jsonRtn->Set(GETLOCALSymbol("PriceTick"), Number::New(pInstrument->PriceTick));
		jsonRtn->Set(GETLOCALSymbol("CreateDate"), GETLOCAL(pInstrument->CreateDate));
		jsonRtn->Set(GETLOCALSymbol("OpenDate"), GETLOCAL(pInstrument->OpenDate));
		jsonRtn->Set(GETLOCALSymbol("ExpireDate"), GETLOCAL(pInstrument->ExpireDate));
		jsonRtn->Set(GETLOCALSymbol("StartDelivDate"), GETLOCAL(pInstrument->StartDelivDate));
		jsonRtn->Set(GETLOCALSymbol("EndDelivDate"), GETLOCAL(pInstrument->EndDelivDate));
		jsonRtn->Set(GETLOCALSymbol("InstLifePhase"), Int32::New(pInstrument->InstLifePhase));
		jsonRtn->Set(GETLOCALSymbol("IsTrading"), Int32::New(pInstrument->IsTrading));
		jsonRtn->Set(GETLOCALSymbol("PositionType"), GETLOCAL(charto_string(pInstrument->PositionType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("PositionDateType"), GETLOCAL(charto_string(pInstrument->PositionDateType).c_str()));
		jsonRtn->Set(GETLOCALSymbol("LongMarginRatio"), Number::New(pInstrument->LongMarginRatio));
		jsonRtn->Set(GETLOCALSymbol("ShortMarginRatio"), Number::New(pInstrument->ShortMarginRatio));
		jsonRtn->Set(GETLOCALSymbol("MaxMarginSideAlgorithm"), GETLOCAL(charto_string(pInstrument->MaxMarginSideAlgorithm).c_str()));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqdepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pDepthMarketData->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("InstrumentID"), GETLOCAL(pDepthMarketData->InstrumentID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeID"), GETLOCAL(pDepthMarketData->ExchangeID));
		jsonRtn->Set(GETLOCALSymbol("ExchangeInstID"), GETLOCAL(pDepthMarketData->ExchangeInstID));
		jsonRtn->Set(GETLOCALSymbol("LastPrice"), Number::New(pDepthMarketData->LastPrice));
		jsonRtn->Set(GETLOCALSymbol("PreSettlementPrice"), Number::New(pDepthMarketData->PreSettlementPrice));
		jsonRtn->Set(GETLOCALSymbol("PreClosePrice"), Number::New(pDepthMarketData->PreClosePrice));
		jsonRtn->Set(GETLOCALSymbol("PreOpenInterest"), Number::New(pDepthMarketData->PreOpenInterest));
		jsonRtn->Set(GETLOCALSymbol("OpenPrice"), Number::New(pDepthMarketData->OpenPrice));
		jsonRtn->Set(GETLOCALSymbol("HighestPrice"), Number::New(pDepthMarketData->HighestPrice));
		jsonRtn->Set(GETLOCALSymbol("LowestPrice"), Number::New(pDepthMarketData->LowestPrice));
		jsonRtn->Set(GETLOCALSymbol("Volume"), Int32::New(pDepthMarketData->Volume));
		jsonRtn->Set(GETLOCALSymbol("Turnover"), Number::New(pDepthMarketData->Turnover));
		jsonRtn->Set(GETLOCALSymbol("OpenInterest"), Number::New(pDepthMarketData->OpenInterest));
		jsonRtn->Set(GETLOCALSymbol("ClosePrice"), Number::New(pDepthMarketData->ClosePrice));
		jsonRtn->Set(GETLOCALSymbol("SettlementPrice"), Number::New(pDepthMarketData->SettlementPrice));
		jsonRtn->Set(GETLOCALSymbol("UpperLimitPrice"), Number::New(pDepthMarketData->UpperLimitPrice));
		jsonRtn->Set(GETLOCALSymbol("LowerLimitPrice"), Number::New(pDepthMarketData->LowerLimitPrice));
		jsonRtn->Set(GETLOCALSymbol("PreDelta"), Number::New(pDepthMarketData->PreDelta));
		jsonRtn->Set(GETLOCALSymbol("CurrDelta"), Number::New(pDepthMarketData->CurrDelta));
		jsonRtn->Set(GETLOCALSymbol("UpdateTime"), GETLOCAL(pDepthMarketData->UpdateTime));
		jsonRtn->Set(GETLOCALSymbol("UpdateMillisec"), Int32::New(pDepthMarketData->UpdateMillisec));
		jsonRtn->Set(GETLOCALSymbol("BidPrice1"), Number::New(pDepthMarketData->BidPrice1));
		jsonRtn->Set(GETLOCALSymbol("BidVolume1"), Number::New(pDepthMarketData->BidVolume1));
		jsonRtn->Set(GETLOCALSymbol("AskPrice1"), Number::New(pDepthMarketData->AskPrice1));
		jsonRtn->Set(GETLOCALSymbol("AskVolume1"), Number::New(pDepthMarketData->AskVolume1));
		jsonRtn->Set(GETLOCALSymbol("BidPrice2"), Number::New(pDepthMarketData->BidPrice2));
		jsonRtn->Set(GETLOCALSymbol("BidVolume2"), Number::New(pDepthMarketData->BidVolume2));
		jsonRtn->Set(GETLOCALSymbol("AskPrice2"), Number::New(pDepthMarketData->AskPrice2));
		jsonRtn->Set(GETLOCALSymbol("AskVolume2"), Number::New(pDepthMarketData->AskVolume2));
		jsonRtn->Set(GETLOCALSymbol("BidPrice3"), Number::New(pDepthMarketData->BidPrice3));
		jsonRtn->Set(GETLOCALSymbol("BidVolume3"), Number::New(pDepthMarketData->BidVolume3));
		jsonRtn->Set(GETLOCALSymbol("AskPrice3"), Number::New(pDepthMarketData->AskPrice3));
		jsonRtn->Set(GETLOCALSymbol("AskVolume3"), Number::New(pDepthMarketData->AskVolume3));
		jsonRtn->Set(GETLOCALSymbol("BidPrice4"), Number::New(pDepthMarketData->BidPrice4));
		jsonRtn->Set(GETLOCALSymbol("BidVolume4"), Number::New(pDepthMarketData->BidVolume4));
		jsonRtn->Set(GETLOCALSymbol("AskPrice4"), Number::New(pDepthMarketData->AskPrice4));
		jsonRtn->Set(GETLOCALSymbol("AskVolume4"), Number::New(pDepthMarketData->AskVolume4));
		jsonRtn->Set(GETLOCALSymbol("BidPrice5"), Number::New(pDepthMarketData->BidPrice5));
		jsonRtn->Set(GETLOCALSymbol("BidVolume5"), Number::New(pDepthMarketData->BidVolume5));
		jsonRtn->Set(GETLOCALSymbol("AskPrice5"), Number::New(pDepthMarketData->AskPrice5));
		jsonRtn->Set(GETLOCALSymbol("AskVolume5"), Number::New(pDepthMarketData->AskVolume5));
		jsonRtn->Set(GETLOCALSymbol("AveragePrice"), Number::New(pDepthMarketData->AveragePrice));
		jsonRtn->Set(GETLOCALSymbol("ActionDay"), GETLOCAL(pDepthMarketData->ActionDay));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if(data->rtnField!=NULL){
	    CThostFtdcSettlementInfoField *pSettlementInfo = static_cast<CThostFtdcSettlementInfoField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(GETLOCALSymbol("TradingDay"), GETLOCAL(pSettlementInfo->TradingDay));
		jsonRtn->Set(GETLOCALSymbol("SettlementID"), Int32::New(pSettlementInfo->SettlementID));
		jsonRtn->Set(GETLOCALSymbol("BrokerID"), GETLOCAL(pSettlementInfo->BrokerID));
		jsonRtn->Set(GETLOCALSymbol("InvestorID"), GETLOCAL(pSettlementInfo->InvestorID));
		jsonRtn->Set(GETLOCALSymbol("SequenceNo"), Int32::New(pSettlementInfo->SequenceNo));
	    jsonRtn->Set(GETLOCALSymbol("Content"), GETLOCAL(pSettlementInfo->Content));
	    *(cbArray + 2) = jsonRtn;
	}
	else {
	    *(cbArray + 2) = Local<Value>::New(Undefined());
    }
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	*(cbArray + 2) = pkg_rspinfo(data->rspInfo);
	return;
}
Local<Value> WrapTrader::pkg_rspinfo(void *vpRspInfo) {
	if (vpRspInfo) {
        CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(vpRspInfo);
		Local<Object> jsonInfo = Object::New();
		jsonInfo->Set(GETLOCALSymbol("ErrorID"), Int32::New(pRspInfo->ErrorID));
		jsonInfo->Set(GETLOCALSymbol("ErrorMsg"), GETLOCAL(pRspInfo->ErrorMsg));
		return jsonInfo;
	}
	else {
		return 	Local<Value>::New(Undefined());
	}
}
