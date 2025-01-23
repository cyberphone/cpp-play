#include <stdio.h>

#include "cbor.h"

// const CborBuffer::CborObject MY_KEY = CBOR::Int(4);

int main(void) {
  unsigned char buffer[500];
  CborBuffer cborBuffer(buffer, sizeof(buffer));


  CborMap cborMap;
  cborBuffer.add(cborMap);
  cborBuffer.add(CBOR::Int(6))
            .add(CBOR::String("fgfgf"));
  
  printf("Pos=%d\n", cborBuffer.getPos());

  return 0;
}