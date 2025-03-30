#ifndef TAGGED_BLOCK_READER_H
#define TAGGED_BLOCK_READER_H

#include <cstddef>

class TaggedBlockReader {
public:
    TaggedBlockReader (void* data, const size_t data_size) : data_(data), data_size_(data_size) {};
    ~TaggedBlockReader ();
    virtual bool readHeader() = 0;
    virtual bool compileTree() = 0;
protected:
    void* data_;
    size_t data_size_;
    int current_offset = 0;
};

#endif //TAGGED_BLOCK_READER_H
