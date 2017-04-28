// addon.cc
#include <node.h>
#include "ypclwriter.h"

//namespace demo {

using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports) {

	YPCLWriter::Init(exports);
  //YPCLWriter::Init(exports);
}

NODE_MODULE(pclwriter, InitAll)

//}  // namespace demo
