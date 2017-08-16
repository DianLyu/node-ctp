#include "defines.h"
#include <node.h>
using namespace v8;
CONVERT2LOCAL(int, Number)
{
	return Int32::New(isolate, value);
}
CONVERT2LOCAL(double, Number)
{
	return Number::New(isolate, value);
}
CONVERT2LOCAL(char, String)
{
	char t[2] = { value,0 };
	return String::NewFromOneByte(isolate, (uint8_t*)t);
}
Local<String> Convert2Local(Isolate* isolate, const char value[])
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
void _ArgsToObject(const FunctionCallbackInfo<Value>& args, int index, char value[]) {
	args[index]->ToString()->WriteOneByte((uint8_t*)value);
}
void ArgsToObject(const FunctionCallbackInfo<Value>& args, int index) {
}


