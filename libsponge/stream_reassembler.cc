#include "stream_reassembler.hh"

#include <cassert>

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in
// `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : buffer_(capacity)
    , cache_header_(0)
    , unassembled_bytes_(0)
    , eof_byte_(0)
    , is_eof_set_(false)
    , _output(capacity)
    , _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (_output.input_ended())
        return;

    const size_t max_byte = _output.bytes_read() + _capacity;
    const size_t index_start = std::max(_output.bytes_written(), index);
    size_t index_end = std::min(max_byte, index + data.length());
    // check for eof
    if (eof) {
        is_eof_set_ = true;
        eof_byte_ = index + data.length();
    }
    if (is_eof_set_)
        index_end = std::min(index_end, eof_byte_);

    // write the data
    for (size_t write_index = index_start; write_index < index_end; write_index++) {
        size_t cache_index = (cache_header_ + write_index - _output.bytes_written()) % _capacity;
        assert(!(buffer_[cache_index].valid && buffer_[cache_index].ch != data[write_index - index]));
        if (!buffer_[cache_index].valid)
            unassembled_bytes_++;
        buffer_[cache_index].valid = true;
        buffer_[cache_index].ch = data[write_index - index];
    }
    // calculate the contiguous bytes
    size_t contiguous_len = 0;
    while (contiguous_len < _capacity && buffer_[(cache_header_ + contiguous_len) % _capacity].valid) {
        contiguous_len++;
    }
    // output the contiguous bytes
    if (contiguous_len > 0) {
        string out_string(contiguous_len, 0);
        for (size_t i = 0; i < contiguous_len; i++) {
            out_string[i] = buffer_[(cache_header_ + i) % _capacity].ch;
            buffer_[(cache_header_ + i) % _capacity].valid = false;
        }
        _output.write(out_string);
        unassembled_bytes_ -= contiguous_len;
        cache_header_ = (cache_header_ + contiguous_len) % _capacity;
    }
    if (is_eof_set_ && _output.bytes_written() >= eof_byte_)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return unassembled_bytes_; }

bool StreamReassembler::empty() const { return unassembled_bytes_ == 0; }
