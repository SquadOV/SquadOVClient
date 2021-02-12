#pragma once

namespace process_watcher::memory::mono::types {

// While this seems useless, it's actually used for us to be able to denote via the type
// whether or not a given struct should be considered a "raw struct." Aka something that
// we can just read from memory directly w/o having to deal with vtables and the like.
struct RawStruct {};

}