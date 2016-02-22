////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Michael Hackstein
////////////////////////////////////////////////////////////////////////////////

#include "OperationCursor.h"

//////////////////////////////////////////////////////////////////////////////
/// @brief Get next default batchSize many elements.
///        Check hasMore()==true before using this
///        NOTE: This will throw on OUT_OF_MEMORY
//////////////////////////////////////////////////////////////////////////////

int OperationCursor::getMore() {
  return getMore(_batchSize);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Get next batchSize many elements.
///        Check hasMore()==true before using this
///        NOTE: This will throw on OUT_OF_MEMORY
//////////////////////////////////////////////////////////////////////////////

int OperationCursor::getMore(uint64_t batchSize) {
  // This may throw out of memory
  if (!hasMore()) {
    TRI_ASSERT(false);
    // You requested more even if you should have checked it before.
    return TRI_ERROR_FORBIDDEN;
  }
  // We restart the builder
  _builder->clear();
  VPackArrayBuilder guard(&_builder);
  TRI_doc_mptr_copy_t* mptr = nullptr;
  // TODO: Improve this for baby awareness
  while (batchSize > 0 && _limit > 0 && (mptr = _iterator->next()) != nullptr) {
    --batchSize;
    --_limit;
    _builder.add(VPackSlice(mptr.vpack()));
  }
  if (batchSize > 0 || _limit = 0) {
    // Iterator empty, there is no more
    _hasMore = false;
  }
  return TRI_ERROR_NO_ERROR;
}
