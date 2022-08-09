#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
  // check syn
  if (seg.header().syn) {
    isn_ = seg.header().seqno;
  }
  if (!isn_.has_value()) return;

  // check fin
  if (seg.header().fin)
    fin_seq_ = unwrap(seg.header().seqno, isn_.value(), seqno_) +
               seg.length_in_sequence_space();

  // compute index(absolute seqno)
  uint64_t index = unwrap(seg.header().seqno, isn_.value(), seqno_);
  if (!seg.header().syn) index--;
  reassembler_.push_substring(seg.payload().copy(), index, seg.header().fin);
  // update the seqno
  seqno_ = reassembler_.stream_out().bytes_written() + 1;
  if (fin_seq_.has_value() && fin_seq_.value() == seqno_ + 1) seqno_++;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
  return isn_.has_value() ? wrap(seqno_, isn_.value()) : isn_;
}

size_t TCPReceiver::window_size() const {
  return capacity_ - (reassembler_.stream_out().bytes_written() -
                      reassembler_.stream_out().bytes_read());
}
