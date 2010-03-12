#pragma once

#include "../chunk_s.h"

/*! \brief MWMO chunk. */
struct MwmoChunk_s : Chunk_s {
  Buffer_t filename_buffer;

  virtual void Initialize() {
    filename_buffer.resize(size);
    CopyData(filename_buffer.data());
  }
};