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

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "../model/satellite-const-variables.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-leo-net-device.h"
#include "../model/satellite-geo-feeder-phy.h"
#include "../model/satellite-leo-user-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-phy-rx-carrier-conf.h"
#include "../model/satellite-channel-estimation-error-container.h"
#include "satellite-leo-helper.h"
#include "satellite-helper.h"
#include <ns3/satellite-typedefs.h>


NS_LOG_COMPONENT_DEFINE ("SatLeoHelper");


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatLeoHelper);


TypeId
SatLeoHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLeoHelper")
    .SetParent<SatGeoHelper> ()
    .AddConstructor<SatLeoHelper> ()
  ;
  return tid;
}


TypeId
SatLeoHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this );

  return GetTypeId ();
}


SatLeoHelper::SatLeoHelper ()
  : SatGeoHelper ()
{
  NS_LOG_FUNCTION (this);

  // this default constructor should be never called
  NS_ASSERT (false);
}


SatLeoHelper::SatLeoHelper (SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                            uint32_t rtnLinkCarrierCount,
                            uint32_t fwdLinkCarrierCount,
                            Ptr<SatSuperframeSeq> seq,
                            RandomAccessSettings_s randomAccessSettings)
  : SatGeoHelper (bandwidthConverterCb, rtnLinkCarrierCount, fwdLinkCarrierCount, seq, randomAccessSettings)
{
  NS_LOG_FUNCTION (this << rtnLinkCarrierCount << fwdLinkCarrierCount );

  m_deviceFactory.SetTypeId ("ns3::SatLeoNetDevice");
}


Ptr<NetDevice>
SatLeoHelper::Install (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this << n);

  NS_ASSERT (m_deviceCount == 0);

  // Create SatLeoNetDevice
  Ptr<SatLeoNetDevice> satDev = m_deviceFactory.Create<SatLeoNetDevice> ();

  satDev->SetAddress (Mac48Address::Allocate ());
  n->AddDevice (satDev);
  m_deviceCount++;
  m_nodeId = n->GetId ();

  return satDev;
}


void
SatLeoHelper::AttachChannels (Ptr<NetDevice> d, Ptr<SatChannel> ff, Ptr<SatChannel> fr, Ptr<SatChannel> uf, Ptr<SatChannel> ur, Ptr<SatAntennaGainPattern> userAgp, Ptr<SatAntennaGainPattern> feederAgp, uint32_t userBeamId )
{
  NS_LOG_FUNCTION (this << d << ff << fr << uf << ur << userAgp << feederAgp << userBeamId);

  Ptr<SatLeoNetDevice> dev = DynamicCast<SatLeoNetDevice> (d);

  SatPhy::CreateParam_t params;
  params.m_beamId = userBeamId;
  params.m_device = d;
  params.m_txCh = uf;
  params.m_rxCh = ur;

  /**
   * Simple channel estimation, which does not do actually anything
   */
  Ptr<SatChannelEstimationErrorContainer> cec = Create<SatSimpleChannelEstimationErrorContainer> ();

  SatPhyRxCarrierConf::RxCarrierCreateParams_s parametersUser = SatPhyRxCarrierConf::RxCarrierCreateParams_s ();
  parametersUser.m_daIfModel = m_daRtnLinkInterferenceModel;
  parametersUser.m_raIfModel = m_raSettings.m_raRtnInterferenceModel;
  parametersUser.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
  parametersUser.m_bwConverter = m_carrierBandwidthConverter;
  parametersUser.m_carrierCount = m_rtnLinkCarrierCount;
  parametersUser.m_cec = cec;
  parametersUser.m_raCollisionModel = m_raSettings.m_raCollisionModel;
  parametersUser.m_randomAccessModel = m_raSettings.m_randomAccessModel;

  SatPhyRxCarrierConf::RxCarrierCreateParams_s parametersFeeder = SatPhyRxCarrierConf::RxCarrierCreateParams_s ();
  parametersFeeder.m_daIfModel = m_daFwdLinkInterferenceModel;
  parametersFeeder.m_raIfModel = m_raSettings.m_raFwdInterferenceModel;
  parametersFeeder.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
  parametersFeeder.m_bwConverter = m_carrierBandwidthConverter;
  parametersFeeder.m_carrierCount = m_fwdLinkCarrierCount;
  parametersFeeder.m_cec = cec;
  parametersFeeder.m_raCollisionModel = m_raSettings.m_raCollisionModel;
  parametersFeeder.m_randomAccessModel = m_raSettings.m_randomAccessModel;

  Ptr<SatLeoUserPhy> uPhy = CreateObject<SatLeoUserPhy> (
      params,
      parametersUser,
      m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE));

  params.m_txCh = fr;
  params.m_rxCh = ff;

  Ptr<SatGeoFeederPhy> fPhy = CreateObject<SatGeoFeederPhy> (
      params,
      parametersFeeder,
      m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE));

  SatPhy::ReceiveCallback uCb = MakeCallback (&SatLeoNetDevice::ReceiveUser, dev);
  SatPhy::ReceiveCallback fCb = MakeCallback (&SatLeoNetDevice::ReceiveFeeder, dev);

  uPhy->SetAttribute ("ReceiveCb", CallbackValue (uCb));
  fPhy->SetAttribute ("ReceiveCb", CallbackValue (fCb));

  // Note, that currently we have only one set of antenna patterns,
  // which are utilized in both in user link and feeder link, and
  // in both uplink and downlink directions.
  uPhy->SetTxAntennaGainPattern (userAgp);
  uPhy->SetRxAntennaGainPattern (userAgp);
  fPhy->SetTxAntennaGainPattern (feederAgp);
  fPhy->SetRxAntennaGainPattern (feederAgp);

  dev->AddUserPhy (uPhy, userBeamId);
  dev->AddFeederPhy (fPhy, userBeamId);

  uPhy->Initialize ();
  fPhy->Initialize ();

  // Create a node info to PHY layers
  Mac48Address address = Mac48Address::ConvertFrom (d->GetAddress ());

  Ptr<SatNodeInfo> niUser = Create <SatNodeInfo> (SatEnums::NT_SAT, m_nodeId, address);
  uPhy->SetNodeInfo (niUser);

  Ptr<SatNodeInfo> niFeeder = Create <SatNodeInfo> (SatEnums::NT_SAT, m_nodeId, address);
  fPhy->SetNodeInfo (niFeeder);
}


} // namespace ns3
