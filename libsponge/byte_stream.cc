#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) {
    pool.resize(capacity, '\0');
    start_idx = 0;
    end_idx = 0;
    _error = false;
    _eof = false;
}

size_t ByteStream::write(const string &data) {
    size_t last_idx = end_idx;
    for (auto &&i : data) {
        if (remaining_capacity() > 0) {
            size_t tar_idx = (end_idx) % pool.size();
            pool[tar_idx] = i;
            ++end_idx;
        } else {
            // error -> full
            set_error();
            break;
            ;
        }
    }
    return end_idx - last_idx;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string s;
    size_t read_num = 0;
    while (read_num < len) {
        s.push_back(pool.at((start_idx + read_num++) % pool.size()));
    }

    return s;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    if (len > buffer_size()) {
        set_error();
    } else {
        start_idx += len;
        if (buffer_empty() && input_ended()) {
            _eof = true;
        }
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string s;
    if (len > buffer_size()) {
        set_error();
    } else {
        s = peek_output(len);
        pop_output(len);

        if (buffer_empty() && input_ended()) {
            _eof = true;
        }
    }

    return s;
}

void ByteStream::end_input() {
    _end_input = true;
    if (buffer_empty()) {
        _eof = true;
    }
}

bool ByteStream::input_ended() const { return _end_input; }

size_t ByteStream::buffer_size() const { return end_idx - start_idx; }

bool ByteStream::buffer_empty() const { return start_idx == end_idx; }

bool ByteStream::eof() const { return _eof; }

size_t ByteStream::bytes_written() const { return end_idx; }

size_t ByteStream::bytes_read() const { return start_idx; }

size_t ByteStream::remaining_capacity() const { return pool.size() - buffer_size(); }
