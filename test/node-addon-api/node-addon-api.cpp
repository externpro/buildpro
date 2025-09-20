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

// Test ArrayBuffer and TypedArray support - new in v8.x
Napi::Value TestArrayBuffer(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  // Create an ArrayBuffer with 16 bytes
  size_t length = 16;
  Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, length);

  // Fill it with some test data
  uint8_t* data = static_cast<uint8_t*>(arrayBuffer.Data());
  for (size_t i = 0; i < length; i++)
  {
    data[i] = static_cast<uint8_t>(i * 2);
  }

  return arrayBuffer;
}

// Test TypedArray operations - enhanced in v8.x
Napi::Value TestTypedArray(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  // Create a Uint32Array with 4 elements
  size_t length = 4;
  Napi::Uint32Array typedArray = Napi::Uint32Array::New(env, length);

  // Fill with test data
  uint32_t* data = typedArray.Data();
  for (size_t i = 0; i < length; i++)
  {
    data[i] = static_cast<uint32_t>((i + 1) * 1000);
  }

  return typedArray;
}

// Test Promise support - available in v8.x
Napi::Value TestPromise(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  // Create a promise that resolves immediately
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  // Resolve with a test value
  deferred.Resolve(Napi::String::New(env, "Promise resolved successfully"));

  return deferred.Promise();
}

// Test improved type checking - enhanced in v8.x
Napi::Value TestTypeChecking(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Expected 1 argument")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Value value = info[0];
  Napi::Object result = Napi::Object::New(env);

  // Test all the new type checking methods
  result.Set("isArray", value.IsArray());
  result.Set("isArrayBuffer", value.IsArrayBuffer());
  result.Set("isBigInt", value.IsBigInt());
  result.Set("isBoolean", value.IsBoolean());
  result.Set("isBuffer", value.IsBuffer());
  result.Set("isDataView", value.IsDataView());
  result.Set("isDate", value.IsDate());
  result.Set("isExternal", value.IsExternal());
  result.Set("isFunction", value.IsFunction());
  result.Set("isNull", value.IsNull());
  result.Set("isNumber", value.IsNumber());
  result.Set("isObject", value.IsObject());
  result.Set("isPromise", value.IsPromise());
  result.Set("isString", value.IsString());
  result.Set("isSymbol", value.IsSymbol());
  result.Set("isTypedArray", value.IsTypedArray());
  result.Set("isUndefined", value.IsUndefined());

  return result;
}

// Initialize the addon using node-addon-api C++ interface
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set("add", Napi::Function::New(env, Add));
  exports.Set("test", Napi::Function::New(env, TestNapi));
  exports.Set("getNapiVersion", Napi::Function::New(env, GetNapiVersion));
  exports.Set("testBigInt", Napi::Function::New(env, TestBigInt));
  exports.Set("addOneToBigInt", Napi::Function::New(env, AddOneToBigInt));
  exports.Set("testArrayBuffer", Napi::Function::New(env, TestArrayBuffer));
  exports.Set("testTypedArray", Napi::Function::New(env, TestTypedArray));
  exports.Set("testPromise", Napi::Function::New(env, TestPromise));
  exports.Set("testTypeChecking", Napi::Function::New(env, TestTypeChecking));
  return exports;
}

NODE_API_MODULE(addon, Init)
