#include <node.h>
#include <v8.h>
using namespace v8;
#include "defines.h"

#include "wrap_trader.h"
#include "wrap_mduser.h"

bool islog;//log?

void Settings(const FunctionCallbackInfo<Value>& args) {
	SCOPE(args)
	if (!args[0]->IsUndefined() && args[0]->IsObject()) {
		Local<Object> setting = args[0]->ToObject();
		Local<Value> log = setting->Get(String::NewFromUtf8(isolate,"log"));
		if (!log->IsUndefined()) {
			islog = log->BooleanValue();
		}
	}
}

void Init(Handle<Object> exports, v8::Local<v8::Value> module, void* priv) {
	WrapTrader::Init(exports);
	WrapMdUser::Init(exports);
	NODE_SET_METHOD(exports, "settings", Settings);
}

NODE_MODULE(shifctp, Init)
