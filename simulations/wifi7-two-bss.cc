#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>

using namespace ns3;
// =====================================================
// OBSS/PD RESET EVENT COUNTERS
// =====================================================

uint64_t obssResetBss1 = 0;
uint64_t obssResetBss2 = 0;

void
ObssPdResetCallback(uint8_t bssColor,
                    dBm_u rssiDbm,
                    bool powerRestricted,
                    dBm_u txPowerMaxDbmSiso,
                    dBm_u txPowerMaxDbmMimo)
{
    if (bssColor == 1)
    {
        obssResetBss1++;
    }
    else if (bssColor == 2)
    {
        obssResetBss2++;
    }
}
int main(int argc, char* argv[])
{
    double obssPd = -72.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("obssPd", "OBSS/PD threshold in dBm", obssPd);
    cmd.Parse(argc, argv);
    // =====================================================
    // 1. CREATE NODES
    // =====================================================

    NodeContainer apNodes;
    apNodes.Create(2);

    NodeContainer staNodes1;
    staNodes1.Create(3);

    NodeContainer staNodes2;
    staNodes2.Create(3);

    // =====================================================
    // 2. CONFIGURE WI-FI 7 AND OBSS/PD
    // =====================================================

    WifiHelper wifi;

    wifi.SetStandard(WIFI_STANDARD_80211be);

        wifi.SetObssPdAlgorithm(
        "ns3::ConstantObssPdAlgorithm",
        "ObssPdLevel", DoubleValue(obssPd));

    WifiMacHelper mac;

    YansWifiChannelHelper channel =
        YansWifiChannelHelper::Default();

    YansWifiPhyHelper phy;

    // All devices share the same wireless channel
    phy.SetChannel(channel.Create());

    phy.Set(
        "ChannelSettings",
        StringValue("{36, 20, BAND_5GHZ, 0}"));

    // =====================================================
    // 3. CONFIGURE SSIDs
    // =====================================================

    Ssid ssid1 = Ssid("BSS-1");
    Ssid ssid2 = Ssid("BSS-2");

    // =====================================================
    // 4. INSTALL STA DEVICES
    // =====================================================

    mac.SetType(
        "ns3::StaWifiMac",
        "Ssid", SsidValue(ssid1));

    NetDeviceContainer staDevices1 =
        wifi.Install(phy, mac, staNodes1);

    mac.SetType(
        "ns3::StaWifiMac",
        "Ssid", SsidValue(ssid2));

    NetDeviceContainer staDevices2 =
        wifi.Install(phy, mac, staNodes2);

    // =====================================================
    // 5. INSTALL AP DEVICES
    // =====================================================

    mac.SetType(
        "ns3::ApWifiMac",
        "Ssid", SsidValue(ssid1));

    NetDeviceContainer apDevices1 =
        wifi.Install(phy, mac, apNodes.Get(0));

    mac.SetType(
        "ns3::ApWifiMac",
        "Ssid", SsidValue(ssid2));

    NetDeviceContainer apDevices2 =
        wifi.Install(phy, mac, apNodes.Get(1));

    // =====================================================
    // 6. CONFIGURE BSS COLORING
    // =====================================================

    auto SetBssColor = [](NetDeviceContainer devices,
                          uint8_t color)
    {
        for (uint32_t i = 0; i < devices.GetN(); i++)
        {
            Ptr<WifiNetDevice> device =
                DynamicCast<WifiNetDevice>(
                    devices.Get(i));

            NS_ABORT_MSG_IF(
                !device,
                "Wi-Fi device not found");

            Ptr<HeConfiguration> heConfig =
                device->GetHeConfiguration();

            NS_ABORT_MSG_IF(
                !heConfig,
                "HE configuration not found");

            heConfig->m_bssColor = color;
        }
    };

    // BSS 1: Color = 1
    SetBssColor(apDevices1, 1);
    SetBssColor(staDevices1, 1);

    // BSS 2: Color = 2
    SetBssColor(apDevices2, 2);
    SetBssColor(staDevices2, 2);

// Verify BSS Color configuration
auto CheckBssColor = [](NetDeviceContainer devices,
                        uint8_t expectedColor)
{
    for (uint32_t i = 0; i < devices.GetN(); i++)
    {
        Ptr<WifiNetDevice> device =
            DynamicCast<WifiNetDevice>(devices.Get(i));

        Ptr<HeConfiguration> heConfig =
            device->GetHeConfiguration();

        NS_ABORT_MSG_IF(
            !heConfig,
            "HE configuration not found");

        uint8_t actualColor = heConfig->m_bssColor;

        NS_ABORT_MSG_IF(
            actualColor != expectedColor,
            "Incorrect BSS Color");

        std::cout
            << "Node " << device->GetNode()->GetId()
            << " | BSS Color: "
            << static_cast<uint32_t>(actualColor)
            << std::endl;
    }
};

CheckBssColor(apDevices1, 1);
CheckBssColor(staDevices1, 1);

CheckBssColor(apDevices2, 2);
CheckBssColor(staDevices2, 2);

    std::cout << "BSS 1 Color: 1" << std::endl;
    std::cout << "BSS 2 Color: 2" << std::endl;

// =====================================================
// CONNECT OBSS/PD RESET TRACE
// =====================================================

auto ConnectObssPdTrace = [](NetDeviceContainer devices)
{
    for (uint32_t i = 0; i < devices.GetN(); i++)
    {
        Ptr<WifiNetDevice> device =
            DynamicCast<WifiNetDevice>(devices.Get(i));

        NS_ABORT_MSG_IF(!device, "Wi-Fi device not found");

Ptr<WifiPhy> wifiPhy = device->GetPhy();

auto hePhy =
    std::dynamic_pointer_cast<HePhy>(
        wifiPhy->GetPhyEntity(WIFI_MOD_CLASS_HE));

NS_ABORT_MSG_IF(!hePhy, "HE PHY not found");

Ptr<ObssPdAlgorithm> algorithm =
    hePhy->GetObssPdAlgorithm();

        NS_ABORT_MSG_IF(!algorithm, "OBSS/PD algorithm not found");

        algorithm->TraceConnectWithoutContext(
            "Reset",
            MakeCallback(&ObssPdResetCallback));
    }
};

ConnectObssPdTrace(apDevices1);
ConnectObssPdTrace(staDevices1);

ConnectObssPdTrace(apDevices2);
ConnectObssPdTrace(staDevices2);
    // =====================================================
    // 7. VERIFY SHARED WIRELESS CHANNEL
    // =====================================================

    Ptr<WifiNetDevice> ap1 =
        DynamicCast<WifiNetDevice>(
            apDevices1.Get(0));

    Ptr<WifiNetDevice> ap2 =
        DynamicCast<WifiNetDevice>(
            apDevices2.Get(0));

    Ptr<YansWifiPhy> phy1 =
        DynamicCast<YansWifiPhy>(
            ap1->GetPhy(0));

    Ptr<YansWifiPhy> phy2 =
        DynamicCast<YansWifiPhy>(
            ap2->GetPhy(0));

    NS_ABORT_MSG_IF(
        !phy1 || !phy2,
        "AP PHY is not YansWifiPhy");

    bool sharedChannel =
        (phy1->GetChannel() == phy2->GetChannel());

    std::cout
        << "AP1 and AP2 share channel: "
        << (sharedChannel ? "YES" : "NO")
        << std::endl;

    NS_ABORT_MSG_IF(
        !sharedChannel,
        "APs are not sharing the same channel");

    // =====================================================
    // 8. CONFIGURE MOBILITY
    // =====================================================

    NodeContainer allNodes;

    allNodes.Add(apNodes);
    allNodes.Add(staNodes1);
    allNodes.Add(staNodes2);

    MobilityHelper mobility;

    Ptr<ListPositionAllocator> positions =
        CreateObject<ListPositionAllocator>();

    // AP positions
    positions->Add(Vector(0, 0, 0));
    positions->Add(Vector(35, 0, 0));

    // BSS 1 stations
    positions->Add(Vector(2, 0, 0));
    positions->Add(Vector(4, 3, 0));
    positions->Add(Vector(7, -2, 0));

    // BSS 2 stations
    positions->Add(Vector(33, 0, 0));
    positions->Add(Vector(38, 3, 0));
    positions->Add(Vector(42, -2, 0));

    mobility.SetPositionAllocator(positions);

    mobility.SetMobilityModel(
        "ns3::ConstantPositionMobilityModel");

    mobility.Install(allNodes);

    // =====================================================
    // 9. INSTALL INTERNET STACK
    // =====================================================

    InternetStackHelper internet;

    internet.Install(allNodes);
// =====================================================
// 10. ASSIGN IP ADDRESSES FOR TWO BSS
// =====================================================

Ipv4AddressHelper address;

// BSS 1: 10.1.1.0/24
NetDeviceContainer bss1Devices;
bss1Devices.Add(apDevices1);
bss1Devices.Add(staDevices1);

address.SetBase("10.1.1.0", "255.255.255.0");

Ipv4InterfaceContainer interfaces1 =
    address.Assign(bss1Devices);

// BSS 2: 10.1.2.0/24
NetDeviceContainer bss2Devices;
bss2Devices.Add(apDevices2);
bss2Devices.Add(staDevices2);

address.SetBase("10.1.2.0", "255.255.255.0");

Ipv4InterfaceContainer interfaces2 =
    address.Assign(bss2Devices);

    // =====================================================
    // 11. CONFIGURE UDP DOWNLINK TRAFFIC
    // =====================================================

    ApplicationContainer serverApps;
    ApplicationContainer clientApps;

    uint16_t port = 9000;

    for (uint32_t i = 0; i < 6; i++)
    {

        UdpServerHelper server(port + i);

        Ptr<Node> staNode =
            (i < 3)
                ? staNodes1.Get(i)
                : staNodes2.Get(i - 3);

        serverApps.Add(
            server.Install(staNode));

Ipv4Address destinationAddress =
    (i < 3)
        ? interfaces1.GetAddress(i + 1)
        : interfaces2.GetAddress(i - 2);

UdpClientHelper client(
    destinationAddress,
    port + i);

        client.SetAttribute(
            "MaxPackets",
            UintegerValue(1000000));

        // One packet every 1 ms
        client.SetAttribute(
            "Interval",
            TimeValue(MilliSeconds(1)));

        // UDP payload size
        client.SetAttribute(
            "PacketSize",
            UintegerValue(1024));

        Ptr<Node> apNode =
            (i < 3)
                ? apNodes.Get(0)
                : apNodes.Get(1);

        clientApps.Add(
            client.Install(apNode));
    }

    // =====================================================
    // 11. APPLICATION START / STOP TIMES
    // =====================================================

    serverApps.Start(Seconds(0.5));
    serverApps.Stop(Seconds(11.0));

    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(10.0));

     // =====================================================
// 12. INSTALL FLOW MONITOR AND RUN SIMULATION
// =====================================================

FlowMonitorHelper flowmonHelper;

Ptr<FlowMonitor> monitor =
    flowmonHelper.InstallAll();

Simulator::Stop(Seconds(11.0));

Simulator::Run();

monitor->CheckForLostPackets();
monitor->SerializeToXmlFile(
    "wifi7-flowmon.xml",
    true,
    true);
    // =====================================================
    // 13. PRINT SIMULATION RESULTS
    // =====================================================

    std::cout
        << "\n===== WIFI 7 SIMULATION RESULTS =====\n";

    uint64_t totalReceived = 0;

    for (uint32_t i = 0; i < 6; i++)
    {
        Ptr<UdpServer> server =
            DynamicCast<UdpServer>(
                serverApps.Get(i));

        uint64_t received =
            server->GetReceived();

        totalReceived += received;

        double throughput =
            received * 1024.0 * 8.0 / 9.0 / 1e6;

        std::cout
            << "STA " << i + 1
            << " | Received: " << received
            << " packets"
            << " | Throughput: "
            << throughput
            << " Mbps"
            << std::endl;
    }

    double totalThroughput =
        totalReceived * 1024.0 * 8.0 / 9.0 / 1e6;

    std::cout
        << "\nTotal received packets: "
        << totalReceived
        << std::endl;

    std::cout
        << "Total throughput: "
        << totalThroughput
        << " Mbps"
        << std::endl;
std::cout << "\n===== OBSS/PD STATISTICS =====\n";

std::cout << "BSS 1 reset events: "
          << obssResetBss1 << "\n";

std::cout << "BSS 2 reset events: "
          << obssResetBss2 << "\n";

std::cout << "Total OBSS/PD reset events: "
          << obssResetBss1 + obssResetBss2
          << "\n";
    // =====================================================
    // 14. CLEAN UP
    // =====================================================

    Simulator::Destroy();

    return 0;
}
