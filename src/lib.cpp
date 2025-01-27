#include <stdio.h>
#include <string.h>
#include "cbor.h"

static const uint64_t MASK_LOWER_32 = 0x00000000fffffffful;

CborBuffer::CborBuffer(uint8_t *outputBuffer, int outputBufferSize) {
  buffer = outputBuffer;
  length = outputBufferSize;
  pos = 0;
  CborBuffer::Error error = Error::OK;
}

void CborBuffer::putByte(uint8_t byte) {
  buffer[pos++] = byte;
}

void CborBuffer::putBytes(const uint8_t *byteBuffer, int length) {
  while (--length >= 0) {
    putByte(*byteBuffer++);
  }
}

void CborBuffer::encodeTagAndValue(int tag, int length, uint64_t value) {
  putByte((uint8_t) tag);
  uint8_t buffer[8];
  int i = length;
  while (--i >= 0) {
      buffer[i] = (uint8_t)value;
      value >>= 8;
  }
  putBytes(buffer, length);
}

void CborBuffer::encodeTagAndN(int majorType, uint64_t n) {
  int modifier = (int)n;
  int length = 0;
  if (n > 23) {
    modifier = 27;
    length = 32;
    while (((MASK_LOWER_32 << length) & n) == 0) {
      modifier--;
      length >>= 1;
    }
  }
  encodeTagAndValue(majorType | modifier, length >> 2, n);
}

CborBuffer* CborBuffer::add(CborObject cborObject) {
  cborObject.executor(this, cborObject);
  return this;
}

CborBuffer::Error CborBuffer::error = CborBuffer::Error::OK;

void CborBuffer::setError(Error error) {
  if (CborBuffer::error == CborBuffer::Error::OK) {
    CborBuffer::error = error;
  }
}

CborStructure::CborStructure() {
  cborBuffer = NULL;
}

CborStructure::CborStructure(CborBuffer* cborMasterBuffer) {
  printf("Str=%x\n", this);
  cborBuffer = cborMasterBuffer;
  size = 0;
  startPos = cborBuffer->pos;
  endPos = cborBuffer->pos;
}

void CborStructure::updateTag() {
  printf("UPD%x\n", this);
  cborBuffer->buffer[startPos] = (cborBuffer->buffer[startPos] & 0xe0) | ++size;
}

void CborStructure::putInitialTag() {
  size = 0;
  startPos = cborBuffer->pos;
  cborBuffer->putByte(getTag());
  endPos = cborBuffer->pos;
}

CborArray* CborArray::add(CborBuffer::CborObject value) {
  updateTag();
  int beginKey = cborBuffer->pos;
  cborBuffer->add(value);
  int endOfEntry = cborBuffer->pos;
  printf("add=%x\n", this);
  return this;
}

CborMap* CborMap::set(CborBuffer::CborObject key, CborBuffer::CborObject value) {
  updateTag();
  int beginKey = cborBuffer->pos;
  cborBuffer->add(key);
  int lastOfKey = cborBuffer->pos;
  cborBuffer->add(value);
  int endOfEntry = cborBuffer->pos;
  printf("set=%x\n", this);
  return this;
}

void CborBuffer::CborObject::intExec(CborBuffer* cborBuffer, 
                                    CborBuffer::CborObject& cborObject) {
  int64_t value = cborObject.coreData.intValue;
  int tag = MT_UNSIGNED;
  if (value < 0) {
      tag = MT_NEGATIVE;
      value = ~value;
  }
  cborBuffer->encodeTagAndN(tag, (uint64_t)value);
  printf("intexec\n");
}

void CborBuffer::CborObject::uintExec(CborBuffer* cborBuffer,
                                      CborBuffer::CborObject& cborObject) {
  cborBuffer->encodeTagAndN(MT_UNSIGNED, (uint64_t)cborObject.coreData.intValue);
  printf("uintexec\n");
}

void CborBuffer::CborObject::stringExec(CborBuffer* cborBuffer,
                                        CborBuffer::CborObject& cborObject) {
  cborBuffer->encodeTagAndN(MT_TEXT_STRING, cborObject.optionalLength);
  cborBuffer->putBytes(cborObject.coreData.stringValue, cborObject.optionalLength);
  printf("stringexec\n");
}

void CborBuffer::CborObject::preComputedExec(CborBuffer* cborBuffer, 
                                             CborBuffer::CborObject& cborObject) {
  for (int length = 0; length < cborObject.optionalLength; ) {
    cborBuffer->putByte(cborObject.coreData.stringValue[length++]);
  }
  printf("precompexec\n");
}

void CborBuffer::CborObject::structuredExec(CborBuffer* cborBuffer,
                                            CborBuffer::CborObject& cborObject) {
  if (!cborObject.coreData.cborStructure->cborBuffer) {
    CborBuffer::setError(CborBuffer::Error::WRONG_CONSTRUCTOR);
  }
  cborObject.coreData.cborStructure->cborBuffer = cborBuffer;
  cborObject.coreData.cborStructure->putInitialTag();
  printf("structexec\n");
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

CborBuffer::CborObject CBOR::Array(CborArray& cborArray) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.cborStructure = &cborArray;
  cborObject.executor = CborBuffer::CborObject::structuredExec;
  return cborObject;
}

CborBuffer::CborObject CBOR::Map(CborMap& cborMap) {
  CborBuffer::CborObject cborObject;
  cborObject.coreData.cborStructure = &cborMap;
  cborObject.executor = CborBuffer::CborObject::structuredExec;
  return cborObject;
}