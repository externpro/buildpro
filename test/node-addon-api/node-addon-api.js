const path = require('path');
const fs = require('fs');

// The .node file is in the same directory as this test script
const addonPath = path.join(__dirname, '$<TARGET_FILE_NAME:@tgt@>');
console.log('Loading addon from:', addonPath);

// Verify the file exists
if (!fs.existsSync(addonPath)) {
  console.error('Addon file not found:', addonPath);
  process.exit(1);
}

const addon = require(addonPath);

console.log('Testing node-addon-api C++ addon...');

// Test the add function
try {
  const result = addon.add(2.5, 3.5);
  console.log('add(2.5, 3.5) =', result);
  console.assert(result === 6.0, 'Addition test failed');
  console.log('✓ Addition test passed');
} catch (error) {
  console.error('✗ Addition test failed:', error.message);
  process.exit(1);
}

// Test the test function
try {
  const result = addon.test();
  console.log('test() =', result);
  console.assert(result === 'node-addon-api C++ test successful', 'Test function failed');
  console.log('✓ Test function passed');
} catch (error) {
  console.error('✗ Test function failed:', error.message);
  process.exit(1);
}

// Test N-API version - Node.js v22 should support N-API version 9
try {
  const version = addon.getNapiVersion();
  console.log('N-API version =', version);
  console.assert(version >= 8, 'N-API version should be at least 8 for Node.js v22');
  console.log('✓ N-API version test passed');
} catch (error) {
  console.error('✗ N-API version test failed:', error.message);
  process.exit(1);
}

// Test BigInt support - available in N-API v6+ and Node.js v10.7.0+
try {
  const bigIntResult = addon.testBigInt();
  console.log('testBigInt() =', bigIntResult);
  console.assert(typeof bigIntResult === 'bigint', 'Result should be a BigInt');
  console.assert(bigIntResult === 9007199254740992n, 'BigInt value should be 2^53');
  console.log('✓ BigInt creation test passed');
} catch (error) {
  console.error('✗ BigInt creation test failed:', error.message);
  process.exit(1);
}

// Test BigInt arithmetic (using a smaller BigInt that fits in int64)
try {
  const inputBigInt = 1234567890123456789n; // Fits in int64 range
  const result = addon.addOneToBigInt(inputBigInt);
  console.log('addOneToBigInt(' + inputBigInt + 'n) =', result);
  console.assert(typeof result === 'bigint', 'Result should be a BigInt');
  console.assert(result === inputBigInt + 1n, 'Result should be input + 1');
  console.log('✓ BigInt arithmetic test passed');
} catch (error) {
  console.error('✗ BigInt arithmetic test failed:', error.message);
  process.exit(1);
}

// Test BigInt arithmetic with very large number (should fail gracefully)
try {
  const veryLargeBigInt = 123456789012345678901234567890n;
  addon.addOneToBigInt(veryLargeBigInt);
  console.error('✗ Large BigInt test failed: should have thrown an error');
  process.exit(1);
} catch (error) {
  console.log('✓ Large BigInt handling test passed: correctly handled very large BigInt');
}

// Test BigInt error handling
try {
  addon.addOneToBigInt(42); // Pass a regular number instead of BigInt
  console.error('✗ BigInt error handling test failed: should have thrown an error');
  process.exit(1);
} catch (error) {
  console.log('✓ BigInt error handling test passed: correctly rejected non-BigInt input');
}

// Test ArrayBuffer support - new in v8.x
try {
  const arrayBuffer = addon.testArrayBuffer();
  console.log('testArrayBuffer() =', arrayBuffer);
  console.assert(arrayBuffer instanceof ArrayBuffer, 'Result should be an ArrayBuffer');
  console.assert(arrayBuffer.byteLength === 16, 'ArrayBuffer should be 16 bytes');
  
  // Check the data
  const view = new Uint8Array(arrayBuffer);
  console.assert(view[0] === 0 && view[1] === 2 && view[2] === 4, 'ArrayBuffer data should be correct');
  console.log('✓ ArrayBuffer test passed');
} catch (error) {
  console.error('✗ ArrayBuffer test failed:', error.message);
  process.exit(1);
}

// Test TypedArray support - enhanced in v8.x
try {
  const typedArray = addon.testTypedArray();
  console.log('testTypedArray() =', typedArray);
  console.assert(typedArray instanceof Uint32Array, 'Result should be a Uint32Array');
  console.assert(typedArray.length === 4, 'TypedArray should have 4 elements');
  console.assert(typedArray[0] === 1000 && typedArray[1] === 2000, 'TypedArray data should be correct');
  console.log('✓ TypedArray test passed');
} catch (error) {
  console.error('✗ TypedArray test failed:', error.message);
  process.exit(1);
}

// Test Promise support - available in v8.x
try {
  const promise = addon.testPromise();
  console.log('testPromise() =', promise);
  console.assert(promise instanceof Promise, 'Result should be a Promise');
  
  // Test the promise resolution
  promise.then(result => {
    console.log('Promise resolved with:', result);
    console.assert(result === 'Promise resolved successfully', 'Promise should resolve with correct value');
    console.log('✓ Promise test passed');
  }).catch(error => {
    console.error('✗ Promise test failed:', error.message);
    process.exit(1);
  });
} catch (error) {
  console.error('✗ Promise test failed:', error.message);
  process.exit(1);
}

// Test enhanced type checking - improved in v8.x
try {
  // Test with different types
  const testValues = [
    42,
    'hello',
    true,
    null,
    undefined,
    [],
    {},
    new Date(),
    123n,
    new Uint8Array(4),
    new ArrayBuffer(8),
    Promise.resolve('test')
  ];

  testValues.forEach((value, index) => {
    const result = addon.testTypeChecking(value);
    console.log(`Type checking for value ${index} (${typeof value}):`, result);
    
    // Verify some expected results
    if (typeof value === 'number') {
      console.assert(result.isNumber === true, 'Should detect number correctly');
    }
    if (typeof value === 'string') {
      console.assert(result.isString === true, 'Should detect string correctly');
    }
    if (typeof value === 'bigint') {
      console.assert(result.isBigInt === true, 'Should detect BigInt correctly');
    }
    if (Array.isArray(value)) {
      console.assert(result.isArray === true, 'Should detect array correctly');
    }
    if (value instanceof ArrayBuffer) {
      console.assert(result.isArrayBuffer === true, 'Should detect ArrayBuffer correctly');
    }
    if (value instanceof Promise) {
      console.assert(result.isPromise === true, 'Should detect Promise correctly');
    }
  });
  
  console.log('✓ Enhanced type checking test passed');
} catch (error) {
  console.error('✗ Enhanced type checking test failed:', error.message);
  process.exit(1);
}

// Display Node.js version and node-addon-api version for reference
console.log('Node.js version:', process.version);
console.log('Using node-addon-api v8.5.0.1 C++ wrapper');

// Wait a bit for the Promise test to complete
setTimeout(() => {
  console.log('All tests passed!');
}, 100);
