#include "latencies.h"

std::string outputLatency(size_t x) {
  using namespace std::chrono_literals;
  std::chrono::nanoseconds t(x);

  if (t < 1us) {
    return std::to_string(t.count()) + "ns";
  } else if (t <= 1ms) {
    return std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(t).count()) + "us";
  } else if (t <= 1s) {
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t).count()) + "ms";
  } else {
    return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(t).count()) + "s";
  }
}
