#include <node-addon-api/napi.h>

// Simple function to test - adds two numbers using node-addon-api C++ interface
Napi::Value Add(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    Napi::TypeError::New(env, "Expected 2 arguments")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber() || !info[1].IsNumber())
  {
    Napi::TypeError::New(env, "Arguments must be numbers")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  double arg0 = info[0].As<Napi::Number>().DoubleValue();
  double arg1 = info[1].As<Napi::Number>().DoubleValue();

  return Napi::Number::New(env, arg0 + arg1);
}

// Test function that exercises various node-addon-api C++ functions
Napi::Value TestNapi(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  // Test creating a string using the C++ API
  return Napi::String::New(env, "node-addon-api C++ test successful");
}

// Initialize the addon using node-addon-api C++ interface
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "add"), Napi::Function::New(env, Add));
  exports.Set(
    Napi::String::New(env, "test"), Napi::Function::New(env, TestNapi));
  return exports;
}

NODE_API_MODULE(addon, Init)
