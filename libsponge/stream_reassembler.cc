#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , reassemble_vec(_capacity)
    , written_vec(_capacity)
    , start_idx(0)
    , end_idx(0)
    , inorder_idx(0)
    , eof_idx(SIZE_MAX) {
    // printf("DEBUG reassem:capacity %ld\n",_capacity);
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // printf("DEBUG reassem:idx  %ld size %ld \n",index,data.size());
    // clear written flag
    for (size_t i = start_idx; i < inorder_idx; i++) {
        written_vec[i % _capacity] = false;
    }
    // update start_idx
    start_idx = _output.bytes_read();
    // write
    size_t data_idx;
    // size_t read_start_idx = SIZE_MAX;
    for (data_idx = 0; data_idx < data.size(); data_idx++) {
        if ((index + data_idx) >= inorder_idx && (index + data_idx) < (start_idx + _capacity)) {
            // read_start_idx = data_idx;
            reassemble_vec[(index + data_idx) % _capacity] = data.at(data_idx);
            written_vec[(index + data_idx) % _capacity] = true;
        }
    }
    // update inorder_idx
    string substr;
    for (size_t i = inorder_idx; i < start_idx + _capacity; i++) {
        if ((written_vec[i % _capacity] == true)/*  || (read_start_idx != SIZE_MAX && reassemble_vec[i % _capacity]  == data.at(read_start_idx - index + i - inorder_idx)) */)
        {
            inorder_idx++;
            substr.push_back(reassemble_vec[i % _capacity]);
        } else {
            // printf("DEBUG reassem:break  idx %ld char %c \n",i,reassemble_vec[i % _capacity] );
            break;
        }
    }

    // write
    _output.write(substr);
    // printf("DEBUG reassem:written num  %ld \n",_output.bytes_written());
    // eof`
    if (eof_idx == inorder_idx) {
        _output.end_input();
        // printf("DEBUG reassem:inorder_idx  %ld eof_idx %ld \n",inorder_idx,eof_idx);
    }
    if (eof == true) {
        if ((inorder_idx - index) == data.size()) {
            _output.end_input();
            // printf("DEBUG reassem:inorder_idx \n");
        } else if ((index + data_idx) <= (start_idx + _capacity)) {
            // printf("DEBUG reassem:eof %d\n",eof);
            eof_idx = index + data_idx;
        }
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t res = 0;
    for (size_t i = inorder_idx; i < start_idx + _capacity; i++) {
        if (written_vec[i % _capacity] == true) {
            res++;
        }
    }
    return res;
}

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }