#include <stdio.h>
#include <stdint.h>

#pragma once

class CborMap;
class CBOR;

class CborBuffer {

  class CborObject {
    union {
      int64_t intValue;
      double floatValue;
      const uint8_t *stringValue;
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

  void put(uint8_t byte);

  public:
    CborBuffer(unsigned char *outputBuffer, int outputBufferSize);

    ~CborBuffer() {
      printf("copy\n");
    }

    CborBuffer& add(CborObject cborObject);

    int getPos() {
      return pos;
    }

    friend class CBOR;
    friend class CborMap;
};

class CBOR {
  public:

    static CborBuffer::CborObject Int(int64_t value);

    static CborBuffer::CborObject Uint(uint64_t value);

    static CborBuffer::CborObject String(const char* string);

    static CborBuffer::CborObject PreComputed(const uint8_t* cborItem, int length);
};

class CborMap {
  int startPos;
  CborBuffer *cborBuffer;

  public:
    CborMap(CborBuffer&);

    CborMap set(CborBuffer::CborObject key, CborBuffer::CborObject value);

  friend class CborBuffer;
};

