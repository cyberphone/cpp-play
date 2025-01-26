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

  CborMap cm;

  CborArray ca;

  CborMap(cborBuffer)
     .set(CBOR::Int(2), CBOR::String("value"))
     ->set(CBOR::Int(-3), CBOR::String("67676"))
     ->set(CBOR::Int(0), CBOR::Map(cm, cborBuffer))
     ->set(CBOR::Int(6), CBOR::Uint(0x8000000000007e00ul))
     ->set(CBOR::Int(-9), CBOR::Array(ca, cborBuffer))
 
;

  
  //  cm.set(CBOR::Int(-2), CBOR::String("h"));

  cborBuffer.add(CBOR::Int(6))
            ->add(CBOR::String("fgfgf"));
  
  cborBuffer.add(CBOR::PreComputed(PRECOMPUTED, sizeof(PRECOMPUTED)));
  
  printf("\n");
  for (int i = 0; i < cborBuffer.getPos(); i++) {
    printf("%02x", cborBuffer.getByte(i));
  }
  printf("\n");

  return 0;
}