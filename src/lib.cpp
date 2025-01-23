#include <stdio.h>
#include <string.h>
#include "cbor.h"

CborBuffer::CborBuffer(uint8_t *outputBuffer, int outputBufferSize) {
  buffer = outputBuffer;
  length = outputBufferSize;
  pos = 0;
}

void CborBuffer::put(uint8_t byte) {
  buffer[pos++] = byte;
}

CborBuffer& CborBuffer::add(CborObject cborObject) {
  cborObject.executor(*this, cborObject);
  return *this;
}

CborMap::CborMap(CborBuffer& cborMasterBuffer) {
  cborBuffer = &cborMasterBuffer;
}

CborMap CborMap::set(CborBuffer::CborObject key, CborBuffer::CborObject value) {
  cborBuffer->add(key).add(value);
  return *this;
}

void CborBuffer::CborObject::intExec(CborBuffer& cborBuffer, CborBuffer::CborObject& cborObject) {
  cborBuffer.put((unsigned char)cborObject.coreData.intValue);
  printf("intexec\n");
}

void CborBuffer::CborObject::uintExec(CborBuffer& cborBuffer, CborBuffer::CborObject& cborObject) {
  printf("intexec\n");
}

void CborBuffer::CborObject::stringExec(CborBuffer& cborBuffer, CborBuffer::CborObject& cborObject) {
  for (int length = 0; length < cborObject.optionalLength; ) {
    cborBuffer.put(cborObject.coreData.stringValue[length++]);
  }
  printf("stringexec\n");
}

void CborBuffer::CborObject::preComputedExec(CborBuffer& cborBuffer, CborBuffer::CborObject& cborObject) {
  for (int length = 0; length < cborObject.optionalLength; ) {
    cborBuffer.put(cborObject.coreData.stringValue[length++]);
  }
  printf("precompexec\n");
}

CborBuffer::CborObject CBOR::Int(int64_t value) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.intValue = value;
  cborObject.executor = CborBuffer::CborObject::intExec;
  return cborObject;
}

CborBuffer::CborObject CBOR::Uint(uint64_t value) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.intValue = (int64_t) value;
  cborObject.executor = CborBuffer::CborObject::uintExec;
  return cborObject;
}

CborBuffer::CborObject CBOR::String(const char* string) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.stringValue = (const uint8_t*) string;
  cborObject.optionalLength = strlen(string);
  cborObject.executor = CborBuffer::CborObject::stringExec;
  return cborObject;
}

CborBuffer::CborObject CBOR::PreComputed(const uint8_t* cborItem, int length) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.stringValue = cborItem;
  cborObject.optionalLength = length;
  cborObject.executor = CborBuffer::CborObject::preComputedExec;
  return cborObject;
}