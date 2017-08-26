#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DctcpExample");

std::stringstream filePlotQueue1;
std::stringstream filePlotQueue2;

void
PrintPayload ( Ptr< const Packet > p)
{
  std::ofstream fPlotQueue ("throughput.dat", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << p->GetSize () << std::endl;
  fPlotQueue.close ();
}

void
CheckQueueSize (Ptr<QueueDisc> queue, std::string filePlotQueue)
{
  uint32_t qSize = StaticCast<RedQueueDisc> (queue)->GetQueueSize ();

  // check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.01), &CheckQueueSize, queue, filePlotQueue);

  std::ofstream fPlotQueue (filePlotQueue.c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();
}

int main (int argc, char *argv[])
{

  LogComponentEnable ("DctcpExample", LOG_LEVEL_INFO);

  std::string pathOut = ".";
  bool writeForPlot = false;
  bool writePcap = false;

  CommandLine cmd;
  cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap", pathOut);
  cmd.AddValue ("writeForPlot", "<0/1> to write results for plot (gnuplot)", writeForPlot);
  cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
  cmd.Parse (argc, argv);

  double global_start_time = 0.0;
  double global_stop_time = 11;
  double sink_start_time = global_start_time;
  double sink_stop_time = global_stop_time + 3.0;
  double client_start_time = sink_start_time + 0.2;
  double client_stop_time = global_stop_time - 2.0;

  NodeContainer S1, S2, S3, R1, R2, T1, T2;
  T1.Create (2);
  T2.Create (1);
  T2.Add (T1.Get (1));
  R1.Create (1);
  R1.Add (T2.Get (0));
  S1.Create (10);
  S1.Add (T1.Get (0));
  S2.Create (20);
  S2.Add (T1.Get (0));
  S3.Create (10);
  S3.Add (T2.Get (0));
  R2.Create (20);
  R2.Add (T2.Get (0));

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpDctcp"));

  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

  uint32_t meanPktSize = 500;

  Config::SetDefault ("ns3::RedQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_PACKETS"));
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (meanPktSize));
  Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (1));
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (85));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (85));
  Config::SetDefault ("ns3::RedQueueDisc::QueueLimit", UintegerValue (500));
  Config::SetDefault ("ns3::RedQueueDisc::UseEcn", BooleanValue (true));

  PointToPointHelper pointToPointSR;
  pointToPointSR.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
  pointToPointSR.SetChannelAttribute ("Delay", StringValue ("0.01ms"));

  PointToPointHelper pointToPointT;
  pointToPointT.SetDeviceAttribute ("DataRate", StringValue ("10000Mbps"));
  pointToPointT.SetChannelAttribute ("Delay", StringValue ("0.01ms"));

  TrafficControlHelper tchRed;
  tchRed.SetRootQueueDisc ("ns3::RedQueueDisc", "LinkBandwidth", StringValue ("10000Mbps"),
                           "LinkDelay", StringValue ("0.01ms"));

  NetDeviceContainer S1dev, S2dev, S3dev, R1dev, R2dev, T1dev, T2dev;
  R1dev = pointToPointSR.Install (R1);
  T1dev = pointToPointT.Install (T1);
  T2dev = pointToPointT.Install (T2);

  T1dev.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback (&PrintPayload));

  for (uint32_t i = 0; i < 10; i++)
    {
      S1dev.Add (pointToPointSR.Install (S1.Get (i), T1.Get (0)));
      S3dev.Add (pointToPointSR.Install (S3.Get (i), T2.Get (0)));
      S2dev.Add (pointToPointSR.Install (S2.Get (i * 2), T1.Get (0)));
      S2dev.Add (pointToPointSR.Install (S2.Get (2 * i + 1), T1.Get (0)));
      R2dev.Add (pointToPointSR.Install (R2.Get (i * 2), T2.Get (0)));
      R2dev.Add (pointToPointSR.Install (R2.Get (2 * i + 1), T2.Get (0)));
    }

  InternetStackHelper stack;
  stack.Install (S2);
  stack.Install (R2);
  stack.Install (T1.Get (1));
  stack.Install (R1.Get (0));

  QueueDiscContainer queueDiscs1 = tchRed.Install (T1dev);
  QueueDiscContainer queueDiscs2 = tchRed.Install (T2dev);

  for (uint32_t i = 0; i < 10; i++)
    {
      stack.Install (S1.Get (i));
      stack.Install (S3.Get (i));
    }

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer S1Int, S2Int, S3Int, R1Int, R2Int, T1Int, T2Int;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  S1Int = address.Assign (S1dev);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  S2Int = address.Assign (S2dev);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  S3Int = address.Assign (S3dev);

  address.SetBase ("10.2.1.0", "255.255.255.0");
  R1Int = address.Assign (R1dev);

  address.SetBase ("10.2.2.0", "255.255.255.0");
  R2Int = address.Assign (R2dev);

  address.SetBase ("10.3.1.0", "255.255.255.0");
  T1Int = address.Assign (T1dev);

  address.SetBase ("10.3.2.0", "255.255.255.0");
  T2Int = address.Assign (T2dev);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (R1.Get (0));
  sinkApp.Start (Seconds (sink_start_time));
  sinkApp.Stop (Seconds (sink_stop_time));

  OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
  clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper1.SetAttribute ("DataRate", DataRateValue (DataRate ("1000Mb/s")));
  clientHelper1.SetAttribute ("PacketSize", UintegerValue (1000));

  ApplicationContainer clientApps1;
  AddressValue remoteAddress (InetSocketAddress (R1Int.GetAddress (0), port));
  clientHelper1.SetAttribute ("Remote", remoteAddress);
  for (uint32_t i = 0; i < 10; i++)
    {
      clientApps1.Add (clientHelper1.Install (S1.Get (i)));
      clientApps1.Add (clientHelper1.Install (S3.Get (i)));
    }
  clientApps1.Start (Seconds (client_start_time));
  clientApps1.Stop (Seconds (client_stop_time));

  for (uint32_t i = 0; i < 20; i++)
    {
      Address sinkLocalAddress2 (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
      ApplicationContainer sinkApp2 = sinkHelper.Install (R2.Get (i));
      sinkApp2.Start (Seconds (sink_start_time));
      sinkApp2.Stop (Seconds (sink_stop_time));

      OnOffHelper clientHelper2 ("ns3::TcpSocketFactory", Address ());
      clientHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      clientHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      clientHelper2.SetAttribute ("DataRate", DataRateValue (DataRate ("1000Mb/s")));
      clientHelper2.SetAttribute ("PacketSize", UintegerValue (1000));

      ApplicationContainer clientApps2;
      AddressValue remoteAddress2 (InetSocketAddress (R2Int.GetAddress (i), port));
      clientHelper2.SetAttribute ("Remote", remoteAddress2);
      clientApps2.Add (clientHelper2.Install (S2.Get (i)));
      clientApps2.Start (Seconds (client_start_time));
      clientApps2.Stop (Seconds (client_stop_time));
    }


  if (writePcap)
    {
      PointToPointHelper ptp;
      std::stringstream stmp;
      stmp << pathOut << "/dctcp-dumbbell";
      ptp.EnablePcapAll (stmp.str ().c_str ());
    }

  if (writeForPlot)
    {
      filePlotQueue1 << pathOut << "/" << "dctcp-dumbbell-queue1.plotme";
      remove (filePlotQueue1.str ().c_str ());
      Ptr<QueueDisc> queue1 = queueDiscs1.Get (0);
      Simulator::ScheduleNow (&CheckQueueSize, queue1, filePlotQueue1.str ());

      filePlotQueue2 << pathOut << "/" << "dctcp-dumbbell-queue2.plotme";
      remove (filePlotQueue2.str ().c_str ());
      Ptr<QueueDisc> queue2 = queueDiscs2.Get (0);
      Simulator::ScheduleNow (&CheckQueueSize, queue2, filePlotQueue2.str ());

    }

  Simulator::Stop (Seconds (sink_stop_time));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
