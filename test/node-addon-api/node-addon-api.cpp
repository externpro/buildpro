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

// Test N-API version using node-addon-api C++ interface
Napi::Value GetNapiVersion(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  // Get N-API version using the underlying C API
  uint32_t version;
  napi_status status = napi_get_version(env, &version);
  if (status != napi_ok)
  {
    Napi::Error::New(env, "Failed to get N-API version")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::Number::New(env, version);
}

// Test BigInt creation using node-addon-api C++ interface
Napi::Value TestBigInt(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  // Create a BigInt from a 64-bit integer using C++ API
  int64_t big_number = 9007199254740992LL; // 2^53, larger than safe integer
  return Napi::BigInt::New(env, big_number);
}

// Test BigInt arithmetic using node-addon-api C++ interface
Napi::Value AddOneToBigInt(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Expected 1 BigInt argument")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsBigInt())
  {
    Napi::TypeError::New(env, "Argument must be a BigInt")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  // Try to get the BigInt value as int64
  bool lossless;
  int64_t value = info[0].As<Napi::BigInt>().Int64Value(&lossless);

  if (!lossless)
  {
    // BigInt is too large for int64 - this is expected for very large numbers
    Napi::Error::New(
      env,
      "BigInt arithmetic for very large numbers not implemented in this demo")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  // Add 1 and create a new BigInt
  return Napi::BigInt::New(env, value + 1);
}

// Initialize the addon using node-addon-api C++ interface
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set("add", Napi::Function::New(env, Add));
  exports.Set("test", Napi::Function::New(env, TestNapi));
  exports.Set("getNapiVersion", Napi::Function::New(env, GetNapiVersion));
  exports.Set("testBigInt", Napi::Function::New(env, TestBigInt));
  exports.Set("addOneToBigInt", Napi::Function::New(env, AddOneToBigInt));
  return exports;
}

NODE_API_MODULE(addon, Init)
