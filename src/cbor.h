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
class CborStructure;

class CborBuffer {

  public:

    enum Error {OK, BUFFER_OVERFLOW, ALREADY_USED};

  private:

    class CborObject {

      union {
        int64_t intValue;
        double floatValue;
        const uint8_t *stringValue;
        CborStructure *cborStructure;
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
      friend class CborStructure;
    };

    uint8_t *buffer;
    int maxBufferLength;
    int currBufferLength;
    CborStructure *root;

    void putByte(uint8_t byte);

    void putBytes(const uint8_t *byteBuffer, int length);

    void encodeTagAndValue(int majorType, int length, uint64_t value);

    void encodeTagAndN(int tag, uint64_t n);

    Error error;

    void setError(Error error);

    bool stillOk();

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
      return currBufferLength;
    }

#ifdef DEBUG_MODE
    void printHex(const char *subject, int startPos, int endPosP1);
    void printHex();
    void printStructuredItems();
    void printOrder();
#endif

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

    static CborBuffer::CborObject Array(CborArray& cborArray);

    static CborBuffer::CborObject Map(CborMap& cborMap);

    static CborBuffer::CborObject PreComputed(const uint8_t* cborItem, int length);
};

class CborStructure {
  int items;
  int startPos;
  int length;
  CborBuffer *cborBuffer;
  CborStructure *next;

  void updateTag();

  virtual int getTag() = 0;

  void putInitialTag();

  void updateStructuredItem(int offset, CborBuffer::CborObject& cborObject);

  void positionItem(int beginItem, CborBuffer::CborObject& object1, CborBuffer::CborObject& object2);

  CborStructure();

#ifdef DEBUG_MODE
  public:
    void printHex();
#endif

  friend class CborBuffer;
  friend class CborMap;
  friend class CborArray;
};

class CborMap : public CborStructure {

  int getTag() {
    return MT_MAP;
  }

  public:
    CborMap() : CborStructure() {
    }

    CborMap* set(CborBuffer::CborObject key, CborBuffer::CborObject value);

  friend class CborBuffer;
};

class CborArray : public CborStructure {

  int getTag() {
    return MT_ARRAY;
  }

  public:
    CborArray() : CborStructure() {
    }

    CborArray* add(CborBuffer::CborObject value);

  friend class CborBuffer;
};

