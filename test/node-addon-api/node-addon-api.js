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

// Display Node.js version for reference
console.log('Node.js version:', process.version);
console.log('Using node-addon-api C++ wrapper');

console.log('All tests passed!');
