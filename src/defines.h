#pragma once
#include <node.h>
using namespace v8;
#define CONVERT2LOCAL(type,type2) Local<type2> Convert2Local(Isolate* isolate, type value) 

inline CONVERT2LOCAL(int, Number)
{
	return Int32::New(isolate, value);
}
inline CONVERT2LOCAL(double, Number)
{
	return Number::New(isolate, value);
}
inline CONVERT2LOCAL(char, String)
{
	char t[2] = { value,0 };
	return String::NewFromOneByte(isolate, (uint8_t*)t);
}
inline Local<String> Convert2Local(Isolate* isolate, const char value[])
{
	//std::codecvt_utf8<char>
	//wchar_t wstring[sizeof(value)];
	//mbstowcs(wstring, value, sizeof(value));
	//return String::NewFromTwoByte(isolate,(const uint16_t*) wstring);
	return String::NewFromOneByte(isolate, (uint8_t*)value);
	//return String::NewFromUtf8(isolate, value);
}
inline CONVERT2LOCAL(bool, Boolean)
{
	return Boolean::New(isolate, value);
}
#define GETLOCAL(x) Convert2Local(isolate,x)
#define CONVERT2LOCALOBJECT(type,...)  CONVERT2LOCAL(type,Object){\
auto result = Object::New(isolate);\
__VA_ARGS__ \
return result;\
};
template<typename T>
struct Constructor;
template<typename T>
void CreateCObject(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	if (args.IsConstructCall())
	{
		new T(args);
		args.GetReturnValue().Set(args.This());
	}
	else
	{
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = T::constructor.Get(isolate);
		int argc = args.Length();
		Local<Value> *_args = argc>0 ? (Local<Value> *)malloc(sizeof(Local<Value>)*argc) : nullptr;
		for (int i = 0; i < argc; ++i)_args[i] = args[i];
		Local<Object> result = cons->NewInstance(context, argc, _args).ToLocalChecked();
		free(_args);
		args.GetReturnValue().Set(result);
	}
};
typedef Persistent<Function> PFunction;
template<typename T>
struct Constructor
{
public:
	//static PFunction constructor;
	Local<FunctionTemplate> tpl;
	Local<String> className;
	Isolate* isolate;
	const Handle<Object>& target;
	Constructor(const Handle<Object>& target, const char* name) :target(target){
		isolate = target->GetIsolate();
		className = String::NewFromUtf8(isolate, name);
		tpl = FunctionTemplate::New(isolate, CreateCObject<T>);
		tpl->SetClassName(className);
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	}
	void SetProtoTypeMethod(const char* name, FunctionCallback callback) {
		NODE_SET_PROTOTYPE_METHOD(tpl, name, callback);
	}
	~Constructor() {
		T::constructor.Reset(isolate, tpl->GetFunction());
		target->Set(className, tpl->GetFunction());
	}
};
//#define DECL_CONSTR(TYPE)                 \
template<> Persistent<Function> Constructor<TYPE>::constructor;  \
template class Constructor<TYPE>;

#define DECL_CONSTR(name) template<> PFunction Constructor<name>::constructor;

#define NEW_CONSTR(classname) Constructor<classname> _##classname(target, #classname)

#define FUNCTIONCALLBACK(name) void name(const FunctionCallbackInfo<Value>& args) {SCOPE(args)
inline void ArgsToObject(const FunctionCallbackInfo<Value>& args, int index){

}
template<typename T, typename ...Args>
inline void ArgsToObject(const FunctionCallbackInfo<Value>& args, int index, T& value, Args&... _args) {
	args[index]->ToString()->WriteOneByte((uint8_t*)value);
	ArgsToObject(args, index + 1, _args...);
}
template<typename ...Args>
inline void ArgsToObject(const FunctionCallbackInfo<Value>& args, Args&... _args) {
	ArgsToObject(args, 0, _args...);
}

template<class T>
inline void SetObjectProperty(Local<Object> object, Isolate* isolate, const char* key, T value)
{
	object->Set(String::NewFromUtf8(isolate, key), Convert2Local(isolate, value));
}
template<class M>
inline void AddToMap(M &m, int k,const Local<Function> &f) {
	m.emplace(std::make_pair(k, Persistent<Function, CopyablePersistentTraits<Function>>(f->GetIsolate(), f)));
}


#define SCOPE(x) Isolate* isolate = x.GetIsolate();\
HandleScope scope(isolate);
#define CSCOPE Isolate* isolate = Isolate::GetCurrent();\
HandleScope scope(isolate);