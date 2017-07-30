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

#ifndef DUAL_QUEUE_PI_SQUARE_QUEUE_DISC_H
#define DUAL_QUEUE_PI_SQUARE_QUEUE_DISC_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/timer.h"
#include "ns3/string.h"
#include "ns3/event-id.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/trace-source-accessor.h"

namespace ns3 {

class TraceContainer;
class UniformRandomVariable;

/**
 * \ingroup traffic-control
 *
 * \brief Implements PI Square queue discipline
 */
class DualQueuePiSquareQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief DualQueuePiSquareQueueDisc Constructor
   */
  DualQueuePiSquareQueueDisc ();

  /**
   * \brief DualQueuePiSquareQueueDisc Destructor
   */
  virtual ~DualQueuePiSquareQueueDisc ();

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
   * \brief Periodically calculate the drop probability
   */
  void CalculateP ();

  Stats m_stats;                                //!< PI Square statistics

  // ** Variables supplied by user
  QueueDiscMode m_mode;                         //!< Mode (bytes or packets)
  Time m_classicQueueDelayRef;                      //!< PI AQM Classic queue delay target
  Time m_sUpdate;                               //!< Start time of the update timer
  Time m_tUpdate;                               //!< Time period after which CalculateP () is called
  uint32_t m_meanPktSize;                       //!< Average packet size in bytes
  double m_alpha;                                   //!< Parameter to PI Square controller
  double m_beta;                                   //!< Parameter to PI Square controller
  //bool m_useEcn;                                //!< True if ECN is used (packets are marked instead of being dropped)
  Time m_l4sThreshold;                          //!< L4S marking threshold in time
  uint32_t m_k;                                  //!< Coupling factor
  double m_maxLinkRate;                        //!< Maximum Link Rate
  double m_maxClassicProb;                     //!< Max Classic drop/mark prob
  uint32_t m_queueLimit;                        //!< Queue limit in bytes / packets

  // ** Variables maintained by PI Square
  Time m_classicQueueTime;
  Time m_l4sQueueTime;
  double m_maxL4SProb;                           // Max L4S marking prob
  Time m_tShift;                                 //!< Scheduler time bias
  uint32_t m_minL4SLength;                       //!< Min L4S marking threshold in bytes
  double m_dropProb;
  double m_classicDropProb;                     //!< Variable used in calculation of drop probability of Classic traffic
  double m_l4sDropProb;                         //!< Variable used in calculation of drop probability of L4S traffic
  double m_alphaU;
  double m_betaU;
  Time m_qDelayOld;                             //!< Old value of queue delay
  Time m_qDelay;                                //!< Current value of queue delay
  EventId m_rtrsEvent;                          //!< Event used to decide the decision of interval of drop probability calculation
  Ptr<UniformRandomVariable> m_uv;              //!< Rng stream
};

};   // namespace ns3

#endif
