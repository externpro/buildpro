#include <node/node_api.h>

// Simple function to test - adds two numbers
napi_value Add(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[2];
  napi_status status;

  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  if (status != napi_ok)
    return nullptr;

  if (argc < 2)
  {
    napi_throw_error(env, "EINVAL", "Expected 2 arguments");
    return nullptr;
  }

  double arg0, arg1;
  status = napi_get_value_double(env, args[0], &arg0);
  if (status != napi_ok)
    return nullptr;

  status = napi_get_value_double(env, args[1], &arg1);
  if (status != napi_ok)
    return nullptr;

  napi_value sum;
  status = napi_create_double(env, arg0 + arg1, &sum);
  if (status != napi_ok)
    return nullptr;

  return sum;
}

// Test function that exercises various N-API functions
napi_value TestNapi(napi_env env, napi_callback_info info)
{
  napi_status status;
  napi_value result;

  // Test creating a string
  status = napi_create_string_utf8(
    env, "N-API test successful", NAPI_AUTO_LENGTH, &result);
  if (status != napi_ok)
  {
    napi_throw_error(env, nullptr, "Failed to create string");
    return nullptr;
  }

  return result;
}

// Test N-API version - Node.js v22 supports N-API version 9
napi_value GetNapiVersion(napi_env env, napi_callback_info info)
{
  napi_status status;
  uint32_t napi_version;
  napi_value result;

  status = napi_get_version(env, &napi_version);
  if (status != napi_ok)
  {
    napi_throw_error(env, nullptr, "Failed to get N-API version");
    return nullptr;
  }

  status = napi_create_uint32(env, napi_version, &result);
  if (status != napi_ok)
  {
    napi_throw_error(env, nullptr, "Failed to create version number");
    return nullptr;
  }

  return result;
}

// Test BigInt support - available in N-API v6+ (Node.js v10.7.0+)
napi_value TestBigInt(napi_env env, napi_callback_info info)
{
  napi_status status;
  napi_value result;

  // Create a BigInt from a 64-bit integer
  int64_t big_number = 9007199254740992LL; // 2^53, larger than safe integer
  status = napi_create_bigint_int64(env, big_number, &result);
  if (status != napi_ok)
  {
    napi_throw_error(env, nullptr, "Failed to create BigInt");
    return nullptr;
  }

  return result;
}

// Test BigInt arithmetic - adds 1 to a BigInt input
napi_value AddOneToBigInt(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  napi_status status;

  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  if (status != napi_ok)
    return nullptr;

  if (argc < 1)
  {
    napi_throw_error(env, "EINVAL", "Expected 1 BigInt argument");
    return nullptr;
  }

  // Try to get the BigInt value - this will fail if it's not a BigInt
  int64_t value;
  bool lossless;
  status = napi_get_value_bigint_int64(env, args[0], &value, &lossless);
  if (status != napi_ok)
  {
    napi_throw_error(env, "EINVAL", "Argument must be a BigInt");
    return nullptr;
  }

  // Add 1 and create a new BigInt
  napi_value result;
  status = napi_create_bigint_int64(env, value + 1, &result);
  if (status != napi_ok)
  {
    napi_throw_error(env, nullptr, "Failed to create result BigInt");
    return nullptr;
  }

  return result;
}

// Initialize the addon
napi_value Init(napi_env env, napi_value exports)
{
  napi_status status;
  napi_value add_fn, test_fn, version_fn, bigint_fn, bigint_add_fn;

  // Create the Add function
  status = napi_create_function(env, nullptr, 0, Add, nullptr, &add_fn);
  if (status != napi_ok)
    return nullptr;

  // Create the TestNapi function
  status = napi_create_function(env, nullptr, 0, TestNapi, nullptr, &test_fn);
  if (status != napi_ok)
    return nullptr;

  // Create the GetNapiVersion function
  status =
    napi_create_function(env, nullptr, 0, GetNapiVersion, nullptr, &version_fn);
  if (status != napi_ok)
    return nullptr;

  // Create the TestBigInt function
  status =
    napi_create_function(env, nullptr, 0, TestBigInt, nullptr, &bigint_fn);
  if (status != napi_ok)
    return nullptr;

  // Create the AddOneToBigInt function
  status = napi_create_function(
    env, nullptr, 0, AddOneToBigInt, nullptr, &bigint_add_fn);
  if (status != napi_ok)
    return nullptr;

  // Set the functions on the exports object
  status = napi_set_named_property(env, exports, "add", add_fn);
  if (status != napi_ok)
    return nullptr;

  status = napi_set_named_property(env, exports, "test", test_fn);
  if (status != napi_ok)
    return nullptr;

  status = napi_set_named_property(env, exports, "getNapiVersion", version_fn);
  if (status != napi_ok)
    return nullptr;

  status = napi_set_named_property(env, exports, "testBigInt", bigint_fn);
  if (status != napi_ok)
    return nullptr;

  status =
    napi_set_named_property(env, exports, "addOneToBigInt", bigint_add_fn);
  if (status != napi_ok)
    return nullptr;

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
