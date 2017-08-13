#pragma once
#define SCOPE(x) Isolate* isolate = x.GetIsolate();\
HandleScope scope(isolate);
#define CONVERT2LOCAL(type,type2) Local<type2> inline Convert2Local(Isolate* isolate, type value) 
#define CONVERT2LOCALOBJECT(type,...)  CONVERT2LOCAL(type,Object){\
auto result = Object::New(isolate);\
__VA_ARGS__ \
return result;\
};
CONVERT2LOCAL(int, Number)
{
	return Int32::New(isolate, value);
}
CONVERT2LOCAL(double, Number)
{
	return Number::New(isolate, value);
}
CONVERT2LOCAL(char, Number)
{
	return Number::New(isolate, value);
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
CONVERT2LOCAL(bool, Boolean)
{
	return Boolean::New(isolate, value);
}
#define GETLOCAL(x) Convert2Local(isolate,x)
template<typename T>
void CreateCObject(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	if (args.IsConstructCall())
	{
		T * cobject = new T(args);

		args.GetReturnValue().Set(args.This());
	}
	else
	{
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, Constructor<T>::constructor);
		int argc = args.Length();
		Local<Value> *_args = argc>0 ? (Local<Value> *)malloc(sizeof(Local<Value>)*argc) : nullptr;
		for (int i = 0; i < argc; ++i)_args[i] = args[i];
		Local<Object> result = cons->NewInstance(context, argc, _args).ToLocalChecked();
		free(_args);
		args.GetReturnValue().Set(result);
	}
};
template<typename T>
struct Constructor
{
public:
	static Persistent<Function> constructor;
	Local<FunctionTemplate> tpl;
	Local<String> className;
	Isolate* isolate;
	const Handle<Object>& target;
	Constructor(const Handle<Object>& target, const char* name) :target(target)
	{
		isolate = target->GetIsolate();
		className = String::NewFromUtf8(isolate, name);
		tpl = FunctionTemplate::New(isolate, CreateCObject<T>);
		tpl->SetClassName(className);
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	}
	void SetProtoTypeMethod(const char* name, FunctionCallback callback)
	{
		NODE_SET_PROTOTYPE_METHOD(tpl, name, callback);
	}
	~Constructor()
	{
		constructor.Reset(isolate, tpl->GetFunction());
		target->Set(className, tpl->GetFunction());
	}
};
#define DECL_CONSTR(name) typedef Constructor<name> name##Constructor;\
Persistent<Function> Constructor<name>::constructor;
#define NEW_CONSTR(classname) classname##Constructor _##classname(target, #classname)

#define FUNCTIONCALLBACK(name) void name(const FunctionCallbackInfo<Value>& args) {SCOPE(args)


inline void ArgsToObject(const FunctionCallbackInfo<Value>& args,int index, char(&value)[]) {
	args[index]->ToString()->WriteOneByte((uint8_t*)value);
}
template<typename T, typename ...Args>
void ArgsToObject(const FunctionCallbackInfo<Value>& args,int index, T& value, Args&... _args) {
	ArgsToObject(args, index + 1, value, _args...);
}
template<typename ...Args>
void ArgsToObject(const FunctionCallbackInfo<Value>& args, Args&... _args) {
	ArgsToObject(args, 0, _args...);
}

template<class T>
void SetObjectProperty(Local<Object> object, Isolate* isolate, const char* key, T value)
{
	object->Set(String::NewFromUtf8(isolate, key), Convert2Local(isolate, value));
}