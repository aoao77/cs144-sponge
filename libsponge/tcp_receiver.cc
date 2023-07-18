#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn == true) {
        _received_syn = true;
        _syn_seq = seg.header().seqno;
    }
    if (_received_syn == true) {
        // printf("initial syn seq : %d    header.seq: %d       ",_syn_seq.raw_value(), seg.header().seqno.raw_value());
        uint64_t absolu_seq = unwrap(seg.header().seqno, _syn_seq, _reassembler.stream_out().bytes_written());
        uint64_t index = seg.header().syn == false ? absolu_seq - 1 : absolu_seq;
        _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);
    }

    // stream_out().write();
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    return _received_syn
               ? wrap(1 + _reassembler.stream_out().bytes_written() + _reassembler.stream_out().input_ended(), _syn_seq)
               : std::optional<WrappingInt32>{};
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
