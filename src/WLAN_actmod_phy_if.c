#include "WLAN_actmod_argparse.h"
#include "bs_pc_2G4.h"
#include "bs_tracing.h"
#include "bs_utils.h"

p2G4_dev_state_s_t dev_phy_state[WLAN_MAX_NBR_PORTS];
static bs_time_t time = 0;
static bs_time_t offset = 0;

static bs_time_t offset_time(bs_time_t time) {
  bs_time_t phy_time;
  if (time != TIME_NEVER) {
    phy_time = time + offset;
  } else {
    phy_time = TIME_NEVER;
  }
  return phy_time;
}

bs_time_t WLAN_actmod_get_time(){
  return time;
}

static int number_ports = 0;

void WLAN_actmod_init_phy_com(WLAN_actmod_args_t *args){
  offset = args->start_offset;
  if ( args->start_offset < 0 ){
    bs_trace_error_line("Time offset (%d) cannot be smaller than 0\n",args->start_offset);
  }
  bs_trace_raw_time(5,"Connecting to Phy in %i ports..\n", args->number_ports);

  number_ports = args->number_ports;
  for (int i = 0 ; i < number_ports; i++) {
    bs_trace_raw_time(8,"Connecting to port %i..\n", args->device_nbr[i]);
    if (p2G4_dev_initcom_s_c(&dev_phy_state[i],
                             args->device_nbr[i],
                             args->s_id,
                             args->p_id,
                             NULL)
          != 0 )
      bs_trace_error_line("Couldn't connect to Phy in port %i\n", args->device_nbr[i]);
  }
}

void WLAN_actmod_disconnect_phy_com(){
  for ( int i = 0 ; i < number_ports; i++ ){
    p2G4_dev_disconnect_s_c(&dev_phy_state[i]);
  }
}

int WLAN_actmod_Tx_in_port(p2G4_tx_t* tx_s, uint port_nbr){
  /*
   * We do a little trick to avoid deadlocking in between the ports
   * (as the phy services one port at a time assuming each port is
   * owned by an independent thread)
   *  We queue ReqQueueDepth-1 requests before we try to pick any response
   *  We keep a FIFO with a list of what we have requested in each port, so
   *  that we remember what to pick back (FIFO_pending_resp)
   *  Each time we are going to send a new request, we check if we have already
   *  queued ReqQueueDepth, and if we did, we pick one response before
   *  proceeding
   *
   *  Right now when we send a Tx request in one port, we send also a wait
   *  response of the same duration in the other ports
   */

  #define ReqQueueDepth 5
  /*
   * Increasing ReqQueueDepth increases slightly the speed of the simulation
   * as we don't context switch so often between this model and the Phy
   */

  typedef enum {None = 0, Wait, Tx} Pending_resp_t;

  static Pending_resp_t FIFO_pending_resp[ReqQueueDepth][WLAN_MAX_NBR_PORTS] = {{0}};
  static int number_pending_requests = 0;
  static int FIFO_read_ptr  = 0;
  static int FIFO_write_ptr = 0;

  if (number_pending_requests == ReqQueueDepth) {
    bs_trace_raw_time(9, "Removing one pending entry\n");
    //let's pick one response from each port:
    for (int i = 0 ; i < number_ports; i++) {
      if (FIFO_pending_resp[FIFO_read_ptr][i] == Wait) {
        bs_trace_raw_time(8, "Port %i pick Wait response\n", i);
        if (p2G4_dev_pick_wait_resp_s_c_b(&dev_phy_state[i]) != 0) {
          dev_phy_state[i].pb_dev_state.connected = false;
          return -1;
        }
      } else if (FIFO_pending_resp[FIFO_read_ptr][i] == Tx) {
        p2G4_tx_done_t tx_done_s;
        bs_trace_raw_time(8, "Port %i pick Tx response\n", i);
        if (p2G4_dev_pick_txresp_s_c_b(&dev_phy_state[i], &tx_done_s ) != 0) {
          dev_phy_state[i].pb_dev_state.connected = false;
          return -1;
        }
      }
    }
    number_pending_requests--;
    FIFO_read_ptr = (FIFO_read_ptr + 1) % ReqQueueDepth;
  }

  bs_trace_raw_time(9, "Sending one requests in each port\n");

  if (tx_s->start_time <= time) {
    char Times[20];
    bs_trace_warning_line_time("Requested Tx starts in the past (%s) == bad error\n",
                               bs_time_to_str(Times, tx_s->start_time));
    return -1;
  }

  time = tx_s->end_time + 1;

  bs_time_t tx_start_time = offset_time(tx_s->start_time);
  bs_time_t time_delta = tx_s->end_time - tx_s->start_time;
  tx_s->start_time = tx_start_time;
  tx_s->end_time = tx_start_time + time_delta;

  pb_wait_t wait_s;
  wait_s.end = tx_s->end_time + 1;
  for (int i = 0 ; i < number_ports; i++) {
    if (i != port_nbr) {
      bs_trace_raw_time(8, "Port %i: requesting Wait\n", i);
      if (p2G4_dev_req_wait_s_c(&dev_phy_state[i], &wait_s) != 0) {
        return -1;
      }
      FIFO_pending_resp[FIFO_write_ptr][i] = Wait;
    } else {
      bs_trace_raw_time(8, "Port %i: requesting Tx\n", i);
      int ret = p2G4_dev_req_tx_s_c(&dev_phy_state[i], tx_s, NULL);
      if ( ret == -1 ){
        return -1;
      }
      FIFO_pending_resp[FIFO_write_ptr][i] = Tx;
    }
  }

  FIFO_write_ptr = ( FIFO_write_ptr + 1 ) % ReqQueueDepth;
  number_pending_requests++;

  return 0;
}
