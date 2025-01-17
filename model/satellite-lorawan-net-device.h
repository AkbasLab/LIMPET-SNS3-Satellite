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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_LORAWAN_NET_DEVICE_H
#define SATELLITE_LORAWAN_NET_DEVICE_H

#include <ns3/satellite-net-device.h>

#include <ns3/node.h>
#include <ns3/error-model.h>

#include <ns3/satellite-phy.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-llc.h>
#include <ns3/satellite-control-message.h>
#include <ns3/satellite-node-info.h>

namespace ns3 {

/**
 * \ingroup satellite
 * SatLorawanNetDevice to be utilized in the UT and GW nodes for IoT configuration.
 */
class SatLorawanNetDevice : public SatNetDevice
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatLorawanNetDevice ();

  /*
   * \brief Receive the packet from mac layer
   * \param packet Pointer to the packet to be received.
   */
  void Receive (Ptr<const Packet> packet);

  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  bool SendControlMsg (Ptr<SatControlMessage> msg, const Address& dest);

protected:
private:
};

} // namespace ns3

#endif /* SATELLITE_LORAWAN_NET_DEVICE_H */
