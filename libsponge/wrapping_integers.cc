#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32{static_cast<uint32_t>((n + isn.raw_value()) % (1LU << 32))};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    // printf("_syn_seq is %d  n_seq is %d written bytes is %ld   ",isn.raw_value(), n.raw_value() , checkpoint);
    uint32_t absolu_diff = n.raw_value() > isn.raw_value() ? n - isn : (1LU << 32) - (isn - n);
    if (absolu_diff > checkpoint) {
        return absolu_diff;
    }

    uint32_t quotient = (checkpoint - absolu_diff) / (1LU << 32);
    uint32_t remainder = (checkpoint - absolu_diff) % (1LU << 32);

    return (remainder < (1LU << 32) / 2) ? (1LU << 32) * quotient + absolu_diff
                                         : (1LU << 32) * (quotient + 1) + absolu_diff;
}
