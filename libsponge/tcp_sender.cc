#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _retransmission_timeout{retx_timeout}
    , _window_size(1)
    , _ack_seq(0)
    , _consecutive_retransmission_cnt(0)
    , _fin(false)
    , _window_size_zero(false) {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - _ack_seq; }

void TCPSender::fill_window() {
    // send isn
    if (_next_seqno == 0) {
        TCPSegment seg;
        seg.header().seqno = _isn;
        seg.header().syn = true;
        _segments_time.push_back(Timer{seg, _retransmission_timeout});
        _segments_out.push(move(seg));
        _next_seqno++;
        _window_size--;
        // printf("syn seqno++\n");
    }
    size_t read_size = min(static_cast<size_t>(_window_size), _stream.buffer_size() + _stream.input_ended());
    // data exist or fin alone
    if (read_size == 0 /*|| !(_stream.buffer_size() ==0 && _stream.input_ended())*/) {
        return;
    }

    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    // send fin
    if (_stream.buffer_size() + _stream.input_ended() <= _window_size && (_stream.buffer_size() <= MAX_TCP_UNIT) &&
        _stream.input_ended() == true) {
        if (_fin == false) {
            _fin = true;
            seg.header().fin = true;
            seg.payload() = Buffer(_stream.read(min(_stream.buffer_size(), static_cast<size_t>(MAX_TCP_UNIT))));
            read_size = seg.payload().size() + 1;
            _next_seqno += read_size;
            _window_size -= read_size;
            _segments_time.push_back(Timer{seg, _retransmission_timeout});
            _segments_out.push(move(seg));
        }
    } else {
        read_size = min(read_size, static_cast<size_t>(MAX_TCP_UNIT));
        seg.payload() = Buffer(_stream.read(read_size));
        _segments_time.push_back(Timer{seg, _retransmission_timeout});
        _segments_out.push(move(seg));
        _next_seqno += read_size;
        _window_size -= read_size;
        if (_window_size > 0 && _stream.buffer_size() + _stream.input_ended() > 0) {
            fill_window();
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t ack_seq = unwrap(ackno, _isn, _next_seqno);
    bool ack_valid = false;
    // new ack
    if (ack_seq > _ack_seq && ack_seq <= _next_seqno) {
        // printf("seg time size %ld  \n",_segments_time.size());
        for (auto &&i : _segments_time) {
            auto seg = i.segment_time().header().seqno +
                       (i.segment_time().payload().size() + (i.segment_time().header().syn ? 1 : 0) +
                        i.segment_time().header().fin);
            if (unwrap(seg, _isn, _next_seqno) == ack_seq) {
                ack_valid = true;
                break;
            }
        }

        if (ack_valid == true) {
            _ack_seq = ack_seq;
            _window_size = window_size + ack_seq - _next_seqno;
            _consecutive_retransmission_cnt = 0;

            if (window_size == 0) {
                _window_size = 1;
                _window_size_zero = true;
            } else {
                _retransmission_timeout = _initial_retransmission_timeout;
            }

            for (auto it = _segments_time.begin(); it < _segments_time.end();) {
                if (unwrap(it->segment_time().header().seqno, _isn, _next_seqno) < _ack_seq) {
                    it = _segments_time.erase(it);
                } else {
                    it->set_tick_time(_retransmission_timeout);
                    it++;
                }
            }
        }
    }
    // same ack
    else if (((_stream.buffer_size() > 0 || (_fin == false && true == _stream.input_ended())) && ack_seq == _ack_seq &&
              ack_seq + window_size > _next_seqno && ack_seq < _next_seqno)) {
        _window_size = window_size + ack_seq - _next_seqno;
        _consecutive_retransmission_cnt = 0;
        _retransmission_timeout = _initial_retransmission_timeout;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    for (auto it = _segments_time.begin(); it < _segments_time.end(); it++) {
        if ((_consecutive_retransmission_cnt == 0) ||
            (_consecutive_retransmission_cnt > 0 && it->get_retransed() == true)) {
            if (it->tick_time() > ms_since_last_tick) {
                it->fly_tick_time(ms_since_last_tick);
            } else {
                it->set_retransed(true);
                _consecutive_retransmission_cnt++;
                if (_window_size_zero == false) {
                    _retransmission_timeout *= 2;
                }
                it->set_tick_time(_retransmission_timeout);
                _segments_out.push(it->segment_time());
            }
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission_cnt; }

void TCPSender::send_empty_segment() {}
