#include <node.h>
#include "wrap_mduser.h"
#include "defines.h"

std::map<const char*, int,ptrCmp> WrapMdUser::event_map;

WrapMdUser::WrapMdUser(const FunctionCallbackInfo<Value>& args) {
	logger_cout("wrap_mduser------>object start init");
	this->Wrap(args.This());
	uvMdUser = new uv_mduser(this);
	if(args.Length()>0){
		auto opt = Local<Object>::Cast(args[0]);
		auto names = opt->GetOwnPropertyNames();
		auto len = names->Length();
		for (auto i = 0;i < len;i++) {
			Local<String> eventName = names->Get(i)->ToString();
			Local<Function> cb = Local<Function>::Cast(opt->Get(eventName));
		
			String::Utf8Value eNameAscii(eventName);
			auto eIt = event_map.find(*eNameAscii);
			if (eIt == event_map.end()) {
				continue;
			}
			else {
				AddToMap(callback_map, eIt->second, cb);
				uvMdUser->On(*eNameAscii, eIt->second, &WrapMdUser::FunCallback);
			}
		}
	}
	logger_cout("wrap_mduser------>object init successed");
}

WrapMdUser::~WrapMdUser() {
    if(uvMdUser){
	    delete uvMdUser;
    }
	logger_cout("wrape_mduser------>object destroyed");
}
Persistent<Function> WrapMdUser::constructor;
void WrapMdUser::Init(Handle<Object> target) {
	NEW_CONSTR(WrapMdUser);
	initEventMap();
	_WrapMdUser.SetProtoTypeMethod("on", On);
	_WrapMdUser.SetProtoTypeMethod("connect", Connect);
	_WrapMdUser.SetProtoTypeMethod("reqUserLogin", ReqUserLogin);
	_WrapMdUser.SetProtoTypeMethod("reqUserLogout", ReqUserLogout);
	_WrapMdUser.SetProtoTypeMethod("subscribeMarketData", SubscribeMarketData);
	_WrapMdUser.SetProtoTypeMethod("unSubscribeMarketData", UnSubscribeMarketData);
	_WrapMdUser.SetProtoTypeMethod("disconnect", Disposed);
	NODE_SET_METHOD(target, "createMdUser", CreateCObject<WrapMdUser>);
}

void WrapMdUser::initEventMap() {
	event_map["connect"] = T_ON_CONNECT;
	event_map["disconnected"] = T_ON_DISCONNECTED;
	event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
	event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
	event_map["rspSubMarketData"] = T_ON_RSPSUBMARKETDATA;
	event_map["rspUnSubMarketData"] = T_ON_RSPUNSUBMARKETDATA;
	event_map["rtnDepthMarketData"] = T_ON_RTNDEPTHMARKETDATA;
	event_map["rspError"] = T_ON_RSPERROR;
}
FUNCTIONCALLBACK(WrapMdUser::On)
	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		logger_cout("Wrong arguments->event name or function");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->event name or function")));
		return;
	}

	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());

	Local<String> eventName = args[0]->ToString();
	Local<Function> cb = Local<Function>::Cast(args[1]);
	String::Utf8Value eNameAscii(eventName);
	
	auto eIt = event_map.find(*eNameAscii);
	if (eIt == event_map.end()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("System has no register this event")));
		return;
	}
	auto cIt = obj->callback_map.find(eIt->second);
	if (cIt != obj->callback_map.end()) {
		logger_cout("Callback is defined before");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Callback is defined before")));
		args.GetReturnValue().SetUndefined();
		return;
	}
	AddToMap(obj->callback_map, eIt->second, cb);
	obj->uvMdUser->On(*eNameAscii,eIt->second, &WrapMdUser::FunCallback);
	args.GetReturnValue().Set(Int32::New(isolate, 0));
}
FUNCTIONCALLBACK(WrapMdUser::Connect)
	std::string log = "wrap_mduser Connect------>";
	if (args[0]->IsUndefined()) {
		logger_cout("Wrong arguments->front addr");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments->front addr")));
		return;
	}	
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++obj->s_uuid;
		AddToMap(obj->fun_rtncb_map, uuid, Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> frontAddr = args[0]->ToString();
	Local<String> szPath = args[1]->IsUndefined() ? GETLOCAL("m") : args[1]->ToString();

	UVConnectField pConnectField;
	memset(&pConnectField, 0, sizeof(pConnectField));
	frontAddr->WriteOneByte((uint8_t*)pConnectField.front_addr);
	szPath->WriteOneByte((uint8_t*)pConnectField.szPath);

	logger_cout(log.append(" ").append(pConnectField.front_addr).append("|").append(pConnectField.szPath).append("|").c_str());
	obj->uvMdUser->Connect(&pConnectField, &WrapMdUser::FunRtnCallback, uuid);
}
FUNCTIONCALLBACK(WrapMdUser::ReqUserLogin)
	std::string log = "wrap_mduser ReqUserLogin------>";
	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}

	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++obj->s_uuid;
		AddToMap(obj->fun_rtncb_map,uuid,Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.UserID, req.Password);
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.UserID).append("|").append(req.Password).c_str());
	obj->uvMdUser->ReqUserLogin(&req, &WrapMdUser::FunRtnCallback, uuid);
}
FUNCTIONCALLBACK(WrapMdUser::ReqUserLogout)
	std::string log = "wrap_mduser ReqUserLogout------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++obj->s_uuid;
		AddToMap(obj->fun_rtncb_map,uuid,Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.UserID);
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.UserID).c_str());
	obj->uvMdUser->ReqUserLogout(&req, &WrapMdUser::FunRtnCallback, uuid);
}
FUNCTIONCALLBACK(WrapMdUser::SubscribeMarketData)
	std::string log = "wrap_mduser SubscribeMarketData------>";

	if (args[0]->IsUndefined() || !args[0]->IsArray()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++obj->s_uuid;
		AddToMap(obj->fun_rtncb_map,uuid,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	} 
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];
	
	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		char* id = new char[instrumentId->Length() + 1];
		instrumentId->WriteOneByte((uint8_t*)id);
		idArray[i] = id;
		log.append(id).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->SubscribeMarketData(idArray, instrumentIDs->Length(), &WrapMdUser::FunRtnCallback, uuid);
	delete idArray;
}
FUNCTIONCALLBACK(WrapMdUser::UnSubscribeMarketData)
	std::string log = "wrap_mduser UnSubscribeMarketData------>";

	if (args[0]->IsUndefined() || !args[0]->IsArray()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Wrong arguments")));
		return;
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++obj->s_uuid;
		AddToMap(obj->fun_rtncb_map,uuid,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];

	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		char* id = new char[instrumentId->Length() + 1];
		idArray[i] = id;
		instrumentId->WriteOneByte((uint8_t*)id);
		log.append(id).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->UnSubscribeMarketData(idArray, instrumentIDs->Length(), &WrapMdUser::FunRtnCallback, uuid);
}
FUNCTIONCALLBACK(WrapMdUser::Disposed)
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
obj->uvMdUser->Disposed();

	auto callback_it = obj->callback_map.begin();
	while (callback_it != obj->callback_map.end()) {
		callback_it->second.Reset();
		callback_it++;
	}
	event_map.clear();
	obj->callback_map.clear();
	obj->fun_rtncb_map.clear();
	delete obj->uvMdUser;
    obj->uvMdUser = NULL;
	logger_cout("wrap_mduser Disposed------>wrap disposed");
}


////////////////////////////////////////////////////////////////////////////////////////////////

void WrapMdUser::FunCallback(CbRtnField *data) {
	CSCOPE
	auto cIt = callback_map.find(data->eFlag);
	if (cIt == callback_map.end())
		return;
#define FunCallback_Switch(type,count,...) case type:{\
 Local<Value> argv[count]\
__VA_ARGS__\
 cIt->second.Get(isolate)->Call(handle(), count, argv);\
break;\
	}
	switch (data->eFlag) {
		FunCallback_Switch(T_ON_CONNECT, 1, ={ Undefined(isolate) };)
		FunCallback_Switch(T_ON_DISCONNECTED, 1, ={ GETLOCAL(data->nReason) };)
		FunCallback_Switch(T_ON_RSPUSERLOGIN, 4, ; pkg_cb_userlogin(data, argv);)
		FunCallback_Switch(T_ON_RSPUSERLOGOUT, 4, ; pkg_cb_userlogout(data, argv);)
		FunCallback_Switch(T_ON_RSPSUBMARKETDATA, 4, ; pkg_cb_rspsubmarketdata(data, argv);)
		FunCallback_Switch(T_ON_RSPUNSUBMARKETDATA, 4, ; pkg_cb_unrspsubmarketdata(data, argv);)
		FunCallback_Switch(T_ON_RTNDEPTHMARKETDATA, 1, ;pkg_cb_rtndepthmarketdata(data, argv);)
		FunCallback_Switch(T_ON_RSPERROR, 3, ;pkg_cb_rsperror(data, argv);)
	}
#undef FunCallback_Switch
}
void WrapMdUser::FunRtnCallback(int result, void* baton) {
	CSCOPE
	LookupCtpApiBaton<WrapMdUser>* tmp = static_cast<LookupCtpApiBaton<WrapMdUser>*>(baton);
	if (tmp->uuid != -1) {
		auto it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[1] = {GETLOCAL(tmp->nResult) };
		it->second.Get(isolate)->Call(handle(), 1, argv);
		it->second.Reset();
		fun_rtncb_map.erase(tmp->uuid);
	}
}
void WrapMdUser::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
	CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pRspUserLogin) {
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

	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
	CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pRspUserLogin) {
		Local<Object> jsonRtn = Object::New(isolate);
		SetObjectProperty(jsonRtn, isolate, "BrokerID", pRspUserLogin->BrokerID);
		SetObjectProperty(jsonRtn, isolate, "UserID", pRspUserLogin->UserID);
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_rspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
	CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSpecificInstrument) {
		Local<Object> jsonRtn = Object::New(isolate);
		jsonRtn->Set(GETLOCAL("InstrumentID"), GETLOCAL(pSpecificInstrument->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_unrspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
	CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSpecificInstrument) {
		Local<Object> jsonRtn = Object::New(isolate);
		jsonRtn->Set(GETLOCAL("InstrumentID"), GETLOCAL(pSpecificInstrument->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined(isolate);
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_rtndepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	CSCOPE
	CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
	if (pDepthMarketData) {	   		
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
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined(isolate);
	}
	
	return;
}  
void WrapMdUser::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	*cbArray = GETLOCAL(data->nRequestID);
	*(cbArray + 1) = GETLOCAL(data->bIsLast);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	*(cbArray + 2) = pkg_rspinfo(pRspInfo);
	return;
}
Local<Value> WrapMdUser::pkg_rspinfo(CThostFtdcRspInfoField *pRspInfo) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	if (pRspInfo) {
		Local<Object> jsonInfo = Object::New(isolate);
		jsonInfo->Set(GETLOCAL("ErrorID"), GETLOCAL(pRspInfo->ErrorID));
		jsonInfo->Set(GETLOCAL("ErrorMsg"), GETLOCAL(pRspInfo->ErrorMsg));
		return jsonInfo;
	}
	else {
		return  Undefined(isolate);
	}
}
