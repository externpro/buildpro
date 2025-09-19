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

// Initialize the addon
napi_value Init(napi_env env, napi_value exports)
{
  napi_status status;
  napi_value add_fn, test_fn;

  // Create the Add function
  status = napi_create_function(env, nullptr, 0, Add, nullptr, &add_fn);
  if (status != napi_ok)
    return nullptr;

  // Create the TestNapi function
  status = napi_create_function(env, nullptr, 0, TestNapi, nullptr, &test_fn);
  if (status != napi_ok)
    return nullptr;

  // Set the functions on the exports object
  status = napi_set_named_property(env, exports, "add", add_fn);
  if (status != napi_ok)
    return nullptr;

  status = napi_set_named_property(env, exports, "test", test_fn);
  if (status != napi_ok)
    return nullptr;

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
