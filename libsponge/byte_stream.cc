#include "byte_stream.hh"

#include <iostream>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in
// `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : buffer_()
    , capacity_(capacity)
    , is_ended_(false)
    , is_eof_(false)
    , bytes_written_(0)
    , bytes_read_(0)
    , _error(false) {}

size_t ByteStream::write(const string &data) {
    size_t write_amount = remaining_capacity() < data.length()
                              ? remaining_capacity()
                              : data.length();  // min(remaining_capacity(), data.length());
    for (size_t i = 0; i < write_amount; i++) {
        buffer_.push_back(data[i]);
    }
    bytes_written_ += write_amount;
    return write_amount;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t peek_length = len < buffer_size() ? len : buffer_size();
    string out_string(peek_length, ' ');
    for (size_t i = 0; i < peek_length; i++) {
        out_string[i] = buffer_[i];
    }
    return out_string;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t pop_length = len < buffer_size() ? len : buffer_size();
    for (size_t i = 0; i < pop_length; i++)
        buffer_.pop_front();
    bytes_read_ += pop_length;
    if (is_ended_ && buffer_empty())
        is_eof_ = true;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string out = peek_output(len);
    pop_output(len);
    return out;
}

void ByteStream::end_input() {
    if (buffer_.empty())
        is_eof_ = true;
    is_ended_ = true;
}

bool ByteStream::input_ended() const { return is_ended_; }

size_t ByteStream::buffer_size() const { return buffer_.size(); }

bool ByteStream::buffer_empty() const { return buffer_.empty(); }

bool ByteStream::eof() const { return is_eof_; }

size_t ByteStream::bytes_written() const { return bytes_written_; }

size_t ByteStream::bytes_read() const { return bytes_read_; }

size_t ByteStream::remaining_capacity() const { return capacity_ - buffer_.size(); }
