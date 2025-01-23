#include <stdio.h>
#include <string.h>
#include "cbor.h"

CborBuffer::CborBuffer(unsigned char *outputBuffer, int outputBufferSize) {
  buffer = outputBuffer;
  length = outputBufferSize;
  pos = 0;
}

void CborBuffer::put(unsigned char byte) {
  buffer[pos++] = byte;
}

CborBuffer& CborBuffer::add(CborObject cborObject) {
  cborObject.executor(*this, cborObject);
  return *this;
}

void CborBuffer::add(CborMap& map) {
  map.startPos = pos;
}

CborMap::CborMap() {
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

CborBuffer::CborObject CBOR::Int(int value) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.intValue = value;
  cborObject.executor = CborBuffer::CborObject::intExec;
  return cborObject;
}

CborBuffer::CborObject CBOR::Uint(unsigned int value) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.intValue = (int) value;
  cborObject.executor = CborBuffer::CborObject::uintExec;
  return cborObject;
}

CborBuffer::CborObject CBOR::String(const char* string) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.stringValue = (const unsigned char*) string;
  cborObject.optionalLength = strlen(string);
  cborObject.executor = CborBuffer::CborObject::stringExec;
  return cborObject;
}