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

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"

#include "satellite-utils.h"
#include "satellite-leo-user-phy.h"


NS_LOG_COMPONENT_DEFINE ("SatLeoUserPhy");


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatLeoUserPhy);


TypeId
SatLeoUserPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLeoUserPhy")
    .SetParent<SatGeoUserPhy> ()
    .AddConstructor<SatLeoUserPhy> ()
    .AddAttribute ("BufferSize",
                   "Buffer size, in bytes, to store packets when all GW are disconnected.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&SatLeoUserPhy::m_bufferSizeInBytes),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}


TypeId
SatLeoUserPhy::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}


SatLeoUserPhy::SatLeoUserPhy (void)
  : SatGeoUserPhy (),
  m_bufferSizeInBytes (0),
  m_consumedBufferSize (0),
  m_connected (false)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatLeoUserPhy default constructor is not allowed to use");
}


SatLeoUserPhy::SatLeoUserPhy (SatPhy::CreateParam_t& params,
                              SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
                              Ptr<SatSuperframeConf> superFrameConf)
  : SatGeoUserPhy (params, parameters, superFrameConf),
  m_bufferSizeInBytes (0),
  m_consumedBufferSize (0),
  m_connected (false)
{
  NS_LOG_FUNCTION (this << &params << &parameters << superFrameConf);
}


SatLeoUserPhy::~SatLeoUserPhy ()
{
  NS_LOG_FUNCTION (this);
}


void
SatLeoUserPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_buffer.clear ();
}


void
SatLeoUserPhy::SetConnected (bool connected)
{
  NS_LOG_FUNCTION (this << connected);

  if (connected && !m_connected)
    {
      SendBurstFromBuffer ();
    }
  m_connected = connected;
}


inline uint32_t
BurstSize (Ptr<SatSignalParameters> rxParams)
{
  uint32_t burstSize = 0;
  for (auto const& packet : rxParams->m_packetsInBurst)
    {
      burstSize += packet->GetSize ();
    }
  return burstSize;
}


void
SatLeoUserPhy::Receive (Ptr<SatSignalParameters> rxParams, bool)
{
  NS_LOG_FUNCTION (this << rxParams);

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (rxParams->m_packetsInBurst));

  if (m_connected && m_consumedBufferSize == 0)
    {
      m_rxCallback ( rxParams->m_packetsInBurst, rxParams);
    }
  else
    {
      uint32_t burstSize = BurstSize (rxParams);
      if (m_consumedBufferSize + burstSize <= m_bufferSizeInBytes)
        {
          m_buffer.push_back (rxParams);
          m_consumedBufferSize += burstSize;
        }
      // TODO: else drop?
    }
}


void
SatLeoUserPhy::SendBurstFromBuffer (void)
{
  NS_LOG_FUNCTION (this);

  if (!m_buffer.empty ())
    {
      Ptr<SatSignalParameters> rxParams = m_buffer.front ();
      m_rxCallback ( rxParams->m_packetsInBurst, rxParams);
      Simulator::Schedule (rxParams->m_duration, &SatLeoUserPhy::ClearBurstFromBuffer, this);
    }
}


void
SatLeoUserPhy::ClearBurstFromBuffer (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatSignalParameters> rxParams = m_buffer.front ();
  m_consumedBufferSize -= BurstSize (rxParams);
  m_buffer.erase (m_buffer.begin ());
  SendBurstFromBuffer ();
}


}
