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
class CBOR;

class CborBuffer {

  class CborObject {
    union {
      int64_t intValue;
      double floatValue;
      const uint8_t *stringValue;
      CborMap *cborMap;
    } coreData;
    int optionalLength;
    void (* executor)(CborBuffer&, CborObject&);
    static void intExec(CborBuffer&, CborObject&);
    static void uintExec(CborBuffer&, CborObject&);
    static void stringExec(CborBuffer&, CborObject&);
    static void preComputedExec(CborBuffer&, CborObject&);

    friend class CborBuffer;
    friend class CBOR;
    friend class CborMap;
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

    CborBuffer& add(CborObject cborObject);

    uint8_t getByte(int postion) {
      return buffer[postion];
    }

    int getPos() {
      return pos;
    }

    friend class CBOR;
    friend class CborMap;
    friend class CborStructure;
};

class CBOR {
  public:

    static CborBuffer::CborObject Int(int64_t value);

    static CborBuffer::CborObject Uint(uint64_t value);

    static CborBuffer::CborObject String(const char* string);

    static CborBuffer::CborObject Map(CborMap map);

    static CborBuffer::CborObject PreComputed(const uint8_t* cborItem, int length);
};

class CborStructure {
  int size;
  int startPos;
  CborBuffer *cborBuffer;

  CborStructure();
  CborStructure(CborBuffer&, int);

  friend class CborBuffer;
  friend class CborMap;
};

class CborMap : CborStructure {

  public:
    CborMap() : CborStructure() {

    };

    CborMap(CborBuffer& cborBuffer) : CborStructure(cborBuffer, MT_MAP) {

    }

    CborMap set(CborBuffer::CborObject key, CborBuffer::CborObject value);

  friend class CborBuffer;
};

