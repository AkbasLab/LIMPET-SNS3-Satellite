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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-map.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/channel.h"
#include "ns3/uinteger.h"

#include "satellite-leo-net-device.h"
#include "satellite-leo-user-phy.h"
#include "satellite-phy-tx.h"
#include "satellite-phy-rx.h"
#include "satellite-mac.h"
#include "satellite-channel.h"

NS_LOG_COMPONENT_DEFINE ("SatLeoNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLeoNetDevice);

TypeId
SatLeoNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLeoNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<SatLeoNetDevice> ()
    .AddAttribute ("ReceiveErrorModel",
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&SatLeoNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("UserPhy", "The User Phy objects attached to this device.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&SatLeoNetDevice::m_userPhy),
                   MakeObjectMapChecker<SatPhy> ())
    .AddAttribute ("FeederPhy", "The Feeder Phy objects attached to this device.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&SatLeoNetDevice::m_feederPhy),
                   MakeObjectMapChecker<SatPhy> ())
  ;
  return tid;
}

SatLeoNetDevice::SatLeoNetDevice ()
  : m_node (0),
  m_mtu (0xffff),
  m_ifIndex (0),
  m_activeGw (0)
{
  NS_LOG_FUNCTION (this);
}


void
SatLeoNetDevice::ReceiveUser (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << packets.size () << rxParams);
  NS_LOG_INFO ("Receiving a packet at the satellite from user link");
  NS_LOG_INFO ("Sending the packet to the feeder link on beam " << rxParams->m_beamId);
  m_feederPhy[rxParams->m_beamId]->SendPduWithParams (rxParams);
}

void
SatLeoNetDevice::ReceiveFeeder (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << packets.size () << rxParams);
  NS_LOG_INFO ("Receiving a packet at the satellite from feeder link");
  NS_LOG_INFO ("Sending the packet to the user link on beam " << rxParams->m_beamId);
  m_userPhy[rxParams->m_beamId]->SendPduWithParams (rxParams);
}

void
SatLeoNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

void
SatLeoNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}
uint32_t
SatLeoNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}
void
SatLeoNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}
Address
SatLeoNetDevice::GetAddress (void) const
{
  //
  // Implicit conversion from Mac48Address to Address
  //
  NS_LOG_FUNCTION (this);
  return m_address;
}
bool
SatLeoNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}
uint16_t
SatLeoNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}
bool
SatLeoNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void
SatLeoNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this << &callback);
}
bool
SatLeoNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
Address
SatLeoNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}
bool
SatLeoNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
Address
SatLeoNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address SatLeoNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

bool
SatLeoNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
SatLeoNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
SatLeoNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  /**
   * The satellite does not have higher protocol layers which
   * utilize the Send method! Thus, this method should not be used!
   */
  NS_ASSERT (false);
  return false;
}
bool
SatLeoNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);

  /**
   * The satellite does not have higher protocol layers which
   * utilize the SendFrom method! Thus, this method should not be used!
   */
  NS_ASSERT (false);
  return false;
}

Ptr<Node>
SatLeoNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}
void
SatLeoNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}
bool
SatLeoNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
void
SatLeoNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
}

void
SatLeoNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_receiveErrorModel = 0;
  m_userPhy.clear ();
  m_feederPhy.clear ();
  m_connectedGw.clear ();
  NetDevice::DoDispose ();
}

void
SatLeoNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscCallback = cb;
}

bool
SatLeoNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Ptr<Channel>
SatLeoNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return NULL;
}

void
SatLeoNetDevice::AddUserPhy (Ptr<SatPhy> phy, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << phy << beamId);
  m_userPhy.insert (std::pair<uint32_t, Ptr<SatPhy> > (beamId, phy));
}

void
SatLeoNetDevice::AddFeederPhy (Ptr<SatPhy> phy, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << phy << beamId);
  m_feederPhy.insert (std::pair<uint32_t, Ptr<SatPhy> > (beamId, phy));
}

void
ConnectGwHelper (SatLeoNetDevice* self, uint32_t gwId, Ptr<SatGwMac> mac)
{
  self->ConnectGw (gwId, mac);
}

void
DisconnectGwHelper (SatLeoNetDevice* self, uint32_t gwId, Ptr<SatGwMac> mac)
{
  self->DisconnectGw (gwId, mac);
}

void
SatLeoNetDevice::ConnectGw (uint32_t gwId, Ptr<SatGwMac> mac)
{
  NS_LOG_FUNCTION (this << gwId << mac);

  bool shouldConnect = m_connectedGw.empty ();
  auto inserted = m_connectedGw.insert (std::pair<uint32_t, Ptr<SatGwMac>> (gwId, mac));
  mac->SetConnectionCallback (MakeBoundCallback (&DisconnectGwHelper, this, gwId, mac));

  if (shouldConnect && inserted.second)
    {
      for (auto& entry : m_userPhy)
        {
          Ptr<SatLeoUserPhy> phy = DynamicCast<SatLeoUserPhy> (entry.second);
          if (phy)
            {
              phy->SetConnected (true);
            }
        }

      m_activeGw = gwId;
      mac->SetHandleAnyBeam (true);
    }
}

void
SatLeoNetDevice::DisconnectGw (uint32_t gwId, Ptr<SatGwMac> mac)
{
  NS_LOG_FUNCTION (this << gwId << mac);

  mac->SetHandleAnyBeam (false);
  mac->SetConnectionCallback (MakeBoundCallback (&ConnectGwHelper, this, gwId, mac));

  auto gw = m_connectedGw.find (gwId);
  if (gw != m_connectedGw.end ())
    {
      m_connectedGw.erase (gw);
      if (m_connectedGw.empty ())
        {
          m_activeGw = 0;
          for (auto& entry : m_userPhy)
            {
              Ptr<SatLeoUserPhy> phy = DynamicCast<SatLeoUserPhy> (entry.second);
              if (phy)
                {
                  phy->SetConnected (false);
                }
            }
        }
      else if (m_activeGw == gwId)
        {
          // Find any connected GW and turn it active
          for (auto& entry : m_connectedGw)
            {
              m_activeGw = entry.first;
              entry.second->SetHandleAnyBeam (true);
              break;
            }
        }
    }
}

} // namespace ns3
