#include "score_counter.h"

ScoreCounter::ScoreCounter()
    : wins_(0),
      raw_stalls_(0),
      waw_stalls_(0),
      structural_stalls_(0),
      bubbles_(0) {}

void ScoreCounter::record_win() {
    wins_++;
}

void ScoreCounter::record_raw_stall() {
    raw_stalls_++;
}

void ScoreCounter::record_waw_stall() {
    waw_stalls_++;
}

void ScoreCounter::record_structural_stall() {
    structural_stalls_++;
}

void ScoreCounter::record_bubble() {
    bubbles_++;
}
