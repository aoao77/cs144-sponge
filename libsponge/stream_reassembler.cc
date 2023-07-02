#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
    reassemble_vec.resize(capacity,'\0');
    start_idx = 0;
    end_idx = 0;
    inorder_idx = 0;
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    for (size_t i = 0; i < data.size(); i++)
    {
        if ((index + i) >= inorder_idx && (index + i) < (start_idx + _capacity))
        {
            reassemble_vec[(index + i) % _capacity] = data.at(i);
        }
    }
    // update inorder_idx
    size_t last_inorder_idx = inorder_idx;
    for (size_t i = inorder_idx; i < start_idx + _capacity; i++)
    {
        if (reassemble_vec[i % _capacity] != '\0')
        {
            inorder_idx++;
        }
    }
    // eof
    if((inorder_idx - last_inorder_idx) == data.size() && eof == true)
    {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t res = 0;
    for (size_t i = inorder_idx; i < start_idx + _capacity; i++)
    {
        if (reassemble_vec[i % _capacity] != '\0')
        {
            res++;
        }
    }
    return res;
}

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
