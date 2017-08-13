#include <node.h>
#include "wrap_mduser.h"
#include "defines.h"
DECL_CONSTR(WrapMdUser)
int WrapMdUser::s_uuid;
std::map<const char*, int,ptrCmp> WrapMdUser::event_map;
std::map<int, Persistent<Function> > WrapMdUser::callback_map;
std::map<int, Persistent<Function> > WrapMdUser::fun_rtncb_map;

WrapMdUser::WrapMdUser() {
	logger_cout("wrap_mduser------>object start init");
	uvMdUser = new uv_mduser();
	logger_cout("wrap_mduser------>object init successed");
}

WrapMdUser::~WrapMdUser() {
    if(uvMdUser){
	    delete uvMdUser;
    }
	logger_cout("wrape_mduser------>object destroyed");
}

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
	NODE_SET_METHOD(target, "createTrader", CreateCObject<WrapMdUser>);
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
	//String::AsciiValue eNameAscii(eventName);
	String::Utf8Value eNameAscii(eventName);
	std::map<const char*, int>::iterator eIt = event_map.find(*eNameAscii);
	if (eIt == event_map.end()) {
		isolate->ThrowException(Exception::TypeError(GETLOCAL("System has no register this event")));
		return;
	}
	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(eIt->second);
	if (cIt != callback_map.end()) {
		logger_cout("Callback is defined before");
		isolate->ThrowException(Exception::TypeError(GETLOCAL("Callback is defined before")));
		args.GetReturnValue().SetUndefined();
		return;
	}
	callback_map[eIt->second] = Persistent<Function>(isolate,cb);
	obj->uvMdUser->On(*eNameAscii,eIt->second, FunCallback);
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
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> frontAddr = args[0]->ToString();
	Local<String> szPath = args[1]->IsUndefined() ? GETLOCAL("m") : args[1]->ToString();
	//String::Utf8Value addrAscii(frontAddr);
	//String::Utf8Value pathAscii(szPath);

	UVConnectField pConnectField;
	memset(&pConnectField, 0, sizeof(pConnectField));
	frontAddr->WriteOneByte((uint8_t*)pConnectField.front_addr);
	//strcpy(pConnectField.front_addr, *addrAscii);
	szPath->WriteOneByte((uint8_t*)pConnectField.szPath);

	//strcpy(pConnectField.szPath, ((std::string)*pathAscii).c_str());  
	logger_cout(log.append(" ").append(pConnectField.front_addr).append("|").append(pConnectField.szPath).append("|").c_str());
	obj->uvMdUser->Connect(&pConnectField, FunRtnCallback, uuid);
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
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	//Local<String> broker = args[0]->ToString();
	//Local<String> userId = args[1]->ToString();
	//Local<String> pwd = args[2]->ToString();
	//String::AsciiValue brokerAscii(broker);
	//String::AsciiValue userIdAscii(userId);
	//String::AsciiValue pwdAscii(pwd);

	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.UserID, req.Password);
	//strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	//strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
	//strcpy(req.Password, ((std::string)*pwdAscii).c_str());
	//broker->WriteOneByte((uint8_t*)req.BrokerID);
	//userId->WriteOneByte((uint8_t*)req.UserID);
	//pwd->WriteOneByte((uint8_t*)req.Password);
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.UserID).append("|").append(req.Password).c_str());
	obj->uvMdUser->ReqUserLogin(&req, FunRtnCallback, uuid);
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
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	//Local<String> broker = args[0]->ToString();
	//Local<String> userId = args[1]->ToString();
	//String::AsciiValue brokerAscii(broker);
	//String::AsciiValue userIdAscii(userId);

	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	ArgsToObject(args, req.BrokerID, req.UserID);
	//strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	//strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
	logger_cout(log.append(" ").append(req.BrokerID).append("|").append(req.UserID).c_str());
	obj->uvMdUser->ReqUserLogout(&req, FunRtnCallback, uuid);
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
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	} 
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];
	
	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		/*String::AsciiValue idAscii(instrumentId);  		 */
		char* id = new char[instrumentId->Length() + 1];
		//strcpy(id, *idAscii);
		instrumentId->WriteOneByte((uint8_t*)id);
		idArray[i] = id;
		log.append(id).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->SubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
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
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>(isolate,Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];

	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		//String::AsciiValue idAscii(instrumentId);
		char* id = new char[instrumentId->Length() + 1];
		//strcpy(id, *idAscii);
		idArray[i] = id;
		instrumentId->WriteOneByte((uint8_t*)id);
		log.append(id).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->UnSubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
}
FUNCTIONCALLBACK(WrapMdUser::Disposed)
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
obj->uvMdUser->Disposed();

	std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
	while (callback_it != callback_map.end()) {
		callback_it->second.Reset();
		callback_it++;
	}
	event_map.clear();
	callback_map.clear();
	fun_rtncb_map.clear();
	delete obj->uvMdUser;
    obj->uvMdUser = NULL;
	logger_cout("wrap_mduser Disposed------>wrap disposed");
}


////////////////////////////////////////////////////////////////////////////////////////////////

void WrapMdUser::FunCallback(CbRtnField *data) {
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
		FunCallback_Switch(T_ON_RSPUSERLOGOUT, 4, ; pkg_cb_userlogout(data, argv);)
		FunCallback_Switch(T_ON_RSPSUBMARKETDATA, 4, ; pkg_cb_rspsubmarketdata(data, argv);)
		FunCallback_Switch(T_ON_RSPUNSUBMARKETDATA, 4, ; pkg_cb_unrspsubmarketdata(data, argv);)
		FunCallback_Switch(T_ON_RTNDEPTHMARKETDATA, 1, ;pkg_cb_rtndepthmarketdata(data, argv);)
		FunCallback_Switch(T_ON_RSPERROR, 3, ;pkg_cb_rsperror(data, argv);)
	}
#undef FunCallback_Switch
}
void WrapMdUser::FunRtnCallback(int result, void* baton) {
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);
	if (tmp->uuid != -1) {
		std::map<const int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[1] = {GETLOCAL(tmp->nResult) };
		it->second.Get(isolate)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
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
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
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
