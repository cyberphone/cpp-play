#include <stdio.h>
#include <stdint.h>

#pragma once

static const int MT_UNSIGNED      = 0x00;
static const int MT_NEGATIVE      = 0x20;
static const int MT_BYTE_STRING   = 0x40;
static const int MT_TEXT_STRING   = 0x60;
static const int MT_ARRAY         = 0x80;
static const int MT_MAP           = 0xa0;
static const int MT_TAG_EXTENSION = 0xc0;
static const int MT_BIG_UNSIGNED  = 0xc2;
static const int MT_BIG_SIGNED    = 0xc3;
static const int MT_FALSE         = 0xf4;
static const int MT_TRUE          = 0xf5;
static const int MT_NULL          = 0xf6;

class CborMap;
class CborArray;
class CBOR;

class CborBuffer {

  class CborObject {

    CborObject() {
      executor = NULL;
    }

    union {
      int64_t intValue;
      double floatValue;
      const uint8_t *stringValue;
    } coreData;
    int optionalLength;
    void (* executor)(CborBuffer*, CborObject&);
    static void intExec(CborBuffer*, CborObject&);
    static void uintExec(CborBuffer*, CborObject&);
    static void stringExec(CborBuffer*, CborObject&);
    static void preComputedExec(CborBuffer*, CborObject&);
    static void structuredExec(CborBuffer*, CborObject&);

    friend class CborBuffer;
    friend class CBOR;
    friend class CborMap;
    friend class CborArray;
  };

  uint8_t *buffer;
  int length;
  int pos;

  void putByte(uint8_t byte);

  void putBytes(const uint8_t *byteBuffer, int length);

  void encodeTagAndValue(int majorType, int length, uint64_t value);

  void encodeTagAndN(int tag, uint64_t n);

  public:
    CborBuffer(uint8_t *outputBuffer, int outputBufferSize);

    ~CborBuffer() {
      printf("buffdtor\n");
    }

    CborBuffer* add(CborObject cborObject);

    uint8_t getByte(int position) {
      return buffer[position];
    }

    int getPos() {
      return pos;
    }

    friend class CBOR;
    friend class CborMap;
    friend class CborArray;
    friend class CborStructure;
};

class CBOR {
  public:

    static CborBuffer::CborObject Int(int64_t value);

    static CborBuffer::CborObject Uint(uint64_t value);

    static CborBuffer::CborObject String(const char* string);

    static CborBuffer::CborObject Array(CborArray& cborArray, CborBuffer& cborBuffer);

    static CborBuffer::CborObject Map(CborMap& cborMap, CborBuffer& cborBuffer);

    static CborBuffer::CborObject PreComputed(const uint8_t* cborItem, int length);
};

class CborStructure {
  int size;
  int startPos;
  int endPos;
  CborBuffer *cborBuffer;

  void updateTag();

  CborStructure();
  CborStructure(CborBuffer*, int);

  friend class CborBuffer;
  friend class CborMap;
  friend class CborArray;
};

class CborMap : CborStructure {

  public:
    CborMap() : CborStructure() {

    }

    CborMap(CborBuffer& cborBuffer) : CborStructure(&cborBuffer, MT_MAP) {
      printf("map%x\n",this);
    }

    CborMap* set(CborBuffer::CborObject key, CborBuffer::CborObject value);

  friend class CborBuffer;
};

class CborArray : CborStructure {

  public:
    CborArray() : CborStructure() {

    }

    CborArray(CborBuffer& cborBuffer) : CborStructure(&cborBuffer, MT_ARRAY) {
      printf("array%x\n",this);
    }

    CborArray* add(CborBuffer::CborObject value);

  friend class CborBuffer;
};

