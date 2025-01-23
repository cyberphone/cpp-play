#include <stdio.h>

#pragma once

class CborMap;
class CBOR;

class CborBuffer {

  class CborObject {
    union {
      int intValue;
      double floatValue;
      const unsigned char *stringValue;
    } coreData;
    int optionalLength;
    void (* executor)(CborBuffer&, CborObject&);
    static void intExec(CborBuffer&, CborObject&);
    static void uintExec(CborBuffer&, CborObject&);
    static void stringExec(CborBuffer&, CborObject&);

    friend class CborBuffer;
    friend class CBOR;
  };

  unsigned char *buffer;
  int length;
  int pos;

  void put(unsigned char byte);

  public:
    CborBuffer(unsigned char *outputBuffer, int outputBufferSize);

    ~CborBuffer() {
      printf("copy\n");
    }

    CborBuffer& add(CborObject cborObject);

    void add(CborMap& map);

    int getPos() {
      return pos;
    }

    friend class CBOR;
};

class CBOR {
  public:

    static CborBuffer::CborObject Int(int i);

    static CborBuffer::CborObject Uint(unsigned int i);

    static CborBuffer::CborObject String(const char* string);
};

class CborMap {
  int startPos;

  public:
    CborMap();

  friend class CborBuffer;
};

