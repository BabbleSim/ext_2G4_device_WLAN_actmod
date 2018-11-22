#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include "WLAN_actmod_argparse.h"
#include "bs_utils.h"
#include "bs_tracing.h"
#include "bs_oswrap.h"
#include "bs_cmd_line.h"
#include "bs_cmd_line_typical.h"
#include "bs_utils.h"
#include "bs_pc_2G4_utils.h"

char executable_name[] = "bs_device_2G4_WLAN_actmod";

void component_print_post_help(){
  fprintf(stdout,"\n\
A ConfigSet defines all parameters from DataRate downto kappa.\n\
Instead of choosing a <ConfigSet>, these parameters can be selected\n\
one by one. Or a set can be chosen, and some of its parameters replaced\n\
\n\
Note that parameters are processed from left to right\n\
(whatever is set later overwrites the previous settings)\n\
\n\
\n\
This WLAN model can be made to start and stop several times.\n\
Use the timestart and timeend options to do so:\n\
\n\
If both of these parameters are omitted, it will run from the beginning to\n\
the end of the simulation\n\
\n\
This model, although very simplistic generates patterns of channel occupancy\n\
similar to what a real WLAN would generate while its underlying traffic\n\
pattern would remain static.\n\
\n\
Documentation of this model can be found in the doc folder\n\
\n\
This model may generate traffic into one Phy port or several. The\n\
distribution of traffic in ports is done uniformly given each port probability\n\
(<port<x>_probab>)\n"
  );
}

static void SetParams_fromConfigSet(char *ConfigSetName, WLAN_actmod_args_t *args) {

  args->PacketHeaderSize = 160; //bits
  args->DataRate = 54; //Mbps
  args->SIFS  = 16;   //useconds (wait time between packet and ACK)
  args->ACKBe = 135;  //bits (an ack is assumed to have ACKTus + ACKBe bits at datarate
  args->ACKT  = 21.5; //us

  args->AveCWSize = 67.5; //us ( average size of the contention window ) JUST 1 DEVICE and the router
  //args->AveCWSize = 350; //us ( average size of the contention window ) MANY DEVICES and high activity
  args->BeaconPeriod = 200e-3; //seconds (Somewhere in [20ms, 1 second] )

  //fully random traffic size
  args->PacketMinSize = 128; //bytes
  args->PacketMaxSize = 1536;//bytes

  if ( strcasecmp(ConfigSetName,"VoIP") == 0 ) {
    args->PacketMinSize = 128; //bytes
    args->PacketMaxSize = 384;//bytes
    args->p     = 0.1;  //probability of having a consecutive transmission
    args->sigma = 11.5; //ms (scale parameter of the generalized pareto)
    args->kappa = -0.45; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"VideoConf") == 0 ) {
    args->PacketMinSize = 500; //bytes
    args->PacketMaxSize = 1536;//bytes
    args->p     = 0.10;  //probability of having a consecutive transmission
    args->sigma = 12; //ms (scale parameter of the generalized pareto)
    args->kappa = -0.45; //shape parameter of the generalized pareto
  } else if (   ( strcasecmp(ConfigSetName,"FileDownload") == 0 )
             || ( strcasecmp(ConfigSetName,"100") == 0 ) ) {
    args->PacketMinSize = 1300; //bytes
    args->PacketMaxSize = 1536;//bytes
    args->p     = 0.988;  //probability of having a consecutive transmission
    args->sigma = 0.04; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.5; //shape parameter of the generalized pareto
  } else if (   ( strcasecmp(ConfigSetName,"FileDownload_24M") == 0 )
             || ( strcasecmp(ConfigSetName,"100_24M") == 0 ) ) {
    args->DataRate = 24;
    args->PacketMinSize = 1300; //bytes
    args->PacketMaxSize = 1536;//bytes
    args->p     = 0.99;  //probability of having a consecutive transmission
    args->sigma = 0.04; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.5; //shape parameter of the generalized pareto
  } else if (   ( strcasecmp(ConfigSetName,"FileDownload_12M") == 0 )
             || ( strcasecmp(ConfigSetName,"100_12M") == 0 ) ) {
    args->DataRate = 12;
    args->PacketMinSize = 1300; //bytes
    args->PacketMaxSize = 1536;//bytes
    args->p     = 0.99;  //probability of having a consecutive transmission
    args->sigma = 0.04; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.5; //shape parameter of the generalized pareto
  } else if (   ( strcasecmp(ConfigSetName,"FileDownload_6M") == 0 )
             || ( strcasecmp(ConfigSetName,"100_6M") == 0 ) ) {
    args->DataRate = 6;
    args->PacketMinSize = 1300; //bytes
    args->PacketMaxSize = 1536;//bytes
    args->p     = 0.988;  //probability of having a consecutive transmission
    args->sigma = 0.04; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.5; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"50") == 0 ) {
    args->p     = 0.55;  //probability of having a consecutive transmission
    args->sigma = 0.45; //ms (scale parameter of the generalized pareto)
    args->kappa = 0; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"50_24M") == 0 ) {
    args->DataRate = 24;
    args->p     = 0.5;  //probability of having a consecutive transmission
    args->sigma = 0.5; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.2; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"50_12M") == 0 ) {
    args->DataRate = 12;
    args->p     = 0.45;  //probability of having a consecutive transmission
    args->sigma = 0.7; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.4; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"50_6M") == 0 ) {
    args->DataRate = 6;
    args->p     = 0.4;  //probability of having a consecutive transmission
    args->sigma = 0.87; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.6; //shape parameter of the generalized pareto

  } else if ( strcasecmp(ConfigSetName,"25") == 0 ) {
    args->p     = 0.45;  //probability of having a consecutive transmission
    args->sigma = 1.4; //ms (scale parameter of the generalized pareto)
    args->kappa = 0; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"25_24M") == 0 ) {
    args->DataRate = 24;
    args->p     = 0.4;  //probability of having a consecutive transmission
    args->sigma = 1.5; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.15; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"25_12M") == 0 ) {
    args->DataRate = 12;
    args->p     = 0.3;  //probability of having a consecutive transmission
    args->sigma = 2.0; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.27; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"25_6M") == 0 ) {
    args->DataRate = 6;
    args->p     = 0.24;  //probability of having a consecutive transmission
    args->sigma = 2.9; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.41; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"10") == 0 ) {
    args->p     = 0.35;  //probability of having a consecutive transmission
    args->sigma = 4; //ms (scale parameter of the generalized pareto)
    args->kappa = 0; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"10_24M") == 0 ) {
    args->DataRate = 24;
    args->p     = 0.3;  //probability of having a consecutive transmission
    args->sigma = 4; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.1; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"10_12M") == 0 ) {
    args->DataRate = 12;
    args->p     = 0.3;  //probability of having a consecutive transmission
    args->sigma = 4.5; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.47; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"10_6M") == 0 ) {
    args->DataRate = 6;
    args->p     = 0.3;  //probability of having a consecutive transmission
    args->sigma = 8.5; //ms (scale parameter of the generalized pareto)
    args->kappa = 0.47; //shape parameter of the generalized pareto
  } else if ( strcasecmp(ConfigSetName,"5") == 0 ) {
    args->p     = 0.2;  //probability of having a consecutive transmission
    args->sigma = 6.5; //ms (scale parameter of the generalized pareto)
    args->kappa = 0; //shape parameter of the generalized pareto
  } else {
    bs_trace_error_line("ConfigSetName \"%s\" is not known\n",ConfigSetName);
  }
}

static p2G4_freq_t CenterFreq_from_WLANChannel(int WChannel) {
  double center;
  p2G4_freq_t centerfreq;
  center = 2407 + WChannel * 5;
  if (WChannel == 14) {
    center = 2484;
  }
  p2G4_freq_from_d(center , 0, &centerfreq);
  return centerfreq;
}

WLAN_actmod_args_t *args_g;

static void cmd_trace_lvl_found(char * argv, int offset) {
  bs_trace_set_level(args_g->verb);
}

static void cmd_gdev_nbr_found(char * argv, int offset) {
  bs_trace_set_prefix_dev(args_g->global_device_nbr);
}

static void cmd_nports_found(char * argv, int offset) {
  if ( args_g->number_ports > WLAN_MAX_NBR_PORTS ){
    bs_trace_error_line("You configured %i ports but the maximum is %i (%s)\n",
                        args_g->number_ports, WLAN_MAX_NBR_PORTS, argv);
  }
}

static double doub;
static char *str;

static void cmd_power_found(char * argv, int offset) {
  args_g->powerdBm = p2G4_power_from_d(doub);
}

static int WChannel;
static void cmd_channel_found(char * argv, int offset) {
  if (( WChannel < 1 ) || ( WChannel > 14 )) {
    bs_trace_error_line("Error parsing channel (only channels between 1 and 14 allowed) (%s)\n", argv);
  }
  args_g->centerfreq = CenterFreq_from_WLANChannel(WChannel);
  bs_trace_raw(9,"=> center frequency set to %.2lfMHz (+2400)\n", ((double)args_g->centerfreq)/(1<<P2G4_freq_FRACB));
}
static void cmd_cfreq_found(char * argv, int offset) {
  int error = p2G4_freq_from_d(doub, 0, &args_g->centerfreq);
  if (error) {
    bs_trace_error_line("Could not parse center frequency (%s)\n", argv);
  }
  bs_trace_raw(9,"center frequency set to %.2lfMHz (+2400)\n", ((double)args_g->centerfreq)/(1<<P2G4_freq_FRACB));
}
static void cmd_mod_found(char * argv, int offset){
  int error = p2G4_modulation_from_string(str, &args_g->modulation, OnlyNonReceivable, 1);
  if ( error != 0 ) {
    bs_trace_error_line("Could not interpret modulation type %s\n",&argv[offset]);
  }
}
static void cmd_cfgs_found(char * argv, int offset){
  SetParams_fromConfigSet(str, args_g);
}
static void cmd_datarate_found(char * argv, int offset) {
  if ((args_g->DataRate < 1) || (args_g->DataRate > 54)) {
    bs_trace_warning_line("DataRate (%.3fMbps) is weird.. (expected 1..54Mbps) (%s)\n", args_g->DataRate, argv);
  }
}
static void cmd_mins_found(char * argv, int offset) {
  if ((args_g->PacketMinSize < 80 ) || ( args_g->PacketMinSize > 1536)) {
    bs_trace_warning_line("Minimum packet size(%i) is weird.. (expected 80..1536) (%s)\n", args_g->PacketMinSize , argv);
  }
}
static void cmd_maxs_found(char * argv, int offset) {
  if ((args_g->PacketMaxSize < 80) || (args_g->PacketMaxSize > 1536)) {
    bs_trace_warning_line("Maximum packet size(%i) is weird.. (expected 80..1536) (%s)\n", args_g->PacketMaxSize , argv);
  }
}
static void cmd_SIFS_found(char * argv, int offset) {
  if ( ( args_g->SIFS < 10 ) || ( args_g->SIFS > 100 ) ) {
    bs_trace_warning_line("SIFS (%f) is weird.. (expected 10 to 100us) (%s)\n", args_g->SIFS);
  }
}

static void cmd_ACKT_found(char * argv, int offset) {
  if ( ( args_g->ACKT < 5 ) || ( args_g->ACKT > 200 ) ) {
    bs_trace_warning_line("ACKT (%f) is weird.. (expected 5 to 200us) (%s)\n", args_g->ACKT , argv);
  }
}

static void cmd_AveCWSize_found(char * argv, int offset) {
  if ( ( args_g->AveCWSize < 5 ) || ( args_g->AveCWSize > 2000 ) ) {
    bs_trace_warning_line("AveCWSize (%f) is weird.. (expected 5 to 2000us) (%s)\n", args_g->AveCWSize , argv);
  }
}

static void cmd_beap_found(char * argv, int offset) {
  if ((args_g->BeaconPeriod < 10e-3) || (args_g->BeaconPeriod > 2)) {
    bs_trace_warning_line("BeaconPeriod (%f) is weird.. (expected 10e-3 to 2s) (%s)\n", args_g->BeaconPeriod , argv);
  }
}

static void cmd_p_found(char * argv, int offset) {
  if ((args_g->p < 0) || (args_g->p > 1)) {
    bs_trace_error_line("p out of range (%f) (expected 0 to 1) (%s)\n", args_g->p , argv);
  }
}
static void cmd_sigma_found(char * argv, int offset) {
  if ((args_g->sigma < 0) || (args_g->sigma > 100)) {
    bs_trace_warning_line("sigma looks out of range (%f) (expected 0 to 100ms) (%s)\n", args_g->sigma , argv);
  }
}

/**
 * Check the arguments provided in the command line: set args based on it or
 * defaults, and check they are correct
 */
void WLAN_actmod_argparse(int argc, char *argv[], WLAN_actmod_args_t *args)
{

  args_g = args;
  bs_args_struct_t args_struct[] = {
      /*manual,mandatory,switch,option,     name ,               type,   destination,         callback,             , description*/
      ARG_TABLE_S_ID,
      ARG_TABLE_P_ID_2G4,
      { false, true  , false, "d",       "port_number0",         'u', (void*)&args->device_nbr[0],NULL,      "device number (port in phy) (equivalent to -d0=<>)"},
      ARG_TABLE_GDEV_NBR,
      ARG_TABLE_VERB,
      ARG_TABLE_STARTO,
      ARG_TABLE_SEED,
      { false, false , false, "nports",  "number_ports",         'u', (void*)&args->number_ports,cmd_nports_found,"(1) number of ports to connect to in the phy" },
      { true,  false , false, "d<x>",    "port_number<x>",       'u', NULL                    ,NULL,         "device/port number in phy to be used for port <x>" },
      { true,  false , false, "pp<x>",   "port<x>_probab",       'u', NULL                    ,NULL,         "probability of a transmission going out thru port <x>" },
      { false, false , false, "power",   "powerdBm",             'f', (void*)&doub            ,cmd_power_found,"In dBm power of the interference (20dBm)" },
      { false, false , false, "channel", "WLANChannel",          'i', (void*)&WChannel        ,cmd_channel_found,"WLAN channel to use" },
      { false, false , false, "centerfreq","centerf",            'f', (void*)&doub            ,cmd_cfreq_found,"Centerfreq in MHz (can be given instead of -channel)" },
      { false, false , false, "mod",      "modtype",             's', (void*)&str             ,cmd_mod_found,"{CW,(WLAN),BLE,WN{1|2|4|8|16|20|40|80}} Type of interference (default WLAN)" },
      { false, false , false, "ConfigSet","ConfigSet",           's', (void*)&str             ,cmd_cfgs_found,"One of the predefined sets of parameters {VoIP,VideoConf,FileDownload,100,50,25,10,(5)} where 100,50,25,10 & 5 model one device driving traffic at 100%%,..,5%% of the WLAN total capacity (Only for <DataRate>=54Mbps) See documentation for a longer description\n" },
      { false, false , false, "DataRate" ,"DataRate",            'f', (void*)&(args->DataRate),cmd_datarate_found,"WLAN datarate in Mbits/s (54)" },
      { false, false , false, "PacketMinSize" ,"PminSize",       'u', (void*)&(args->PacketMinSize),cmd_mins_found,"Minimum size of the WLAN packets in bytes (128)" },
      { false, false , false, "PacketMaxSize" ,"PmaxSize",       'u', (void*)&(args->PacketMaxSize),cmd_maxs_found,"Maximum size of the WLAN packets in bytes (1536)" },
      { false, false , false, "PacketHeaderSize" ,"PHSize",      'u', (void*)&(args->PacketHeaderSize),NULL,  "Size of the WLAN headers in bits(160)" },
      { false, false , false, "SIFS"      ,"SIFS",               'f', (void*)&(args->SIFS)    ,cmd_SIFS_found,"WLAN SIFS time in us (16)" },
      { false, false , false, "ACKBe"     ,"ACKBe",              'u', (void*)&(args->ACKBe)   ,NULL,          "ACK duration in bits at DataRate (will be added to ACKT) (135)" },
      { false, false , false, "ACKT"      ,"ACKT",               'f', (void*)&(args->ACKT)    ,cmd_ACKT_found,"ACK duration in us (will be added to ACKBe) (21.5)" },
      { false, false , false, "AveCWSize" ,"AveCWSize",          'f', (void*)&(args->AveCWSize),cmd_AveCWSize_found,"Average contention window size in us [(67.5)..20460]" },
      { false, false , false, "BeaconPeriod" ,"BeacP",           'f', (void*)&(args->BeaconPeriod),cmd_beap_found,"Period of the WLAN beacons,in seconds [10e-3..(200e-3)..2]" },
      { false, false , false, "pr"        ,"pr",                 'f', (void*)&(args->p)       ,cmd_p_found,  "Probability of having a new packet right after finalizing transmitting a previous one (0.2)" },
      { false, false , false, "sigma"     ,"sigma",              'f', (void*)&(args->sigma)   ,cmd_sigma_found,"Generalized Pareto scale factor (in ms) (6.5)" },
      { false, false , false, "kappa"     ,"kappa",              'f', (void*)&(args->kappa)   ,NULL,         "Generalized Pareto shape factor (0)\n" },
      { true,  false , false, "timestart" ,"time",               'l', NULL                    ,NULL,         "Times at which the active periods shall start (omit to run from the beginning) (up to " STR(WLAN_actmod_MAX_TIMES) " periods)" },
      { true,  false , false, "timeend" ,"time",                 'l', NULL                    ,NULL,         "Times at which the activity periods shall end (omit the last to run until the end of the simulation) (up to " STR(WLAN_actmod_MAX_TIMES) " periods)" },
      ARG_TABLE_ENDMARKER
  };

  bs_args_set_defaults(args_struct);
  args->verb = 2;
  bs_trace_set_level(args->verb);
  args->rseed = 0xFFFF;
  args->start_offset = 0;
  args->number_ports = 1;
  for (int i = 0; i < WLAN_MAX_NBR_PORTS; i++){
    args->device_nbr[i] = UINT_MAX;
    args->ports_probs[i] = 0;
  }
  args->ports_probs[0] = 1; //by default there is only 1 port where everything goes

  args->n_times_start = 0;
  args->n_times_end   = 0;
  args->modulation    = P2G4_MOD_WLANINTER;
  args->centerfreq    = P2G4_INVALID_FREQ;
  args->powerdBm = p2G4_power_from_d(20);
  args->PacketMinSize = 128;
  args->PacketMaxSize = 1536;
  args->PacketHeaderSize = 160;
  args->DataRate = 54;
  args->SIFS  = 16;
  args->ACKBe = 135;
  args->ACKT  = 21.5;
  args->AveCWSize = 67.5;
  args->BeaconPeriod = 200e-3;
  args->p = 0.2;
  args->sigma = 6.5;
  args->kappa = 0;
  static char default_phy[] ="2G4";

  for (int i=1; i<argc; i++) {
    int offset;
    uint index;
    if ( !bs_args_parse_one_arg(argv[i], args_struct) ){ //Attempt to find and parse it between the non-manual options
      if ((offset = bs_is_multi_opt(argv[i], "d", &index, 1))) {
        if (index < args->number_ports) {
          char name[50];
          snprintf(name, 50, "port_number%i", index);
          bs_read_optionparam(&argv[i][offset], (void*)&(args->device_nbr[index]), 'u', name);
        } else {
          bs_trace_error_line("You configured %i ports but tried to set the device numbr for port %i (%s) [you need to give -nports option before any -d<port> option] \n", args->number_ports, index, argv[i]);
        }
      }
      else if ((offset = bs_is_multi_opt(argv[i], "pp", &index, 1))) {
        if (index < args->number_ports) {
          char name[50];
          snprintf(name, 50, "port%i_prob", index);
          bs_read_optionparam(&argv[i][offset], (void*)&(args->ports_probs[index]), 'f', name);
        } else {
          bs_trace_error_line("You configured %i ports but tried to set the probability port %i (%s) [you need to give -np option before any -pp<port> option]\n", args->number_ports, index, argv[i]);
        }
      }
      else if ((offset = bs_is_option(argv[i], "timestart", 0))) {
        double time;
        while ((i + 1 < argc) && (argv[i+1][0] != '-')) {
          i += 1;
          if (sscanf(argv[i],"%lf",&time) != 1) {
            bs_trace_error_line("Could not parse timestart entry nbr %i (%s)\n", args->n_times_start+1, argv[i]);
          }
          args->times_start[args->n_times_start] = time;
          bs_trace_raw(9,"added timestart[%i] = %"PRItime" to list\n", args->n_times_start, args->times_start[args->n_times_start]);
          args->n_times_start += 1;
        }
      }
      else if ((offset = bs_is_option(argv[i], "timeend", 0))) {
        double time;
        while ((i + 1 < argc) && (argv[i+1][0] != '-')) {
          i += 1;
          if (sscanf(argv[i],"%lf",&time) != 1) {
            bs_trace_error_line("Could not parse timeend entry nbr %i (%s)\n", args->n_times_end+1, argv[i]);
          }
          args->times_end[args->n_times_end] = time;
          bs_trace_raw(9,"added timeend[%i] = %"PRItime" to list\n", args->n_times_end, args->times_end[args->n_times_end]);
          args->n_times_end += 1;
        }
      } else {
        bs_args_print_switches_help(args_struct);
        bs_trace_error_line("Unknown command line switch '%s'\n",argv[i]);
      }
    }
  } 

  if (!args->s_id) {
    bs_args_print_switches_help(args_struct);
    bs_trace_error_line("The command line option <simulation ID> needs to be set\n");
  }
  if (!args->p_id) {
    args->p_id = default_phy;
  }

  double total_prob = 0;
  for (int i = 0 ; i < args->number_ports ; i++) {
    if ( args->device_nbr[i] == UINT_MAX ) {
      bs_args_print_switches_help(args_struct);
      bs_trace_error_line("The command line option <port_number%i> needs to be"
                          " set (%i ports set)\n", i, args->number_ports);
    }
    total_prob += args->ports_probs[i];
  }
  double warning_tolerance = 1e-3;
  if ((total_prob > 1.0 + warning_tolerance) || (total_prob < 1.0 - warning_tolerance)) {
    //if the ports probabilities don't add up to 1 by a big amount, we tell the user
    bs_trace_raw(2, "All ports probability doesn't add up to 100% but to "
                    "%0.2f%%, I will re-scale them proportionally\n",
                    total_prob*100.0);
  }
  //we always scale the ports probabilities so they add up to 1 together
  for (int i = 0;  i < args->number_ports ; i++) {
    args->ports_probs[i] /= total_prob;
  }
  for (int i = 1 ; i < args->number_ports ; i++) {
    if (args->device_nbr[i] <= args->device_nbr[i-1] )
      bs_trace_error_line("device ports need to be in ascending order "
                          "(otherwise it will deadlock)\n");
  }

  if (args->global_device_nbr == UINT_MAX) {
    args->global_device_nbr = args->device_nbr[0];
    bs_trace_set_prefix_dev(args->global_device_nbr);
  }

  if (args->centerfreq == P2G4_INVALID_FREQ) {
    bs_args_print_switches_help(args_struct);
    bs_trace_error_line("The WLAN channel or center frequency needs to be "
                        "specified\n");
  }

  if (args->n_times_start == 0) {
    args->n_times_start = 1;
    args->times_start[0] = 1;
  }
  if (args->n_times_end == args->n_times_start - 1) {
    args->n_times_end = args->n_times_start ;
    args->times_end[args->n_times_end-1] = TIME_NEVER;
  }
  if ( args->n_times_end != args->n_times_start ){
    bs_trace_error_line("You need to provide the same number of end and start "
                        "times (or one less if you want to let the last burst "
                        "run forever) (run with --help for more info)\n");
  }

  args->DIFS = args->SIFS*2 + 2*9; //we assume the slot time is 9us
}
