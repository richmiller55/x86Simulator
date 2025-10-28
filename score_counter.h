#ifndef SCORE_COUNTER_H
#define SCORE_COUNTER_H

#include <cstdint>

// Tracks pipeline performance metrics.
class ScoreCounter {
public:
    ScoreCounter();

    void record_win(); // e.g., successful forwarding
    void record_raw_stall();
    void record_waw_stall();
    void record_structural_stall();
    void record_bubble(); // General stall/bubble

    // Add getters for these values if needed for UI display

private:
    uint64_t wins_;
    uint64_t raw_stalls_;
    uint64_t waw_stalls_;
    uint64_t structural_stalls_;
    uint64_t bubbles_;
};

#endif // SCORE_COUNTER_H
