/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathias Ettinger <mathias.ettinger@viveris.fr>
 */

#ifndef SATELLITE_LEO_USER_PHY_H
#define SATELLITE_LEO_USER_PHY_H

#include "satellite-geo-user-phy.h"

namespace ns3 {

class SatLeoUserPhy : public SatGeoUserPhy
{
public:
  /**
   * Default constructor
   */
  SatLeoUserPhy (void);

  SatLeoUserPhy (SatPhy::CreateParam_t& params,
                 SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
                 Ptr<SatSuperframeConf> superFrameConf);

  /**
   * Destructor for SatGeoUserPhy
   */
  virtual ~SatLeoUserPhy ();


  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Dispose of this class instance
   */
  void DoDispose (void) override;

  /**
   * \brief Receives packets from lower layer.
   *
   * \param rxParams Packet reception parameters
   * \param phyError Boolean indicating whether the packet successfully
   * received or not?
   */
  void Receive (Ptr<SatSignalParameters> rxParams, bool phyError) override;

  void SetConnected (bool connected);

private:
  /**
   * Buffer size to store packets when no GW is connected
   */
  uint32_t m_bufferSizeInBytes;
  uint32_t m_consumedBufferSize;

  bool m_connected;
  std::vector< Ptr<SatSignalParameters> > m_buffer;

  void SendBurstFromBuffer (void);
  void ClearBurstFromBuffer (void);
};

}

#endif /* SATELLITE_LEO_USER_PHY_H */
