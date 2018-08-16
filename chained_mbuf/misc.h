
#include <rte_ethdev.h>
#define UNUSED(a) (void)(a)

inline static void init_portconf(struct rte_eth_conf* conf)
{
  memset(conf, 0, sizeof(struct rte_eth_conf));

  /*********************************************\
   * Rx Mode
  \*********************************************/
  conf->rxmode.mq_mode        = ETH_MQ_RX_NONE;
  conf->rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
  conf->rxmode.split_hdr_size = 0;

  /* bit fields */
  conf->rxmode.header_split   = 0;
  conf->rxmode.hw_ip_checksum = 0;
  conf->rxmode.hw_vlan_filter = 0;
  conf->rxmode.hw_vlan_strip  = 0;
  conf->rxmode.hw_vlan_extend = 0;
  conf->rxmode.jumbo_frame    = 0;
  conf->rxmode.hw_strip_crc   = 0;
  conf->rxmode.enable_scatter = 1;
  conf->rxmode.enable_lro     = 0;


  /*********************************************\
   * Tx Mode
  \*********************************************/
  conf->txmode.mq_mode                 = ETH_MQ_TX_NONE;
  conf->txmode.pvid                    = 0;  /* only I40E? */
  conf->txmode.hw_vlan_reject_tagged   = 0;  /* only I40E? */
  conf->txmode.hw_vlan_reject_untagged = 0;  /* only I40E? */
  conf->txmode.hw_vlan_insert_pvid     = 0;  /* only I40E? */


  /*********************************************\
   * Rx Adv Conf
  \*********************************************/
  UNUSED(conf->rx_adv_conf);


  /*********************************************\
   * Tx Adv Conf
  \*********************************************/
  UNUSED(conf->tx_adv_conf);


  /*********************************************\
   * FDIR conf
  \*********************************************/
  UNUSED(conf->fdir_conf);
}
