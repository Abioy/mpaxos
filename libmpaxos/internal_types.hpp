#pragma once
#include <cstdint>
namespace mpaxos {
enum AckType {
  DROP = 0,
  NOT_ENOUGH = 1,
  CONTINUE = 2,
  RESTART = 3,
  CHOOSE = 4
};

using node_id_t = uint16_t;
using slot_id_t = uint64_t;
using ballot_id_t = uint64_t;
using value_id_t = uint64_t;

}  // namespace mpaxos
