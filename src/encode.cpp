#include "hphp/runtime/base/base-includes.h"
#include <bson.h>
#include "encode.h"

namespace HPHP {

void fillBSONWithArray(const Array& value, bson_t* bson) {
  for (ArrayIter iter(value); iter; ++iter) {
      Variant key(iter.first());
      const Variant& data(iter.secondRef());
      
      variantToBSON(data, key.toString().c_str(), bson);
  }
}

void variantToBSON(const Variant& value, const char* key, bson_t* bson) {
  switch(value.getType()) {
    case KindOfUninit:
      case KindOfNull:
        nullToBSON(key, bson);
        break;
      case KindOfBoolean:
        boolToBSON(value.toBoolean(), key, bson);
        break;
      case KindOfInt64:
        int64ToBSON(value.toInt64(), key, bson);
        break;
      case KindOfDouble:
        doubleToBSON(value.toDouble(), key, bson);
        break;
      case KindOfStaticString:
      case KindOfString:
        stringToBSON(value.toString(), key, bson);
        break;
      case KindOfArray:
        arrayToBSON(value.toArray(), key, bson);
        break;  
      default:
        throw NotImplementedException("bson_decode");
  }
}


void arrayToBSON(const Array& value, const char* key, bson_t* bson) {
  bson_t child;
  bool isDocument = arrayIsDocument(value);
  if (isDocument) {
    bson_append_document_begin(bson, key, -1, &child);
  } else {
    bson_append_array_begin(bson, key, -1, &child);
  }

  fillBSONWithArray(value, &child);

  if (isDocument) {
    bson_append_document_end(bson, &child);
  } else {
    bson_append_array_end(bson, &child);
  }
}

void doubleToBSON(const double value,const char* key, bson_t* bson) {
  bson_append_double(bson, key, -1, value);
}

void nullToBSON(const char* key, bson_t* bson) {
  bson_append_null(bson, key, -1);
}

void boolToBSON(const bool value, const char* key, bson_t* bson) {
  bson_append_bool(bson, key, -1, value);
}

void int64ToBSON(const int64_t value, const char* key, bson_t* bson) {
  bson_append_int32(bson, key, -1, value);
}

void stringToBSON(const String& value, const char* key, bson_t* bson) {
  bson_append_utf8(bson, key, strlen(key), value.c_str(), -1);
}

bool arrayIsDocument(const Array& arr) {
  int64_t max_index = 0;

  for (ArrayIter it(arr); it; ++it) {
    Variant key = it.first();
    if (!key.isNumeric()) {
      return true;
    }
    int64_t index = key.toInt64();
    if (index < 0) {
      return true;
    }
    if (index > max_index) {
      max_index = index;
    }
  }

  if (max_index >= arr.size() * 2) {
    // Might as well store it as a map
    return true;
  }

  return false;
}
} 