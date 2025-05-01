/**
 * load_argon2.js - Lightweight wrapper for argon2.js with manual WASM loading
 */

(function(global) {
  'use strict';

  // Configuration for Argon2
  const DEFAULT_CONFIG = {
    time: 2,            // Iterations
    //mem: 65536,         // Memory (64 MB)
    mem: 131072,        // Memory (128 MB)
    hashLen: 32,        // Output hash length
    parallelism: 2,     // Parallelism factor
    type: 2             // Argon2id (0=Argon2d, 1=Argon2i, 2=Argon2id)
  };

  // Status tracking
  let wasmLoaded = false;
  let wasmLoading = false;
  let wasmLoadPromise = null;
  let readyCallback = null;

  // Utility functions
  function encodeUtf8(str) {
    return new TextEncoder().encode(str);
  }

  function allocateArray(arr) {
    return Module.allocate(arr, 'i8', Module.ALLOC_NORMAL);
  }

  // Expose the ready function
  global.argon2Ready = function(callback) {
    if (wasmLoaded) {
      callback();
    } else {
      readyCallback = callback;
      if (!wasmLoading) {
        loadWasm();
      }
    }
  };

  // Main hash calculation function
  global.argon2_calc = function(origin_msg) {
    if (!wasmLoaded) {
      console.error("Argon2 WASM not loaded. Call argon2Ready first.");
      return null;
    }

    try { 
      //const salt = "argon2saltsalt"; // 16-byte fixed salt
      const salt = typeof global.argon2saltsalt !== 'undefined' ? global.argon2saltsalt : "argon2saltsalt"; // Use global var if exists

      const password = origin_msg;

      const passEncoded = encodeUtf8(password);
      const pwd = allocateArray(passEncoded);
      const pwdlen = passEncoded.length;

      const saltEncoded = encodeUtf8(salt);
      const saltPtr = allocateArray(saltEncoded);
      const saltlen = saltEncoded.length;

      const hashLen = DEFAULT_CONFIG.hashLen;
      const hash = Module.allocate(new Array(hashLen), 'i8', Module.ALLOC_NORMAL);

      const encodedlen = Module._argon2_encodedlen(
        DEFAULT_CONFIG.time,
        DEFAULT_CONFIG.mem,
        DEFAULT_CONFIG.parallelism,
        saltlen,
        hashLen,
        DEFAULT_CONFIG.type
      );
      const encoded = Module.allocate(new Array(encodedlen + 1), 'i8', Module.ALLOC_NORMAL);

      const secret = 0;
      const secretlen = 0;
      const ad = 0;
      const adlen = 0;
      const version = 0x13;

      const res = Module._argon2_hash_ext(
        DEFAULT_CONFIG.time,
        DEFAULT_CONFIG.mem,
        DEFAULT_CONFIG.parallelism,
        pwd,
        pwdlen,
        saltPtr,
        saltlen,
        hash,
        hashLen,
        encoded,
        encodedlen,
        DEFAULT_CONFIG.type,
        secret,
        secretlen,
        ad,
        adlen,
        version
      );

      if (res !== 0) {
        Module._free(pwd);
        Module._free(saltPtr);
        Module._free(hash);
        Module._free(encoded);
        return null;
      }

      // Extract the hash as hex string
      let hashHex = '';
      for (let i = 0; i < hashLen; i++) {
        const byteValue = Module.HEAP8[hash + i] & 0xff;
        hashHex += ('0' + byteValue.toString(16)).slice(-2);
      }

      Module._free(pwd);
      Module._free(saltPtr);
      Module._free(hash);
      Module._free(encoded);

      return hashHex;
    } catch (e) {
      console.error("Error in argon2_calc:", e);
      return null;
    }
  };

  // Initialize and load WASM
  function loadWasm() {
    if (wasmLoaded || wasmLoading) return;
    wasmLoading = true;

    wasmLoadPromise = new Promise((resolve, reject) => {
      if (!global.WebAssembly) {
        console.error("WebAssembly not supported");
        wasmLoading = false;
        reject("WebAssembly not supported");
        return;
      }

      const mem = DEFAULT_CONFIG.mem;
      const method = 'native-wasm';

      const KB = 1024 * 1024;
      const MB = 1024 * KB;
      const GB = 1024 * MB;
      const WASM_PAGE_SIZE = 64 * 1024;

      const totalMemory = (2 * GB - 64 * KB) / 1024 / WASM_PAGE_SIZE;
      const initialMemory = Math.min(
        Math.max(Math.ceil((mem * 1024) / WASM_PAGE_SIZE), 256) + 256,
        totalMemory
      );

      const wasmMemory = new WebAssembly.Memory({
        initial: initialMemory,
        maximum: totalMemory,
      });

      global.Module = {
        print: console.log,
        printErr: console.error,
        setStatus: console.log,
        wasmBinary: null,
        wasmJSMethod: method,
        wasmMemory: wasmMemory,
        buffer: wasmMemory.buffer,
        TOTAL_MEMORY: initialMemory * WASM_PAGE_SIZE,
      };

      // Load WASM file
      const xhr = new XMLHttpRequest();
      xhr.open('GET', 'argon2.wasm', true);
      xhr.responseType = 'arraybuffer';
      xhr.onload = function() {
        if (xhr.status === 200 || xhr.status === 0) {
          global.Module.wasmBinary = xhr.response;

          // Load argon2.js script
          const script = document.createElement('script');
          script.src = 'argon2.js';
          script.onload = function() {
            // Wait for Module to be fully initialized
            if (global.Module._argon2_hash_ext) {
              finishLoading();
            } else {
              // Module might need some time to initialize
              const checkInterval = setInterval(function() {
                if (global.Module._argon2_hash_ext) {
                  clearInterval(checkInterval);
                  finishLoading();
                }
              }, 10);
            }
          };
          script.onerror = function() {
            wasmLoading = false;
            console.error("Failed to load argon2.js");
            reject("Failed to load argon2.js");
          };
          document.body.appendChild(script);
        } else {
          wasmLoading = false;
          console.error("Failed to load argon2.wasm");
          reject("Failed to load argon2.wasm");
        }
      };
      xhr.onerror = function() {
        wasmLoading = false;
        console.error("Failed to load argon2.wasm");
        reject("Failed to load argon2.wasm");
      };
      xhr.send(null);
    });

    return wasmLoadPromise;
  }

  function finishLoading() {
    wasmLoaded = true;
    wasmLoading = false;
    if (readyCallback) {
      readyCallback();
      readyCallback = null;
    }
  }
})(typeof window !== 'undefined' ? window : this);
