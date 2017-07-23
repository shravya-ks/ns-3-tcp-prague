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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"

#include <iostream>
#include <iomanip>
#include <map>

using namespace ns3;

int main (int argc, char *argv[])
{
  uint32_t    nLeaf = 10;
  uint32_t    maxPackets = 100;
  bool        modeBytes  = false;
  uint32_t    queueDiscLimitPackets = 1000;
  uint32_t    pktSize = 512;
  std::string appDataRate = "10Mbps";
  std::string queueDiscType = "PI2";
  uint16_t port = 5001;
  std::string bottleNeckLinkBw = "1Mbps";
  std::string bottleNeckLinkDelay = "50ms";

  CommandLine cmd;
  cmd.AddValue ("nLeaf",     "Number of left and right side leaf nodes", nLeaf);
  cmd.AddValue ("maxPackets","Max Packets allowed in the device queue", maxPackets);
  cmd.AddValue ("queueDiscLimitPackets","Max Packets allowed in the queue disc", queueDiscLimitPackets);
  cmd.AddValue ("queueDiscType", "Set Queue disc type to PIE or PI2", queueDiscType);
  cmd.AddValue ("appPktSize", "Set OnOff App Packet Size", pktSize);
  cmd.AddValue ("appDataRate", "Set OnOff App DataRate", appDataRate);
  cmd.AddValue ("modeBytes", "Set Queue disc mode to Packets <false> or bytes <true>", modeBytes);

  cmd.Parse (argc,argv);

  if ((queueDiscType != "PIE") && (queueDiscType != "PI2"))
    {
      std::cout << "Invalid queue disc type: Use --queueDiscType=PIE or --queueDiscType=PI2" << std::endl;
      exit (1);
    }

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (pktSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (appDataRate));

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxPackets", UintegerValue (1000));

  if (!modeBytes)
    {
      Config::SetDefault ("ns3::PieQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_PACKETS"));
      Config::SetDefault ("ns3::PieQueueDisc::QueueLimit", UintegerValue (queueDiscLimitPackets));
      Config::SetDefault ("ns3::PiSquareQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_PACKETS"));
      Config::SetDefault ("ns3::PiSquareQueueDisc::QueueLimit", UintegerValue (queueDiscLimitPackets));
    }
  else
    {
      Config::SetDefault ("ns3::PieQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_BYTES"));
      Config::SetDefault ("ns3::PieQueueDisc::QueueLimit", UintegerValue (queueDiscLimitPackets * pktSize));
      Config::SetDefault ("ns3::PiSquareQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_BYTES"));
      Config::SetDefault ("ns3::PiSquareQueueDisc::QueueLimit", UintegerValue (queueDiscLimitPackets * pktSize));
    }

  if (queueDiscType == "PIE")
    {
      Config::SetDefault ("ns3::PieQueueDisc::MeanPktSize", UintegerValue (pktSize));
    }
  else
    {
      Config::SetDefault ("ns3::PiSquareQueueDisc::MeanPktSize", UintegerValue (pktSize));
    }

  // Create the point-to-point link helpers
  PointToPointHelper bottleNeckLink;
  bottleNeckLink.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckLinkBw));
  bottleNeckLink.SetChannelAttribute ("Delay", StringValue (bottleNeckLinkDelay));

  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("10Mbps"));
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("1ms"));

  PointToPointDumbbellHelper d (nLeaf, pointToPointLeaf,
                                nLeaf, pointToPointLeaf,
                                bottleNeckLink);

  // Install Stack
  InternetStackHelper stack;
  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {
      stack.Install (d.GetLeft (i));
    }
  for (uint32_t i = 0; i < d.RightCount (); ++i)
    {
      stack.Install (d.GetRight (i));
    }

  stack.Install (d.GetLeft ());
  stack.Install (d.GetRight ());
  TrafficControlHelper tchBottleneck;
  QueueDiscContainer queueDiscs;
  if (queueDiscType == "PIE")
    {
      tchBottleneck.SetRootQueueDisc ("ns3::PieQueueDisc");
    }
  else
    {
      tchBottleneck.SetRootQueueDisc ("ns3::PiSquareQueueDisc");
    }
  tchBottleneck.Install (d.GetLeft ()->GetDevice (0));
  queueDiscs = tchBottleneck.Install (d.GetRight ()->GetDevice (0));

  // Assign IP Addresses
  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

  // Install on/off app on all right side nodes
  OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApps;
  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {
      sinkApps.Add (packetSinkHelper.Install (d.GetLeft (i)));
    }
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (18.0));

  ApplicationContainer clientApps;
  for (uint32_t i = 0; i < d.RightCount (); ++i)
    {
      // Create an on/off app sending packets to the left side
      AddressValue remoteAddress (InetSocketAddress (d.GetLeftIpv4Address (i), port));
      clientHelper.SetAttribute ("Remote", remoteAddress);
      clientApps.Add (clientHelper.Install (d.GetRight (i)));
    }
  clientApps.Start (Seconds (1.0)); // Start 1 second after sink
  clientApps.Stop (Seconds (15.0)); // Stop before the sink

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (18.0));

  std::cout << "Running the simulation" << std::endl;
  Simulator::Run ();

  std::cout << "*** Stats from the bottleneck queue disc ***" << std::endl;

  if (queueDiscType == "PIE")
    {
      PieQueueDisc::Stats st = StaticCast<PieQueueDisc> (queueDiscs.Get (0))->GetStats ();
      if (st.unforcedDrop == 0 || st.forcedDrop != 0)
        {
          std::cout << "There should be some unforced drops, but no forced drops" << std::endl;
          exit (1);
        }

      std::cout << "\t " << st.unforcedDrop << " drops due to prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due to hard mark" << std::endl;
    }
  else
    {
      PiSquareQueueDisc::Stats st = StaticCast<PiSquareQueueDisc> (queueDiscs.Get (0))->GetStats ();
      if (st.unforcedDrop == 0 || st.forcedDrop != 0)
        {
          std::cout << "There should be some unforced drops, but no forced drops" << std::endl;
          exit (1);
        }
      std::cout << "\t " << st.unforcedDrop << " drops due to prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due to hard mark" << std::endl;
    }

  std::cout << "Destroying the simulation" << std::endl;
  Simulator::Destroy ();
  return 0;
}
