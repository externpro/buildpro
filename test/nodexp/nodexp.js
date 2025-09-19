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

console.log('Testing N-API addon...');

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
  console.assert(result === 'N-API test successful', 'Test function failed');
  console.log('✓ Test function passed');
} catch (error) {
  console.error('✗ Test function failed:', error.message);
  process.exit(1);
}

console.log('All tests passed!');
