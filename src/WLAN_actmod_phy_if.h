#ifndef WLAN_ACTMOD_PHY_IF_H
#define WLAN_ACTMOD_PHY_IF_H

#include "bs_types.h"
#include "bs_pc_2G4_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void WLAN_actmod_init_phy_com(WLAN_actmod_args_t *args);
void WLAN_actmod_disconnect_phy_com();
int WLAN_actmod_Tx_in_port(p2G4_tx_t* tx_s, uint port_nbr);
bs_time_t WLAN_actmod_get_time();

#ifdef __cplusplus
}
#endif

#endif
