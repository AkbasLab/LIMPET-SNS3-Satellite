/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 * Author: Mathias Ettinger <mettinger@viveris.toulouse.fr>
 */

#ifndef SAT_LEO_HELPER_H
#define SAT_LEO_HELPER_H

#include <string>
#include "ns3/satellite-geo-helper.h"

namespace ns3 {

/**
 * \brief Creates needed objects for Leo Satellite node like SatLeoNetDevice objects.
 *        Handles needed configuration for the Leo Satellite node.
 *
 */
class SatLeoHelper : public SatGeoHelper
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor.
   */
  SatLeoHelper ();

  /**
   * Create a SatLeoHelper to make life easier when creating Satellite point to
   * point network connections.
   */
  SatLeoHelper (SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                uint32_t rtnLinkCarrierCount,
                uint32_t fwdLinkCarrierCount,
                Ptr<SatSuperframeSeq> seq,
                RandomAccessSettings_s randomAccessSettings);

  virtual ~SatLeoHelper () {}

  /**
   * \param n a node
   *
   * This method creates a ns3::SatLeoNetDevice with the requested attributes
   * and associate the resulting ns3::NetDevice with the ns3::Node.
   *
   * \return pointer to the created device
   */
  Ptr<NetDevice> Install (Ptr<Node> n) override;

  /*
   * Attach the SatChannels for the beam to NetDevice
   * \param dev NetDevice to attach channels
   * \param fr feeder return channel
   * \param uf user forward channel
   * \param uf user return channel
   * \param userAgp user beam antenna gain pattern
   * \param feederAgp feeder beam antenna gain pattern
   * \param beamId Id of the beam
   */
  void AttachChannels ( Ptr<NetDevice> dev,
                        Ptr<SatChannel> ff,
                        Ptr<SatChannel> fr,
                        Ptr<SatChannel> uf,
                        Ptr<SatChannel> ur,
                        Ptr<SatAntennaGainPattern> userAgp,
                        Ptr<SatAntennaGainPattern> feederAgp,
                        uint32_t userBeamId) override;
};

} // namespace ns3

#endif /* SAT_LEO_HELPER_H */
