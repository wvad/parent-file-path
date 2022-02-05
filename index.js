const { isAbsolute } = require('node:path');
const { fileURLToPath } = require('node:url');
const { isArray } = Array;

const StringPrototypeStartsWith = Function.call.bind(String.prototype.startsWith);

exports.inferParentFile = require('bindings')('parent-file-path-addon')(function inferParentFile(stackTrace) {
  if (!isArray(stackTrace)) throw new Error('Failed to get stack trace');
  let currentFile;
  for (let frame of stackTrace) {
    if (typeof frame != 'string') continue;
    if (
      StringPrototypeStartsWith(frame, 'node:') ||
      StringPrototypeStartsWith(frame, 'data:')
    ) continue;
    if (StringPrototypeStartsWith(frame, 'file:')) {
      frame = fileURLToPath(frame);
    }
    if (!isAbsolute(frame)) continue;
    if (currentFile && currentFile !== frame) return frame;
    currentFile = frame;
  }
  throw new Error('Failed to infer parent file path');
});
