#include "bs_types.h"
#include "bs_tracing.h"
#include "bs_utils.h"
#include "bs_rand_main.h"
#include "WLAN_actmod_argparse.h"
#include "WLAN_actmod_phy_if.h"
#include <signal.h>

/**
 * This function calculates when the next WLAN transmission will start and end,
 * given that <Now> the previous one has just finished
 */
static inline
void calculate_next_transmission(bs_time_t Now, bs_time_t *times_start,
                                 bs_time_t *times_end, WLAN_actmod_args_t *args)
{
  //All calculations in us (double)
  double SilenceT = args->DIFS;

  //we calculate a contention window:
  SilenceT = SilenceT + bs_random_uniformR(0, args->AveCWSize * 2); //a U(0,AveCWSize*2)

  if (bs_random_uniform() >= args->p) { //and we check if there is or not just now a new transmission
    //if we don't actually want to transmit, we calculate how far away in the future we will
    SilenceT = SilenceT + bs_random_GPRND(args->kappa, args->sigma*1000, 0);
  }

  if ( SilenceT > args->BeaconPeriod * 1e6 ) { //to have at least "beacons"
    SilenceT = args->BeaconPeriod * 1e6;
  }

  //And we calculate the Tx duration
  //We draw a random packet:
  double packet_size = bs_random_uniformRi( args->PacketMinSize, args->PacketMaxSize) * 8.0/ args->DataRate;
  //And the total TX time (including the silent SIFS time is)
  double tx_size = packet_size + args->PacketHeaderSize/args->DataRate
                 + args->SIFS + args->ACKT + args->ACKBe/args->DataRate;

  *times_start = Now + SilenceT;
  *times_end   = Now + SilenceT + tx_size;
}

static unsigned int choose_a_port(WLAN_actmod_args_t *args){
  //do a random drop and chose from which port this burst will go
  if (args->number_ports == 1)
    return 0;

  double drop = bs_random_uniform(); //a random number in [0..1]

  for (int i = 0 ; i < args->number_ports ; i ++) {
    if (drop < args->ports_probs[i]) {
      return i;
    } else {
      drop -= args->ports_probs[i];
    }
  }
  //This should really not happen as args->ports_probs[*] adds to 1
  return args->number_ports;
}

int main(int argc, char *argv[]) {
  WLAN_actmod_args_t args;

  /*
   * Let's ensure that even if we are redirecting to a file, we get stdout line
   * buffered (default for console)
   * Note that glibc ignores size, just in case we set a reasonable number in
   * case somebody tries to compile against a different library
   */
  setvbuf(stdout, NULL, _IOLBF, 512);
  setvbuf(stderr, NULL, _IOLBF, 512);

  //when disconnecting we are not careful to check if the Phy has already closed its end
  signal(SIGPIPE, SIG_IGN);

  bs_trace_set_prefix("d_??:");
  bs_trace_register_time_function(WLAN_actmod_get_time);
  WLAN_actmod_argparse(argc, argv, &args);

  bs_trace_raw(9,"Connecting...\n");
  WLAN_actmod_init_phy_com(&args);

  bs_random_init(args.rseed);

  int i;
  bs_time_t Time = 0;
  p2G4_tx_t tx_s;

  tx_s.radio_params.modulation = args.modulation;
  tx_s.packet_size = 0;
  tx_s.phy_address = 0;
  tx_s.radio_params.center_freq = args.centerfreq;
  tx_s.power_level = args.powerdBm;
  tx_s.abort.abort_time = TIME_NEVER;
  tx_s.abort.recheck_time = TIME_NEVER;

  for (i = 0 ; i < args.n_times_start ; i ++) {
    if ( args.times_start[i] <= Time ) {
      bs_trace_error_line("The list of times needs to be ordered (index %i:"
          " %i <= %i)\n",
          i, args.times_start[i], Time);
    }
    if (args.times_end[i] <= args.times_start[i]) {
      bs_trace_error_line("End times need to be bigger than starting times "
          "(index %i: %i <= %i)\n",
          i, args.times_end[i], args.times_start[i]);
    }

    Time = args.times_start[i];

    while (Time < args.times_end[i]) {
      bs_time_t start_time, end_time;

      calculate_next_transmission(Time, &start_time, &end_time, &args);

      if ( end_time > args.times_end[i]){
        break;
      }
      tx_s.start_time = start_time;
      tx_s.end_time = end_time;

      if (WLAN_actmod_Tx_in_port(&tx_s, choose_a_port(&args))) {
        i = args.n_times_start; //to also exit the for loop
        break; //while
      }

      Time = end_time;
    }
  }

  bs_trace_raw_time(9,"Disconnecting...\n");
  WLAN_actmod_disconnect_phy_com();

  return 0;
}
