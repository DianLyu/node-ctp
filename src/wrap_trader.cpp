#include <node.h>
#include "wrap_trader.h"
#include "defines.h"

int WrapTrader::s_uuid;
std::map<const char*, int,ptrCmp> WrapTrader::event_map;
std::map<int, Persistent<Function, CopyablePersistentTraits<Function>> > WrapTrader::callback_map;
std::map<int, Persistent<Function, CopyablePersistentTraits<Function>> > WrapTrader::fun_rtncb_map;

WrapTrader::WrapTrader(const FunctionCallbackInfo<Value>& args) {
	logger_cout("wrap_trader------>object start init");
	this->Wrap(args.This());
	uvTrader = new uv_trader();	
	logger_cout("wrap_trader------>object init successed");
}

WrapTrader::~WrapTrader(void) {
    if(uvTrader){
	    delete uvTrader;
    }
	logger_cout("wrap_trader------>object destroyed");
}
Persistent<Function> WrapTrader::constructor;
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
	NODE_SET_METHOD(target, "createTrader", CreateCObject<WrapTrader>);
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

	auto cIt = callback_map.find(eIt->second);
	if (cIt != callback_map.end()) {
		logger_cout("Callback is defined before");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Callback is defined before")));
		return;
	}
	AddToMap(callback_map, eIt->second, cb);
	//callback_map[eIt->second] = Persistent<Function>(isolate,cb);
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
		AddToMap(fun_rtncb_map, uuid, Local<Function>::Cast(args[4]));
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
		AddToMap(fun_rtncb_map, uuid, Local<Function>::Cast(args[3]));
		//AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[3]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[2]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[2]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[1]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[2]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[3]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[3]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[1]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[1]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[4]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[1]));
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
		AddToMap(fun_rtncb_map,uuid,Local<Function>::Cast(args[3]));
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
	auto callback_it = callback_map.begin();
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
	auto cIt = callback_map.find(data->eFlag);
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
		FunCallback_Switch(T_ON_RSPUSERLOGOUT, 3, ; pkg_cb_rsperror(data, argv);)
		FunCallback_Switch(T_ON_RSPINFOCONFIRM, 4, ;pkg_cb_confirm(data, argv);)
		FunCallback_Switch(T_ON_RSPINSERT, 4, ; pkg_cb_orderinsert(data, argv);)
		FunCallback_Switch(T_ON_ERRINSERT, 2, ; pkg_cb_errorderinsert(data, argv);)
		FunCallback_Switch(T_ON_RSPACTION, 4, ; pkg_cb_orderaction(data, argv);)
		FunCallback_Switch(T_ON_ERRACTION, 2, ; pkg_cb_errorderaction(data, argv);)
		FunCallback_Switch(T_ON_RQORDER, 4, ; pkg_cb_rspqryorder(data, argv);)
		FunCallback_Switch(T_ON_RTNORDER, 1, ; pkg_cb_rsperror(data, argv);)
		FunCallback_Switch(T_ON_RQTRADE, 4, ; pkg_cb_rqtrade(data, argv);)
		FunCallback_Switch(T_ON_RTNTRADE, 1, ; pkg_cb_rtntrade(data, argv);)
		FunCallback_Switch(T_ON_RQINVESTORPOSITION, 4, ; pkg_cb_rqinvestorposition(data, argv);)
		FunCallback_Switch(T_ON_RQINVESTORPOSITIONDETAIL, 4, ; pkg_cb_rqinvestorpositiondetail(data, argv);)
		FunCallback_Switch(T_ON_RQTRADINGACCOUNT,4, ; pkg_cb_rqtradingaccount(data, argv);)
		FunCallback_Switch(T_ON_RQINSTRUMENT, 4, ; pkg_cb_rqinstrument(data, argv);)
		FunCallback_Switch(T_ON_RQDEPTHMARKETDATA, 4, ; pkg_cb_rqdepthmarketdata(data, argv);)
		FunCallback_Switch(T_ON_RQSETTLEMENTINFO, 4, ; pkg_cb_rqsettlementinfo(data, argv);)
		FunCallback_Switch(T_ON_RSPERROR, 3, ; pkg_cb_rsperror(data, argv);)
	}
#undef FunCallback_Switch
}

void WrapTrader::FunRtnCallback(int result, void* baton) {
	CSCOPE
	LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);	 
	if (tmp->uuid != -1) {
		auto it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[2] = { GETLOCAL(tmp->nResult),GETLOCAL(tmp->iRequestID) };
		it->second.Get(isolate)->Call(isolate->GetCurrentContext()->Global(), 2, argv);
		it->second.Reset();
		fun_rtncb_map.erase(tmp->uuid);	  		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WrapTrader::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
	if (data->rtnField){  
	    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pRspUserLogin->x);
jsonRtnSet(TradingDay)
jsonRtnSet(LoginTime)
jsonRtnSet(BrokerID)
jsonRtnSet(UserID)
jsonRtnSet(SystemName)
jsonRtnSet(FrontID)
jsonRtnSet(SessionID)
jsonRtnSet(MaxOrderRef)
jsonRtnSet(SHFETime)
jsonRtnSet(DCETime)
jsonRtnSet(CZCETime)
jsonRtnSet(FFEXTime)
jsonRtnSet(INETime)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pRspUserLogin->x);
jsonRtnSet(BrokerID)
jsonRtnSet(UserID)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm = static_cast<CThostFtdcSettlementInfoConfirmField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pSettlementInfoConfirm->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(ConfirmDate)
jsonRtnSet(ConfirmTime)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pInputOrder->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(InstrumentID)
jsonRtnSet(OrderRef)
jsonRtnSet(UserID)
jsonRtnSet(OrderPriceType)
jsonRtnSet(Direction)
jsonRtnSet(CombOffsetFlag)
jsonRtnSet(CombHedgeFlag)
jsonRtnSet(LimitPrice)
jsonRtnSet(VolumeTotalOriginal)
jsonRtnSet(TimeCondition)
jsonRtnSet(GTDDate)
jsonRtnSet(VolumeCondition)
jsonRtnSet(MinVolume)
jsonRtnSet(ContingentCondition)
jsonRtnSet(StopPrice)
jsonRtnSet(ForceCloseReason)
jsonRtnSet(IsAutoSuspend)
jsonRtnSet(BusinessUnit)
jsonRtnSet(RequestID)
jsonRtnSet(UserForceClose)
jsonRtnSet(IsSwapOrder)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
    if (data->rtnField){ 
	    CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pInputOrder->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(InstrumentID)
jsonRtnSet(OrderRef)
jsonRtnSet(UserID)
jsonRtnSet(OrderPriceType)
jsonRtnSet(Direction)
jsonRtnSet(CombOffsetFlag)
jsonRtnSet(CombHedgeFlag)
jsonRtnSet(LimitPrice)
jsonRtnSet(VolumeTotalOriginal)
jsonRtnSet(TimeCondition)
jsonRtnSet(GTDDate)
jsonRtnSet(VolumeCondition)
jsonRtnSet(MinVolume)
jsonRtnSet(ContingentCondition)
jsonRtnSet(StopPrice)
jsonRtnSet(ForceCloseReason)
jsonRtnSet(IsAutoSuspend)
jsonRtnSet(BusinessUnit)
jsonRtnSet(RequestID)
jsonRtnSet(UserForceClose)
jsonRtnSet(IsSwapOrder)
#undef jsonRtnSet
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined(isolate);
	}
	*(cbArray + 1) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcInputOrderActionField* pInputOrderAction = static_cast<CThostFtdcInputOrderActionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pInputOrderAction->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(OrderActionRef)
jsonRtnSet(OrderRef)
jsonRtnSet(RequestID)
jsonRtnSet(FrontID)
jsonRtnSet(SessionID)
jsonRtnSet(ExchangeID)
jsonRtnSet(OrderSysID)
jsonRtnSet(ActionFlag)
jsonRtnSet(LimitPrice)
jsonRtnSet(VolumeChange)
jsonRtnSet(UserID)
jsonRtnSet(InstrumentID)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
    if (data->rtnField){ 
	    CThostFtdcOrderActionField* pOrderAction = static_cast<CThostFtdcOrderActionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pOrderAction->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(OrderActionRef)
jsonRtnSet(OrderRef)
jsonRtnSet(RequestID)
jsonRtnSet(FrontID)
jsonRtnSet(SessionID)
jsonRtnSet(ExchangeID)
jsonRtnSet(OrderSysID)
jsonRtnSet(ActionFlag)
jsonRtnSet(LimitPrice)
jsonRtnSet(VolumeChange)
jsonRtnSet(ActionDate)
jsonRtnSet(TraderID)
jsonRtnSet(InstallID)
jsonRtnSet(OrderLocalID)
jsonRtnSet(ActionLocalID)
jsonRtnSet(ParticipantID)
jsonRtnSet(ClientID)
jsonRtnSet(BusinessUnit)
jsonRtnSet(OrderActionStatus)
jsonRtnSet(UserID)
jsonRtnSet(StatusMsg)
jsonRtnSet(InstrumentID)
#undef jsonRtnSet
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined(isolate);
	}
	*(cbArray + 1) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pOrder->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(InstrumentID)
jsonRtnSet(OrderRef)
jsonRtnSet(UserID)
jsonRtnSet(OrderPriceType)
jsonRtnSet(Direction)
jsonRtnSet(CombOffsetFlag)
jsonRtnSet(CombHedgeFlag)
jsonRtnSet(LimitPrice)
jsonRtnSet(VolumeTotalOriginal)
jsonRtnSet(TimeCondition)
jsonRtnSet(GTDDate)
jsonRtnSet(VolumeCondition)
jsonRtnSet(MinVolume)
jsonRtnSet(ContingentCondition)
jsonRtnSet(StopPrice)
jsonRtnSet(ForceCloseReason)
jsonRtnSet(IsAutoSuspend)
jsonRtnSet(BusinessUnit)
jsonRtnSet(RequestID)
jsonRtnSet(OrderLocalID)
jsonRtnSet(ExchangeID)
jsonRtnSet(ParticipantID)
jsonRtnSet(ClientID)
jsonRtnSet(ExchangeInstID)
jsonRtnSet(TraderID)
jsonRtnSet(InstallID)
jsonRtnSet(OrderSubmitStatus)
jsonRtnSet(NotifySequence)
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(OrderSysID)
jsonRtnSet(OrderSource)
jsonRtnSet(OrderStatus)
jsonRtnSet(OrderType)
jsonRtnSet(VolumeTraded)
jsonRtnSet(VolumeTotal)
jsonRtnSet(InsertDate)
jsonRtnSet(InsertTime)
jsonRtnSet(ActiveTime)
jsonRtnSet(SuspendTime)
jsonRtnSet(UpdateTime)
jsonRtnSet(CancelTime)
jsonRtnSet(ActiveTraderID)
jsonRtnSet(ClearingPartID)
jsonRtnSet(SequenceNo)
jsonRtnSet(FrontID)
jsonRtnSet(SessionID)
jsonRtnSet(UserProductInfo)
jsonRtnSet(StatusMsg)
jsonRtnSet(UserForceClose)
jsonRtnSet(ActiveUserID)
jsonRtnSet(BrokerOrderSeq)
jsonRtnSet(RelativeOrderSysID)
jsonRtnSet(ZCETotalTradedVolume)
jsonRtnSet(IsSwapOrder)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
    if (data->rtnField){ 
	    CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pOrder->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(InstrumentID)
jsonRtnSet(OrderRef)
jsonRtnSet(UserID)
jsonRtnSet(OrderPriceType)
jsonRtnSet(Direction)
jsonRtnSet(CombOffsetFlag)
jsonRtnSet(CombHedgeFlag)
jsonRtnSet(LimitPrice)
jsonRtnSet(VolumeTotalOriginal)
jsonRtnSet(TimeCondition)
jsonRtnSet(GTDDate)
jsonRtnSet(VolumeCondition)
jsonRtnSet(MinVolume)
jsonRtnSet(ContingentCondition)
jsonRtnSet(StopPrice)
jsonRtnSet(ForceCloseReason)
jsonRtnSet(IsAutoSuspend)
jsonRtnSet(BusinessUnit)
jsonRtnSet(RequestID)
jsonRtnSet(OrderLocalID)
jsonRtnSet(ExchangeID)
jsonRtnSet(ParticipantID)
jsonRtnSet(ClientID)
jsonRtnSet(ExchangeInstID)
jsonRtnSet(TraderID)
jsonRtnSet(InstallID)
jsonRtnSet(OrderSubmitStatus)
jsonRtnSet(NotifySequence)
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(OrderSysID)
jsonRtnSet(OrderSource)
jsonRtnSet(OrderStatus)
jsonRtnSet(OrderType)
jsonRtnSet(VolumeTraded)
jsonRtnSet(VolumeTotal)
jsonRtnSet(InsertDate)
jsonRtnSet(InsertTime)
jsonRtnSet(ActiveTime)
jsonRtnSet(SuspendTime)
jsonRtnSet(UpdateTime)
jsonRtnSet(CancelTime)
jsonRtnSet(ActiveTraderID)
jsonRtnSet(ClearingPartID)
jsonRtnSet(SequenceNo)
jsonRtnSet(FrontID)
jsonRtnSet(SessionID)
jsonRtnSet(UserProductInfo)
jsonRtnSet(StatusMsg)
jsonRtnSet(UserForceClose)
jsonRtnSet(ActiveUserID)
jsonRtnSet(BrokerOrderSeq)
jsonRtnSet(RelativeOrderSysID)
jsonRtnSet(ZCETotalTradedVolume)
jsonRtnSet(IsSwapOrder)
#undef jsonRtnSet
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined(isolate);
	}
	return;
}
void WrapTrader::pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pTrade->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(InstrumentID)
jsonRtnSet(OrderRef)
jsonRtnSet(UserID)
jsonRtnSet(ExchangeID)
jsonRtnSet(TradeID)
jsonRtnSet(Direction)
jsonRtnSet(OrderSysID)
jsonRtnSet(ParticipantID)
jsonRtnSet(ClientID)
jsonRtnSet(TradingRole)
jsonRtnSet(ExchangeInstID)
jsonRtnSet(OffsetFlag)
jsonRtnSet(HedgeFlag)
jsonRtnSet(Price)
jsonRtnSet(Volume)
jsonRtnSet(TradeDate)
jsonRtnSet(TradeTime)
jsonRtnSet(TradeType)
jsonRtnSet(PriceSource)
jsonRtnSet(TraderID)
jsonRtnSet(OrderLocalID)
jsonRtnSet(ClearingPartID)
jsonRtnSet(BusinessUnit)
jsonRtnSet(SequenceNo)
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(BrokerOrderSeq)
jsonRtnSet(TradeSource)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
    if (data->rtnField){ 
	    CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pTrade->x);
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(InstrumentID)
jsonRtnSet(OrderRef)
jsonRtnSet(UserID)
jsonRtnSet(ExchangeID)
jsonRtnSet(TradeID)
jsonRtnSet(Direction)
jsonRtnSet(OrderSysID)
jsonRtnSet(ParticipantID)
jsonRtnSet(ClientID)
jsonRtnSet(TradingRole)
jsonRtnSet(ExchangeInstID)
jsonRtnSet(OffsetFlag)
jsonRtnSet(HedgeFlag)
jsonRtnSet(Price)
jsonRtnSet(Volume)
jsonRtnSet(TradeDate)
jsonRtnSet(TradeTime)
jsonRtnSet(TradeType)
jsonRtnSet(PriceSource)
jsonRtnSet(TraderID)
jsonRtnSet(OrderLocalID)
jsonRtnSet(ClearingPartID)
jsonRtnSet(BusinessUnit)
jsonRtnSet(SequenceNo)
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(BrokerOrderSeq)
jsonRtnSet(TradeSource)
#undef jsonRtnSet
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined(isolate);
	}
	 
	return;
}
void WrapTrader::pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcInvestorPositionField* _pInvestorPosition = static_cast<CThostFtdcInvestorPositionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, _pInvestorPosition->x);
jsonRtnSet(InstrumentID)
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID) 
jsonRtnSet(PosiDirection)
jsonRtnSet(HedgeFlag)
jsonRtnSet(PositionDate)
jsonRtnSet(YdPosition)
jsonRtnSet(Position)
jsonRtnSet(LongFrozen)
jsonRtnSet(ShortFrozen)
jsonRtnSet(LongFrozenAmount)
jsonRtnSet(ShortFrozenAmount)
jsonRtnSet(OpenVolume)
jsonRtnSet(CloseVolume)
jsonRtnSet(OpenAmount)
jsonRtnSet(CloseAmount)
jsonRtnSet(PositionCost)
jsonRtnSet(PreMargin)
jsonRtnSet(UseMargin)
jsonRtnSet(FrozenMargin)
jsonRtnSet(FrozenCash)
jsonRtnSet(FrozenCommission)
jsonRtnSet(CashIn)
jsonRtnSet(Commission)
jsonRtnSet(CloseProfit)
jsonRtnSet(PositionProfit)
jsonRtnSet(PreSettlementPrice)
jsonRtnSet(SettlementPrice)
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(OpenCost)
jsonRtnSet(ExchangeMargin)
jsonRtnSet(CombPosition)
jsonRtnSet(CombLongFrozen)
jsonRtnSet(CombShortFrozen)
jsonRtnSet(CloseProfitByDate)
jsonRtnSet(CloseProfitByTrade)
jsonRtnSet(TodayPosition)
jsonRtnSet(MarginRateByMoney)
jsonRtnSet(MarginRateByVolume)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail = static_cast<CThostFtdcInvestorPositionDetailField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pInvestorPositionDetail->x);
jsonRtnSet(InstrumentID)
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(HedgeFlag)
jsonRtnSet(Direction)
jsonRtnSet(OpenDate)
jsonRtnSet(TradeID)
jsonRtnSet(Volume)
jsonRtnSet(OpenPrice)
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(TradeType)
jsonRtnSet(CombInstrumentID)
jsonRtnSet(ExchangeID)
jsonRtnSet(CloseProfitByDate)
jsonRtnSet(CloseProfitByTrade)
jsonRtnSet(PositionProfitByDate)
jsonRtnSet(PositionProfitByTrade)
jsonRtnSet(Margin)
jsonRtnSet(ExchMargin)
jsonRtnSet(MarginRateByMoney)
jsonRtnSet(MarginRateByVolume)
jsonRtnSet(LastSettlementPrice)
jsonRtnSet(SettlementPrice)
jsonRtnSet(CloseVolume)
jsonRtnSet(CloseAmount)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcTradingAccountField *pTradingAccount = static_cast<CThostFtdcTradingAccountField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pTradingAccount->x);
jsonRtnSet(BrokerID)
jsonRtnSet(AccountID)
jsonRtnSet(PreMortgage)
jsonRtnSet(PreCredit)
jsonRtnSet(PreDeposit)
jsonRtnSet(PreBalance)
jsonRtnSet(PreMargin)
jsonRtnSet(InterestBase)
jsonRtnSet(Interest)
jsonRtnSet(Deposit)
jsonRtnSet(Withdraw)
jsonRtnSet(FrozenMargin)
jsonRtnSet(FrozenCash)
jsonRtnSet(FrozenCommission)
jsonRtnSet(CurrMargin)
jsonRtnSet(CashIn)
jsonRtnSet(Commission)
jsonRtnSet(CloseProfit)
jsonRtnSet(PositionProfit)
jsonRtnSet(Balance)
jsonRtnSet(Available)
jsonRtnSet(WithdrawQuota)
jsonRtnSet(Reserve)
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(Credit)
jsonRtnSet(Mortgage)
jsonRtnSet(ExchangeMargin)
jsonRtnSet(DeliveryMargin)
jsonRtnSet(ExchangeDeliveryMargin)
jsonRtnSet(ReserveBalance)
jsonRtnSet(CurrencyID)
jsonRtnSet(PreFundMortgageIn)
jsonRtnSet(PreFundMortgageOut)
jsonRtnSet(FundMortgageIn)
jsonRtnSet(FundMortgageOut)
jsonRtnSet(FundMortgageAvailable)
jsonRtnSet(MortgageableFund)
jsonRtnSet(SpecProductMargin)
jsonRtnSet(SpecProductFrozenMargin)
jsonRtnSet(SpecProductCommission)
jsonRtnSet(SpecProductFrozenCommission)
jsonRtnSet(SpecProductPositionProfit)
jsonRtnSet(SpecProductCloseProfit)
jsonRtnSet(SpecProductPositionProfitByAlg)
jsonRtnSet(SpecProductExchangeMargin)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcInstrumentField *pInstrument = static_cast<CThostFtdcInstrumentField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pInstrument->x);
jsonRtnSet(InstrumentID)
jsonRtnSet(ExchangeID)
jsonRtnSet(InstrumentName)
jsonRtnSet(ExchangeInstID)
jsonRtnSet(ProductID)
jsonRtnSet(ProductClass)
jsonRtnSet(DeliveryYear)
jsonRtnSet(DeliveryMonth)
jsonRtnSet(MaxMarketOrderVolume)
jsonRtnSet(MinMarketOrderVolume)
jsonRtnSet(MaxLimitOrderVolume)
jsonRtnSet(MinLimitOrderVolume)
jsonRtnSet(VolumeMultiple)
jsonRtnSet(PriceTick)
jsonRtnSet(CreateDate)
jsonRtnSet(OpenDate)
jsonRtnSet(ExpireDate)
jsonRtnSet(StartDelivDate)
jsonRtnSet(EndDelivDate)
jsonRtnSet(InstLifePhase)
jsonRtnSet(IsTrading)
jsonRtnSet(PositionType)
jsonRtnSet(PositionDateType)
jsonRtnSet(LongMarginRatio)
jsonRtnSet(ShortMarginRatio)
jsonRtnSet(MaxMarginSideAlgorithm)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqdepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if (data->rtnField){ 
	    CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pDepthMarketData->x);
jsonRtnSet(TradingDay)
jsonRtnSet(InstrumentID)
jsonRtnSet(ExchangeID)
jsonRtnSet(ExchangeInstID)
jsonRtnSet(LastPrice)
jsonRtnSet(PreSettlementPrice)
jsonRtnSet(PreClosePrice)
jsonRtnSet(PreOpenInterest)
jsonRtnSet(OpenPrice)
jsonRtnSet(HighestPrice)
jsonRtnSet(LowestPrice)
jsonRtnSet(Volume)
jsonRtnSet(Turnover)
jsonRtnSet(OpenInterest)
jsonRtnSet(ClosePrice)
jsonRtnSet(SettlementPrice)
jsonRtnSet(UpperLimitPrice)
jsonRtnSet(LowerLimitPrice)
jsonRtnSet(PreDelta)
jsonRtnSet(CurrDelta)
jsonRtnSet(UpdateTime)
jsonRtnSet(UpdateMillisec)
jsonRtnSet(BidPrice1)
jsonRtnSet(BidVolume1)
jsonRtnSet(AskPrice1)
jsonRtnSet(AskVolume1)
jsonRtnSet(BidPrice2)
jsonRtnSet(BidVolume2)
jsonRtnSet(AskPrice2)
jsonRtnSet(AskVolume2)
jsonRtnSet(BidPrice3)
jsonRtnSet(BidVolume3)
jsonRtnSet(AskPrice3)
jsonRtnSet(AskVolume3)
jsonRtnSet(BidPrice4)
jsonRtnSet(BidVolume4)
jsonRtnSet(AskPrice4)
jsonRtnSet(AskVolume4)
jsonRtnSet(BidPrice5)
jsonRtnSet(BidVolume5)
jsonRtnSet(AskPrice5)
jsonRtnSet(AskVolume5)
jsonRtnSet(AveragePrice)
jsonRtnSet(ActionDay)
#undef jsonRtnSet
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
    if(data->rtnField!=NULL){
	    CThostFtdcSettlementInfoField *pSettlementInfo = static_cast<CThostFtdcSettlementInfoField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New(isolate);
#define jsonRtnSet(x) SetObjectProperty(jsonRtn, isolate, #x, pSettlementInfo->x);
jsonRtnSet(TradingDay)
jsonRtnSet(SettlementID)
jsonRtnSet(BrokerID)
jsonRtnSet(InvestorID)
jsonRtnSet(SequenceNo)
jsonRtnSet(Content)
#undef jsonRtnSet
	    *(cbArray + 2) = jsonRtn;
	}
	else {
	    *(cbArray + 2) = Undefined(isolate);
    }
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
	*(cbArray + 2) = pkg_rspinfo(data->rspInfo);
	return;
}
Local<Value> WrapTrader::pkg_rspinfo(void *vpRspInfo) {
	CSCOPE
	if (vpRspInfo) {
        CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(vpRspInfo);
		Local<Object> jsonInfo = Object::New(isolate);
		SetObjectProperty(jsonInfo, isolate, "ErrorID", pRspInfo->ErrorID);
		SetObjectProperty(jsonInfo, isolate, "ErrorMsg", pRspInfo->ErrorMsg);
		return jsonInfo;
	}
	else {
		return 	Undefined(isolate);
	}
}
