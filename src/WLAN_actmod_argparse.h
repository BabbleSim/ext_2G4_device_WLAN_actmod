#ifndef BS_WLAN_ARGS_H
#define BS_WLAN_ARGS_H

#include "bs_types.h"
#include "bs_pc_2G4_types.h"
#include "bs_cmd_line_typical.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WLAN_actmod_MAX_TIMES 256

#define WLAN_MAX_NBR_PORTS 255

typedef struct{
  ARG_S_ID
  ARG_P_ID
  unsigned int device_nbr[WLAN_MAX_NBR_PORTS];
  ARG_GDEV_NBR
  ARG_VERB
  ARG_SEED
  unsigned int number_ports;
  double ports_probs[WLAN_MAX_NBR_PORTS];
  ARG_STARTO

  p2G4_power_t powerdBm;
  p2G4_freq_t centerfreq;
  p2G4_modulation_t modulation;

  uint n_times_start;
  uint n_times_end;
  bs_time_t times_start[WLAN_actmod_MAX_TIMES];
  bs_time_t times_end[WLAN_actmod_MAX_TIMES];

  unsigned int PacketMinSize;
  unsigned int PacketMaxSize;
  unsigned int PacketHeaderSize;
  double DataRate;
  double SIFS;
  unsigned int ACKBe;
  double ACKT;
  double AveCWSize;
  double BeaconPeriod;

  double p;
  double sigma;
  double kappa;

  double DIFS;
} WLAN_actmod_args_t;

void WLAN_actmod_argparse(int argc, char *argv[], WLAN_actmod_args_t *args);

#ifdef __cplusplus
}
#endif

#endif
