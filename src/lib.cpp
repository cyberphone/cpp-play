#include <stdio.h>
#include <string.h>
#include "cbor.h"

static const uint64_t MASK_LOWER_32 = 0x00000000fffffffful;

CborBuffer::CborBuffer(uint8_t *outputBuffer, int outputBufferSize) {
  buffer = outputBuffer;
  maxBufferLength = outputBufferSize;
  currBufferLength = 0;
  root = NULL;
  error = Error::OK;
}

void CborBuffer::putByte(uint8_t byte) {
  if (currBufferLength < maxBufferLength) {
    buffer[currBufferLength++] = byte;
  } else {
    setError(Error::BUFFER_OVERFLOW);
  }
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

bool CborBuffer::stillOk() {
  return error == CborBuffer::Error::OK;
}

void CborBuffer::setError(Error error) {
  if (stillOk()) {
#ifdef DEBUG_MODE
  printf("\nERROR=%d\n", error);
#endif
    CborBuffer::error = error;
  }
}

#ifdef DEBUG_MODE
void CborBuffer::printHex(const char *subject, int startPos, int endPosP1) {
   printf("\n%s[%d-%d]:\n", subject, startPos, endPosP1 - 1);
   for (int i = startPos; i < endPosP1; i++) {
    printf("%02x", buffer[i]);
  }
  printf("\n");
}

void CborBuffer::printHex() {
  printHex("buffer", 0, currBufferLength);
}

void CborBuffer::printStructuredItems() {
  CborStructure *cborStructure = root;
  while (cborStructure) {
    printf("\nLINKEDITEM");
    cborStructure->printHex();
    cborStructure = cborStructure->next;
  }
}

void CborBuffer::printOrder() {
  CborStructure *cborStructure = root;
  printf("order: ");
  while (cborStructure) {
    printf("%d-%d ", cborStructure->startPos, cborStructure->startPos + cborStructure->length - 1);
    cborStructure = cborStructure->next;
  }
  printf("\n");
}

#endif

CborStructure::CborStructure() {
  // putInitialTag() does the actual initialization
}

void CborStructure::updateTag() {
  if (cborBuffer->stillOk()) {
    if (++items == 24) {
      // Extend buffer with one byte
      cborBuffer->putByte(0);
      // Make space for the additional byte by moving everything above one step
      for (int i = cborBuffer->currBufferLength; --i > startPos; ) {
        cborBuffer->buffer[i] = cborBuffer->buffer[i - 1];
      }
      // The structure just got one byte longer
      length++;
    }
    int modifier = items;
    if (items >= 24) {
      modifier = 24;
      cborBuffer->buffer[startPos + 1] = items;
    }
    cborBuffer->buffer[startPos] = (cborBuffer->buffer[startPos] & 0xe0) | modifier;
  }
}

void CborStructure::putInitialTag() {
  items = 0;
  length = 1;
  startPos = cborBuffer->currBufferLength;
  cborBuffer->putByte(getTag());
  // Add the structured item to the linked list of structured items
  // Order is of no importance
  CborStructure* cborStructure;
  if (cborStructure = cborBuffer->root) {
    while (true) {
      if (cborStructure == this) {
        return;
      }
      if (!cborStructure->next) {
        break;
      }
      cborStructure = cborStructure->next;
    }
    cborStructure->next = this;
  } else {
    cborBuffer->root = this;
  }
}

void CborStructure::positionItem(int beginItem, 
                                 CborBuffer::CborObject& object1, 
                                 CborBuffer::CborObject& object2) {
  // structured-item the-misplaced-items new-item
  //                 |                    |       |
  //                 * startPos + length  |       * endItem
  //                                      * beginItem
  int offset = beginItem - (startPos + length);
  int endItem = cborBuffer->currBufferLength;
 
  if (offset) {
    // There are misplaced items.
    updateStructuredItem(offset, object1);
    updateStructuredItem(offset, object2);
    // Now move into position
    for (int i = 0; i < endItem - beginItem; i++) {
      // Get new-item byte 
      int position = startPos + length + i;
      uint8_t swap = cborBuffer->buffer[beginItem + i];
      // Move misplaced-items one step to the right
      // Yeah, saving memory got priority over speed...
      for (int j = offset; j > 0; j--) {
        cborBuffer->buffer[position + j] = cborBuffer->buffer[position + j - 1];
      }
      // Store the new-item byte in the right position
      cborBuffer->buffer[position] = swap;
    }
  }
  int lengthOfItem = endItem - beginItem;
  // Update length of the structured item
  length += lengthOfItem;
  int totalBytes = lengthOfItem + (items == 24 ? 1 : 0); 
  cborBuffer->printOrder();
  // Potentially relocate other structured
  CborStructure* cborStructure = cborBuffer->root;
  while (cborStructure) {
    // Don't update ourselves
    if (cborStructure != this) {
      // Figuring out what to update
      if (startPos < cborStructure->startPos) {
        if (startPos + length < cborStructure->startPos + cborStructure->length) {
          // Earlier structured object
          cborStructure->startPos += totalBytes;
        }
      } else if (cborStructure->startPos + cborStructure->length + totalBytes >= startPos + length) {
        // Higher in the buffer? Update!
        cborStructure->length += totalBytes;
      }
    }
    cborStructure = cborStructure->next;
  }
  cborBuffer->printOrder();
}

void CborStructure::updateStructuredItem(int offset, CborBuffer::CborObject& cborObject) {
  if (cborObject.executor == CborBuffer::CborObject::structuredExec) {
    cborObject.coreData.cborStructure->startPos -= offset;
  }
}

#ifdef DEBUG_MODE

void CborStructure::printHex() {
  cborBuffer->printHex(getTag() == MT_ARRAY ? "array" : "map", startPos, startPos + length);
}
#endif

CborArray* CborArray::add(CborBuffer::CborObject value) {
  CborBuffer::CborObject nullItem;
  nullItem.executor = NULL;
  updateTag();
  int beginItem = cborBuffer->currBufferLength;
  cborBuffer->add(value);
  positionItem(beginItem, value, nullItem);
  return this;
}

CborMap* CborMap::set(CborBuffer::CborObject key, CborBuffer::CborObject value) {
  updateTag();
  int beginItem = cborBuffer->currBufferLength;
  cborBuffer->add(key);
  cborBuffer->add(value);
  positionItem(beginItem, key, value);
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
}

void CborBuffer::CborObject::uintExec(CborBuffer* cborBuffer,
                                      CborBuffer::CborObject& cborObject) {
  cborBuffer->encodeTagAndN(MT_UNSIGNED, (uint64_t)cborObject.coreData.intValue);
}

void CborBuffer::CborObject::stringExec(CborBuffer* cborBuffer,
                                        CborBuffer::CborObject& cborObject) {
  cborBuffer->encodeTagAndN(MT_TEXT_STRING, cborObject.optionalLength);
  cborBuffer->putBytes(cborObject.coreData.stringValue, cborObject.optionalLength);
}

void CborBuffer::CborObject::preComputedExec(CborBuffer* cborBuffer, 
                                             CborBuffer::CborObject& cborObject) {
  for (int length = 0; length < cborObject.optionalLength; ) {
    cborBuffer->putByte(cborObject.coreData.stringValue[length++]);
  }
}

void CborBuffer::CborObject::structuredExec(CborBuffer* cborBuffer,
                                            CborBuffer::CborObject& cborObject) {
  cborObject.coreData.cborStructure->cborBuffer = cborBuffer;
  cborObject.coreData.cborStructure->putInitialTag();
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