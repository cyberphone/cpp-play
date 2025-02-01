#include <stdio.h>

#include "cbor.h"

// const CborBuffer::CborObject MY_KEY = CBOR::Int(4);

// ["precomputed rocks", true]
static const uint8_t PRECOMPUTED[] = { 
    0x82, 0x71, 0x70, 0x72, 0x65, 0x63, 0x6f, 0x6d, 0x70, 0x75, 
    0x74, 0x65, 0x64, 0x20, 0x72, 0x6f, 0x63, 0x6b, 0x73, 0xf5 };

int main(void) {
  uint8_t buffer[500];
  CborBuffer cborBuffer(buffer, sizeof(buffer));

  CborMap cborMap;

  CborArray cborArray;

  CborMap(cborBuffer)
     .set(CBOR::Int(2), CBOR::String("value"))
     ->set(CBOR::Int(-3), CBOR::String("6789"))
     ->set(CBOR::Int(0), CBOR::Map(cborMap))
     ->set(CBOR::Int(-9), CBOR::Array(cborArray))
     ->set(CBOR::Int(6), CBOR::Uint(0x8000000000007e00ul))

 
;
cborArray.printHex();

  cborArray.add(CBOR::String("0123"))->add(CBOR::String("second element"));

  for (int i = 0; i < 30; i++) { cborArray.add(CBOR::Int(i)); }
  
  cborArray.printHex();
  //  cm.set(CBOR::Int(-2), CBOR::String("h"));

  cborBuffer.add(CBOR::Int(6))
            ->add(CBOR::String("AAAA"));

  for (int i = 0; i < 30; i++) { 
    char string [5];
    sprintf (string, "%d", i);
    cborMap.set(CBOR::Int(i), CBOR::String(string)); 
  }

  cborBuffer.printHex();

  cborMap.printHex();
  cborArray.printHex();
  cborBuffer.printHex("Custom", 134, 192);
  
  cborBuffer.add(CBOR::PreComputed(PRECOMPUTED, sizeof(PRECOMPUTED)));
  cborBuffer.printHex("Custom", 134, 192);
  cborArray.setPosition(134, 192);
   cborArray.add(CBOR::Int(-1));
    cborBuffer.printHex("Custom", 134, 193);
 

  cborBuffer.printHex();

  cborBuffer.printStructuredItems();

  return 0;
}