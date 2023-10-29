#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (1);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();

  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());
  phy.Set ("TxPowerStart", DoubleValue (15)); // Station's transmission power
  phy.Set ("TxPowerEnd", DoubleValue (15));
  phy.Set ("RxSensitivity", DoubleValue (-92));
  phy.Set ("CcaMode1Threshold", DoubleValue (-62)); // CCA ED threshold
  phy.Set ("ChannelSettings", StringValue("{0, 20, BAND_5GHZ, 0}"));
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("HeMcs5"),
                                "ControlMode", StringValue ("HeMcs5"));

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns3-80211ax");

  // Setup AP
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  WifiMacHelper apMac = mac;
  NetDeviceContainer apDevice;
  apDevice = wifi.Install (phy, apMac, wifiApNode);

  // Setup STA
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  WifiMacHelper staMac = mac;
  NetDeviceContainer staDevice;
  staDevice = wifi.Install (phy, staMac, wifiStaNodes);

  // Mobility
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);
  mobility.Install (wifiApNode);

  // Positioning APs and STAs
  // Assuming the number of BSS is 2 as an example
  for (uint32_t i = 0; i < wifiApNode.GetN (); ++i)
  {
    Ptr<Node> node = wifiApNode.Get (i);
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
    mobility->SetPosition (Vector (i * 20, 0, 0)); // Example: APs are 20 meters apart
  }
  
  // Place the station 2 meters from the AP
  for (uint32_t i = 0; i < wifiStaNodes.GetN (); ++i)
  {
    Ptr<Node> node = wifiStaNodes.Get (i);
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
    mobility->SetPosition (Vector (2, 2, 0)); // 2 meters in x and y direction
  }

  // Internet stack
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  // IP address
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;

  staNodeInterface = address.Assign (staDevice);
  apNodeInterface = address.Assign (apDevice);

  // TODO: Add application, traffic generation and further configuration based on requirements.

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
