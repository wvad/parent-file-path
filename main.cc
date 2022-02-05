#include <node.h>

using namespace v8;

Local<String> ToV8String(Isolate *isolate, const char *str) {
  return String::NewFromUtf8(isolate, str, NewStringType::kNormal).ToLocalChecked();
}

Local<Function> toJSFunction(Isolate *isolate, FunctionCallback func, const char *funcName, int paramsLength, Local<Value> value) {
  Local<Function> function = Function::New(
    isolate->GetCurrentContext(), func, value,
    paramsLength, ConstructorBehavior::kThrow
  ).ToLocalChecked();
  function->SetName(ToV8String(isolate, funcName));
  return function;
}

void addMethodToObject(Local<Object> object, const char *funcName, FunctionCallback function, int paramsLength) {
  Isolate *isolate = object->GetIsolate();
  object->Set(
    isolate->GetCurrentContext(),
    ToV8String(isolate, funcName),
    toJSFunction(isolate, function, funcName, paramsLength, Local<Value>())
  ).Check();
}

void GetStackTrace(Isolate *isolate, Local<Context> &context, Local<Array> &array) {
  Local<StackTrace> stackTrace = StackTrace::CurrentStackTrace(isolate, 20);
  if (stackTrace.IsEmpty()) return;
  int length = stackTrace->GetFrameCount();
  for (int i = 0; i < length; i++) {
    Local<StackFrame> flame = stackTrace->GetFrame(isolate, i);
    if (flame.IsEmpty()) continue;
    Local<String> scriptNameOrSourceUrl = flame->GetScriptNameOrSourceURL();
    if (scriptNameOrSourceUrl.IsEmpty()) continue;
    array->Set(context, i, scriptNameOrSourceUrl);
  }
}

void InferParentFile(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  if (!isolate) return;
  Local<Array> result = Array::New(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  GetStackTrace(isolate, context, result);
  Local<Function> callback = args.Data().As<Function>();
  Local<Value> argv[] = { result };
  MaybeLocal<Value> returnValue = callback->Call(context, args.This(), 1, argv);
  if (returnValue.IsEmpty()) return;
  args.GetReturnValue().Set(returnValue.ToLocalChecked());
}

void FunctionFactory(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  if (args.Length() != 1 || !args[0]->IsFunction()) return;
  args.GetReturnValue().Set(
    toJSFunction(isolate, InferParentFile, "inferParentFile", 0, args[0])
  );
}

void init(Local<Object>, Local<Object> module) {
  Isolate *isolate = module->GetIsolate();
  addMethodToObject(module, "exports", FunctionFactory, 1);
}

NODE_MODULE(ParentFilePath, init)
