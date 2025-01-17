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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#ifndef SATELLITE_UT_MAC_H
#define SATELLITE_UT_MAC_H

#include <ns3/ptr.h>
#include <ns3/callback.h>
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>
#include <ns3/nstime.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-queue.h>
#include <ns3/satellite-ut-scheduler.h>
#include <ns3/satellite-signal-parameters.h>
#include <ns3/satellite-random-access-container.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-beam-scheduler.h>
#include <utility>

namespace ns3 {

class Packet;
class Mac48Address;
class SatSuperframeSeq;
class SatNodeInfo;
class SatFrameConf;
class SatTbtpMessage;
class SatWaveform;
class SatTimeSlotConf;
class SatTbtpContainer;
class UniformRandomVariable;

/**
 * \ingroup satellite
 * \brief UT specific Mac class for Sat Net Devices.
 *
 * This SatUtMac class specializes the MAC class with UT characteristics. UT MAC
 * receives BB frames intended for it (including at least one packet intended for it)
 * from the forward link and disassembles the GSE PDUs from the BB frame. BB frame may
 * hold also TBTP messages from GW (forward link) including allocations for the UT.
 * UT schedules the tx opportunities to LLC via UT scheduler. UT forwards a container
 * of packets to the PHY layer. Packet container models the Frame PDU, including one or
 * more RLE PDUs.
 *
 */
class SatUtMac : public SatMac
{
public:
  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);
  /**
   * Derived from Object
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor, which is not used.
   */
  SatUtMac ();

  /**
   * Construct a SatUtMac
   *
   * This is the constructor for the SatUtMac
   *
   * \param seq Pointer to superframe sequence.
   * \param beamId Id of the beam.
   * \param crdsaOnlyForControl CRDSA buffer operation mode
   */
  SatUtMac (Ptr<SatSuperframeSeq> seq, uint32_t beamId, bool crdsaOnlyForControl);

  /**
   * Destroy a SatUtMac
   *
   * This is the destructor for the SatUtMac.
   */
  ~SatUtMac ();

  /**
   * Receive packet from lower layer.
   *
   * \param packets Pointers to packets received.
   */
  virtual void Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/);

  /**
   * \return Timing advance as Time object.
   */
  typedef Callback<Time> TimingAdvanceCallback;

  /**
   * \brief Set the timing advance callback
   * \param cb callback to invoke whenever a timing advance is needed by SatUtMac object.
   */
  void SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb);

  /**
   * Callback for informing the amount of dedicated access
   * bytes received from TBTP
   * \param uint8_t   RC index
   * \param uint32_t  Sum of bytes in this superframe
   */
  typedef Callback<void, uint8_t, uint32_t> AssignedDaResourcesCallback;

  /**
   * \brief Set the assigned DA resources callback
   * \param cb callback to invoke whenever TBTP is received.
   */
  void SetAssignedDaResourcesCallback (SatUtMac::AssignedDaResourcesCallback cb);

  /**
   * Callback to check if TX is operational
   */
  typedef Callback<bool> TxCheckCallback;

  /**
   * \brief Set the TX check callback
   * \param cb callback to invoke when checking if TX is enabled
   */
  void SetTxCheckCallback (SatUtMac::TxCheckCallback cb);

  /**
   * Callback to check if TX is operational
   */
  typedef Callback<void, uint8_t> SliceSubscriptionCallback;

  /**
   * \brief Get sliec subscription info from MAC layer
   * \param cb callback to invoke slices subscription changes
   */
  void SetSliceSubscriptionCallback (SatUtMac::SliceSubscriptionCallback cb);

  /**
   * Callback to send a logon message to the gateway
   */
  typedef Callback<void> SendLogonCallback;

  /**
   * \brief Set the logon callback
   * \param cb callback to invoke when sending a logon message
   */
  void SetSendLogonCallback (SatUtMac::SendLogonCallback cb);

  /**
   * Callback to update GW address to SatRequestManager
   */
  typedef Callback<void, Mac48Address> UpdateGwAddressCallback;

  /**
   * \brief Set the gw update callback
   * \param cb callback to update GW address to SatRequestManager
   */
  void SetUpdateGwAddressCallback (SatUtMac::UpdateGwAddressCallback cb);

  /**
   * Callback to get the SatBeamScheduler from the beam ID for handover
   */
  typedef Callback<Ptr<SatBeamScheduler>, uint32_t> BeamScheculerCallback;

  /**
   * \brief Set the beam scheduler callback
   * \param cb Callback to get the SatBeamScheduler
   */
  void SetBeamScheculerCallback (SatUtMac::BeamScheculerCallback cb);

  /**
   * Get Tx time for the next possible superframe.
   * \param superFrameSeqId Superframe sequence id
   * \return Time Time to transmit
   */
  Time GetNextSuperFrameTxTime (uint8_t superFrameSeqId) const;

  /**
   * Receive a queue event:
   * - FIRST_BUFFER_RCVD
   * - BUFFER_EMPTY
   * /param event Queue event from SatQueue
   * /param rcIndex Identifier of the queue
   */
  virtual void ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex);

  /**
   * Receive a queue event:
   * - FIRST_BUFFER_RCVD
   * - BUFFER_EMPTY
   * /param event Queue event from SatQueue
   * /param rcIndex Identifier of the queue
   */
  virtual void ReceiveQueueEventEssa (SatQueue::QueueEvent_t event, uint8_t rcIndex);

  /**
   * Receive a logon message to transmit
   * /param packet The logon packet to send
   */
  void SendLogon (Ptr<Packet> packet);

  /**
   * Set address of the GW (or its MAC) serving this UT.
   *
   * \param gwAddress Address of the GW.
   */
  void SetGwAddress (Mac48Address gwAddress);

  /**
   * Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * Set RA channel assigned for this UT.
   *
   * \param raChannel RA channel assigned to this UT.
   */
  void SetRaChannel (uint32_t raChannel);

  /**
   * Get RA channel assigned for this UT.
   *
   * \return RA channel assigned to this UT.
   */
  uint32_t GetRaChannel () const;

  /**
   * \brief Set the random access module
   * \param randomAccess
   */
  void SetRandomAccess (Ptr<SatRandomAccess> randomAccess);

  /**
   * \brief Method to check whether a transmission of a control msg
   * is somewhat possible. Transmission cannot be guaranteed, but at least
   * the possibility to do so (e.g. random access / CRDSA enabled, TBTP with
   * time slots received).
   * \return Boolean to indicate whether a control msg transmission is possible
   */
  bool ControlMsgTransmissionPossible () const;

  /**
   * \brief Method to check whether a transmission of a logon msg
   * is somewhat possible.
   * \return Boolean to indicate whether a logon msg transmission is possible
   */
  bool LogonMsgTransmissionPossible () const;

  /**
   * \brief Callback signature for `DaResourcesTrace` trace source.
   * \param size the amount of assigned TBTP resources (in bytes) in the
   *             superframe for this UT
   */
  typedef void (* TbtpResourcesTraceCallback)(uint32_t size);

  /**
   * \brief Callback to reconfigure physical layer during handover.
   * \param uint32_t new beam Id
   */
  typedef Callback<void, uint32_t> HandoverCallback;

  /**
   * \brief Method to set handover callback.
   * \param cb callback to invoke whenever a TIM-U is received prompting us to switch beams
   */
  void SetHandoverCallback (SatUtMac::HandoverCallback cb);

  /**
   * \brief Callback to update gateway address after handover
   * \param Mac48Address the address of the new gateway
   */
  typedef Callback<void, Mac48Address> GatewayUpdateCallback;

  /**
   * \brief Method to set the gateway address update callback
   * \param cb callback to invoke to update gateway address
   */
  void SetGatewayUpdateCallback (SatUtMac::GatewayUpdateCallback cb);

  /**
   * \brief Callback to update routing and ARP tables after handover
   * \param Address the address of this device
   * \param Address the address of the new gateway
   */
  typedef Callback<void, Address, Address> RoutingUpdateCallback;

  /**
   * \brief Method to set the routing update callback
   * \param cb callback to invoke to update routing
   */
  void SetRoutingUpdateCallback (SatUtMac::RoutingUpdateCallback cb);

  /**
   * \brief Callback to check whether the current beam is still the best one
   * to use for sending data; and sending handover recommendation if not
   * \param uint32_t the current beam ID
   */
  typedef Callback<bool, uint32_t> BeamCheckerCallback;

  /**
   * \brief Callback to ask for the best beam ID during handover
   * \return The best beam ID
   */
  typedef Callback<uint32_t> AskedBeamCallback;

  /**
   * \brief Method to set the beam checker callback
   * \param cb callback to invoke to check beams and recommend handover
   */
  void SetBeamCheckerCallback (SatUtMac::BeamCheckerCallback cb);

  /**
   * \brief Method to get the best beam when performing handover
   * \param cb callback to invoke to ask best beam ID
   */
  void SetAskedBeamCallback (SatUtMac::AskedBeamCallback cb);

  void LogOff ();

  void SetLogonChannel (uint32_t channelId);

protected:
  /**
   * Dispose of SatUtMac
   */
  void DoDispose (void);

private:
  /**
   * \brief Get start time for the current superframe.
   * \param superFrameSeqId Superframe sequence id
   * \return Time Time to transmit
   */
  Time GetCurrentSuperFrameStartTime (uint8_t superFrameSeqId) const;

  uint32_t GetCurrentSuperFrameId (uint8_t superFrameSeqId) const;

  /**
   * \brief Do random access evaluation for Tx opportunities
   * \param randomAccessTriggerType
   */
  void DoRandomAccess (SatEnums::RandomAccessTriggerType_t randomAccessTriggerType);

  /**
   * \brief Function for selecting the allocation channel for the current RA evaluation
   * \return allocation channel ID
   */
  uint32_t GetNextRandomAccessAllocationChannel ();

  /**
   * \brief Function for scheduling the Slotted ALOHA transmissions
   * \param allocationChannel allocation channel
   */
  void ScheduleSlottedAlohaTransmission (uint32_t allocationChannel);

  /**
   * \brief Function for scheduling the ESSA transmissions
   * \param allocationChannel RA allocation channel
   */
  void ScheduleEssaTransmission (uint32_t allocationChannel);

  /**
   * \brief Function for scheduling the CRDSA transmissions
   * \param allocationChannel RA allocation channel
   * \param txOpportunities Tx opportunities
   */
  void ScheduleCrdsaTransmission (uint32_t allocationChannel, SatRandomAccess::RandomAccessTxOpportunities_s txOpportunities);

  /**
   *
   */
  void CreateCrdsaPacketInstances (uint32_t allocationChannel, std::set<uint32_t> slots);

  /**
   * \brief Function for removing the past used RA slots
   * \param superFrameId super frame ID
   */
  void RemovePastRandomAccessSlots (uint32_t superFrameId);

  /**
   * \brief Function for updating the used RA slots
   * \param superFrameId super frame ID
   * \param allocationChannel allocation channel
   * \param slot RA slot
   * \return was the update successful
   */
  bool UpdateUsedRandomAccessSlots (uint32_t superFrameId, uint32_t allocationChannel, uint32_t slot);

  /**
   *
   * \param opportunityOffset
   * \param frameConf
   * \param timeSlotCount
   * \param superFrameId
   * \param allocationChannel
   * \return
   */
  std::pair<bool, uint32_t> FindNextAvailableRandomAccessSlot (Time opportunityOffset,
                                                               Ptr<SatFrameConf> frameConf,
                                                               uint32_t timeSlotCount,
                                                               uint32_t superFrameId,
                                                               uint32_t allocationChannel);

  /**
   *
   * \param superframeStartTime
   * \param frameConf
   * \param timeSlotCount
   * \param superFrameId
   * \param allocationChannel
   * \return
   */
  std::pair<bool, uint32_t> SearchFrameForAvailableSlot (Time superframeStartTime,
                                                         Ptr<SatFrameConf> frameConf,
                                                         uint32_t timeSlotCount,
                                                         uint32_t superFrameId,
                                                         uint32_t allocationChannel);

  /**
   *
   * \param superFrameId
   * \param allocationChannelId
   * \param slotId
   * \return
   */
  bool IsRandomAccessSlotAvailable (uint32_t superFrameId, uint32_t allocationChannelId, uint32_t slotId);

  /**
   *
   */
  void PrintUsedRandomAccessSlots ();

  /**
   *  Schedules time slots according to received TBTP message.
   *
   * \param tbtp Pointer to TBTP message.
   */
  void ScheduleTimeSlots (Ptr<SatTbtpMessage> tbtp);

  /**
   * Schdules one Tx opportunity, i.e. time slot.
   * \param transmitDelay time when transmit possibility starts
   * \param duration duration of the burst
   * \param wf waveform
   * \param tsConf Time slot conf
   * \param carrierId Carrier id used for the transmission
   */
  void ScheduleDaTxOpportunity (Time transmitDelay, Time duration, Ptr<SatWaveform> wf, Ptr<SatTimeSlotConf> tsConf, uint32_t carrierId);

  /**
   * Notify the upper layer about the Tx opportunity. If upper layer
   * returns a PDU, send it to lower layer.
   *
   * \param duration duration of the burst
   * \param carrierId Carrier id used for the transmission
   * \param wf waveform
   * \param tsConf Time slot conf
   * \param policy UT scheduler policy
   */
  void DoTransmit (Time duration, uint32_t carrierId, Ptr<SatWaveform> wf, Ptr<SatTimeSlotConf> tsConf, SatUtScheduler::SatCompliancePolicy_t policy = SatUtScheduler::LOOSE);

  /**
   * Notify the upper layer about the Slotted ALOHA Tx opportunity. If upper layer
   * returns a PDU, send it to lower layer.
   *
   * \param duration duration of the burst
   * \param waveform waveform
   * \param carrierId Carrier id used for the transmission
   * \param rcIndex RC index
   */
  void DoSlottedAlohaTransmit (Time duration, Ptr<SatWaveform> waveform, uint32_t carrierId, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy = SatUtScheduler::LOOSE);

  /**
   * Notify the upper layer about the ESSA Tx opportunity. If upper layer
   * returns a PDU, send it to lower layer.
   *
   * \param duration duration of the burst
   * \param waveform waveform
   * \param carrierId Carrier id used for the transmission
   * \param rcIndex RC index
   */
  void DoEssaTransmit (Time duration, Ptr<SatWaveform> waveform, uint32_t carrierId, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy = SatUtScheduler::LOOSE);

  /**
   *
   * \param payloadBytes Tx opportunity payload
   * \param type Time slot type
   * \param rcIndex RC index
   * \param policy Scheduler policy
   * \return
   */
  SatPhy::PacketContainer_t FetchPackets (uint32_t payloadBytes, SatTimeSlotConf::SatTimeSlotType_t type, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy);

  /**
   * \brief Extract packets from the underlying queue and put them in the provided container
   * \param packets Container for the packets to extract
   * \param payloadBytes Tx opportunity payload
   * \param type Time slot type
   * \param rcIndex RC index
   * \param policy Scheduler policy
   * \param randomAccessChannel whether the packets are to be sent on RA or DA
   */
  void ExtractPacketsToSchedule (SatPhy::PacketContainer_t& packets,
                                 uint32_t payloadBytes,
                                 SatTimeSlotConf::SatTimeSlotType_t type,
                                 uint8_t rcIndex,
                                 SatUtScheduler::SatCompliancePolicy_t policy,
                                 bool randomAccessChannel);

  /**
   *
   * \param packets
   * \param duration
   * \param carrierId
   * \param txInfo
   */
  void TransmitPackets (SatPhy::PacketContainer_t packets, Time duration, uint32_t carrierId, SatSignalParameters::txInfo_s txInfo);

  /**
   * Signaling packet receiver, which handles all the signaling packet
   * receptions.
   * \param packet Received signaling packet
   */
  void ReceiveSignalingPacket (Ptr<Packet> packet);

  /**
   * \brief Function which is executed at every frame start.
   */
  void DoFrameStart ();

  SatUtMac& operator = (const SatUtMac &);
  SatUtMac (const SatUtMac &);

  /**
   * Used superframe sequence for the return link
   */
  Ptr<SatSuperframeSeq> m_superframeSeq;

  /**
   * Callback for getting the timing advance information
   */
  TimingAdvanceCallback m_timingAdvanceCb;

  /**
   * Callback for informing the assigned TBTP resources
   */
  AssignedDaResourcesCallback m_assignedDaResourcesCallback;

  /**
   * \brief RA main module
   */
  Ptr<SatRandomAccess> m_randomAccess;

  /**
   * Guard time for time slots. The guard time is modeled by shortening
   * the duration of a time slot by a m_guardTime set by an attribute.
   */
  Time m_guardTime;

  /**
   * Container for storing all the TBTP information related to this UT.
   */
  Ptr<SatTbtpContainer> m_tbtpContainer;

  /**
   * \brief Uniform random variable distribution generator
   */
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  /**
   * \brief A container for storing the used RA slots in each frame and allocation channel
   */
  std::map < std::pair<uint32_t, uint32_t>, std::set<uint32_t> > m_usedRandomAccessSlots;

  /**
   * RA channel assigned to the UT.
   */
  uint32_t m_raChannel;

  /**
   * RA channel dedicated to logon messages
   */
  uint32_t m_logonChannel;

  /**
   * UT is logged on
   */
  bool m_loggedOn;

  /**
   * Should logon be simulated?
   */
  bool m_useLogon;

  /**
   * Number of times a logon message has been sent without response
   */
  uint32_t m_sendLogonTries;

  /**
   * The initial max time to wait when sending a logon message
   */
  Time m_windowInitLogon;

  /**
   * Timeout for waiting for a response for a logon message
   */
  Time m_maxWaitingTimeLogonResponse;
  /**
   * The random generator for waiting transmission time
   */
  Ptr<UniformRandomVariable> m_waitingTimeLogonRng;

  /**
   * Instant when a logon message can be transmitted
   */
  Time m_nextLogonTransmissionPossible;


  /**
   * UT scheduler
   */
  Ptr<SatUtScheduler> m_utScheduler;

  /**
   * Assigned TBTP resources in superframe for this UT (in bytes).
   */
  TracedCallback<uint32_t> m_tbtpResourcesTrace;

  /**
   * CRDSA packet ID (per frame)
   */
  uint32_t m_crdsaUniquePacketId;

  /**
   * Planned CRDSA usage:
   * - true -> only for control
   * - false -> for control and user data
   */
  bool m_crdsaOnlyForControl;

  /**
   * Next time when a next ESSA packet can be safely sent.
   */
  Time m_nextPacketTime;

  /**
   * Flag that indicates if a method DoRandomAccess is scheduled for
   * asynchronous access.
   */
  bool m_isRandomAccessScheduled;

  class SatTimuInfo : public SimpleRefCount<SatTimuInfo>
  {
  public:
    SatTimuInfo (uint32_t beamId, Address address);

    uint32_t GetBeamId () const;

    Address GetGwAddress () const;

  private:
    uint32_t m_beamId;
    Address m_gwAddress;
  };

  Ptr<SatTimuInfo> m_timuInfo;

  Mac48Address m_gwAddress;

  typedef enum
  {
    NO_HANDOVER,
    HANDOVER_RECOMMENDATION_SENT,
    WAITING_FOR_TBTP,
  } HandoverState_t;

  HandoverState_t m_handoverState;

  uint32_t m_handoverMessagesCount;
  uint32_t m_maxHandoverMessagesSent;

  uint32_t m_firstTransmittableSuperframeId;

  /**
   * The physical layer handover callback
   */
  SatUtMac::HandoverCallback m_handoverCallback;

  /**
   * Gateway address update callback
   */
  SatUtMac::GatewayUpdateCallback m_gatewayUpdateCallback;

  /**
   * Callback to update routing and ARP tables after a beam handover
   */
  SatUtMac::RoutingUpdateCallback m_routingUpdateCallback;

  /**
   * Beam checker and handover recommendation sending callback
   */
  SatUtMac::BeamCheckerCallback m_beamCheckerCallback;

  /**
   * Beam checker and handover recommendation sending callback
   */
  SatUtMac::AskedBeamCallback m_askedBeamCallback;

  /**
   * Tx checking callback
   */
  SatUtMac::TxCheckCallback m_txCheckCallback;

  /**
   * Tx checking callback
   */
  SatUtMac::SliceSubscriptionCallback m_sliceSubscriptionCallback;

  /**
   * Callback for sending a logon message
   */
  SatUtMac::SendLogonCallback m_sendLogonCallback;

  /**
   * Callback for sending a logon message
   */
  SatUtMac::UpdateGwAddressCallback m_updateGwAddressCallback;

  /**
   * Callback to get the SatBeamScheduler linked to a beam ID
   */
  SatUtMac::BeamScheculerCallback m_beamScheculerCallback;
};

} // namespace ns3

#endif /* SATELLITE_UT_MAC_H */
