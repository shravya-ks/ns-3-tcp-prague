/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Trinity College Dublin
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
 * Authors: Rohit P. Tahiliani <rohit.tahil@gmail.com>
 *
 */

#ifndef PI_SQUARE_QUEUE_DISC_H
#define PI_SQUARE_QUEUE_DISC_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class TraceContainer;
class UniformRandomVariable;

/**
 * \ingroup traffic-control
 *
 * \brief Implements PI Square queue discipline
 */
class PiSquareQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief PiSquareQueueDisc Constructor
   */
  PiSquareQueueDisc ();

  /**
   * \brief PiSquareQueueDisc Destructor
   */
  virtual ~PiSquareQueueDisc ();

  /**
   * \brief Stats
   */
  typedef struct
  {
    uint32_t unforcedDrop;      //!< Early probability drops: proactive
    uint32_t unforcedMark;      //!< Early probability marks: proactive
    uint32_t forcedDrop;        //!< Drops due to queue limit: reactive
  } Stats;

  /**
   * \brief Enumeration of the modes supported in the class.
   *
   */
  enum QueueDiscMode
  {
    QUEUE_DISC_MODE_PACKETS,     /**< Use number of packets for maximum queue disc size */
    QUEUE_DISC_MODE_BYTES,       /**< Use number of bytes for maximum queue disc size */
  };

  /**
   * \brief Set the operating mode of this queue.
   *
   * \param mode The operating mode of this queue.
   */
  void SetMode (QueueDiscMode mode);

  /**
   * \brief Get the encapsulation mode of this queue.
   *
   * \returns The encapsulation mode of this queue.
   */
  QueueDiscMode GetMode (void);

  /**
   * \brief Get the current value of the queue in bytes or packets.
   *
   * \returns The queue size in bytes or packets.
   */
  uint32_t GetQueueSize (void);

  /**
   * \brief Set the limit of the queue in bytes or packets.
   *
   * \param lim The limit in bytes or packets.
   */
  void SetQueueLimit (uint32_t lim);

  /**
   * \brief Get queue delay
   */
  Time GetQueueDelay (void);

  /**
   * \brief Get PI Square statistics after running.
   *
   * \returns The drop statistics.
   */
  Stats GetStats ();

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);
 
  /**
   * \brief Set the value of m_useDualQ and enable ECN functionality of the router if useDualQ is true.
   *
   * \param useDualQ The value of UseDualQ.
   */
  void SetDualQ (bool useDualQ);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void) const;
  virtual bool CheckConfig (void);

  /**
   * \brief Initialize the queue parameters.
   */
  virtual void InitializeParams (void);

  /**
   * \brief Check if a packet needs to be dropped due to probability drop
   * \param item queue item
   * \param qSize queue size
   * \returns 0 for no drop, 1 for drop
   */
  bool DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize);

  /**
   * \brief Periodically calculate the drop probability
   */
  void CalculateP ();

  Stats m_stats;                                //!< PI Square statistics

  // ** Variables supplied by user
  QueueDiscMode m_mode;                         //!< Mode (bytes or packets)
  uint32_t m_queueLimit;                        //!< Queue limit in bytes / packets
  Time m_sUpdate;                               //!< Start time of the update timer
  Time m_tUpdate;                               //!< Time period after which CalculateP () is called
  Time m_qDelayRef;                             //!< Desired queue delay
  uint32_t m_meanPktSize;                       //!< Average packet size in bytes
  double m_a;                                   //!< Parameter to PI Square controller
  double m_b;                                   //!< Parameter to PI Square controller
  uint32_t m_dqThreshold;                       //!< Minimum queue size in bytes before dequeue rate is measured
  bool m_useDualQ;                              //!< True if DualQ Framework is used
  bool m_useEcn;                                //!< True if ECN is used (packets are marked instead of being dropped)

  // ** Variables maintained by PI Square
  double m_dropProb;                            //!< Variable used in calculation of drop probability
  Time m_qDelayOld;                             //!< Old value of queue delay
  Time m_qDelay;                                //!< Current value of queue delay
  bool m_inMeasurement;                         //!< Indicates whether we are in a measurement cycle
  double m_avgDqRate;                           //!< Time averaged dequeue rate
  double m_dqStart;                             //!< Start timestamp of current measurement cycle
  uint32_t m_dqCount;                           //!< Number of bytes departed since current measurement cycle starts
  EventId m_rtrsEvent;                          //!< Event used to decide the decision of interval of drop probability calculation
  Ptr<UniformRandomVariable> m_uv;              //!< Rng stream
};

};   // namespace ns3

#endif
