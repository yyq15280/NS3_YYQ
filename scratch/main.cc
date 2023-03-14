#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include <vector>
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include <ctime>
// #include "error-model.h"
// #include "random-walk-2d-mobility-model.h"
// #include "ns3/pointer.h"
// #include "ns3/log.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("APCC");
static Ptr<OutputStreamWrapper> rttStream;
static void RttTracer(Time oldval, Time newval)
{
    *rttStream->GetStream() << Simulator::Now().GetSeconds() << " " << newval.GetSeconds() << std::endl;
}
static void TraceRtt(string rtt_tr_file_name)
{
    AsciiTraceHelper ascii;
    rttStream = ascii.CreateFileStream(rtt_tr_file_name.c_str());
    Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeCallback(&RttTracer));
}


static ns3::GlobalValue currentqueuesize ("currentqueuesize_name", "currentqueuesize",//当前的队列长度
                                   ns3::IntegerValue(0),
                                   ns3::MakeIntegerChecker<uint32_t> ());

static ns3::GlobalValue dequeuecount ("dequeuecount", "dequeuecount",//用来给出队的数目计数
                                   ns3::IntegerValue(0),
                                   ns3::MakeIntegerChecker<uint32_t> ());           

static ns3::GlobalValue prerttglobal ("prerttglobal", "prertt",//预测的rtt是多少
                                   ns3::DoubleValue(0),
                                   ns3::MakeDoubleChecker<double> ()); 
static ns3::GlobalValue timeold ("timeold", "timeold",//
                                   ns3::DoubleValue(0),
                                   ns3::MakeDoubleChecker<double> ()); 

static ns3::GlobalValue enqueuecount ("enqueuecount", "enqueuecount",//用来给出队的数目计数
                                   ns3::IntegerValue(0),
                                   ns3::MakeIntegerChecker<uint32_t> ());    

static ns3::GlobalValue timeoldforenqueue ("timeoldforenqueue", "timeoldforenqueue",//
                                   ns3::DoubleValue(0),
                                   ns3::MakeDoubleChecker<double> ()); 

static ns3::GlobalValue datachange ("datachange", "datachange",//预测的rtt是多少
                                   ns3::DoubleValue(0),
                                   ns3::MakeDoubleChecker<double> ());   

static ns3::GlobalValue apwindow("apwindow", "A global pointer to a TcpL4Protocol object",
               PointerValue(new TcpL4Protocol),
               MakePointerChecker<TcpL4Protocol>());


ofstream qlenOutput;
void DevicePacketsInQueueTrace(Ptr<QueueDisc> root, uint32_t oldValue, uint32_t newValue)
{
    uint32_t qlen = newValue + root->GetQueueDiscClass(0)->GetQueueDisc()->GetCurrentSize().GetValue();
    Config::SetGlobal("currentqueuesize_name",IntegerValue(qlen));
    // GlobalValue::BindFailSafe ("currentqueuesize_name", IntegerValue(qlen));
    // IntegerValue value;
    // currentqueuesize.GetValue(value);
    // std::cout<<value.Get()<<endl;

    qlenOutput << Simulator::Now().GetSeconds() << " " << qlen << std::endl;

}
// static Ptr<OutputStreamWrapper> ossprertt;
// ostringstream ossprertt;
ofstream ossprertt;
void DequeueTrace(std::string context, Ptr<const WifiMacQueueItem> item) {
    IntegerValue value;
    dequeuecount.GetValue(value);//获得当前的计数值；
    IntegerValue dangqianduiliechangdu;
    double prertt;
    double b = 0.025;
    DoubleValue changedataforen_de;
    if(value.Get()==499){
        
        currentqueuesize.GetValue(dangqianduiliechangdu);
        DoubleValue zhiqianshijian;
        timeold.GetValue(zhiqianshijian);
        prertt = dangqianduiliechangdu.Get()/(500/(Simulator::Now().GetSeconds()-zhiqianshijian.Get()));
        datachange.GetValue(changedataforen_de);
        double a = changedataforen_de.Get()+prertt+b;
        Config::SetGlobal("prerttglobal",DoubleValue(a));
        GlobalValue::BindFailSafe ("dequeuecount", IntegerValue(0));
        DoubleValue b;
        prerttglobal.GetValue(b);
    // *ossprertt->GetStream() << Simulator::Now().GetSeconds() << " " <<b.Get() << std::endl;
    ossprertt << Simulator::Now().GetSeconds() << " " <<b.Get() << std::endl;
    }
    else if(value.Get()==0){
        Config::SetGlobal("timeold",DoubleValue(Simulator::Now().GetSeconds()));
        uint32_t a = value.Get();
        a=a+1;
        GlobalValue::BindFailSafe ("dequeuecount", IntegerValue(a));
    }
    else{
        uint32_t a = value.Get();
        a=a+1;
        GlobalValue::BindFailSafe ("dequeuecount", IntegerValue(a));
    }


//   std::cout<<"Inside Deque Current Time = "<<Simulator::Now() << "," << "Packet Generation Time = "<< item->GetTimeStamp() << std::endl;
//   std::cout << "Queueing Delay : " << delay << std::endl;
    // std::cout << timesecond <<endl;
   
    // std::cout<<ossprertt.<<endl;
}
// static void Tracepre(string prertt_tr_file_name)
// {
//     AsciiTraceHelper ascii;
//     ossprertt = ascii.CreateFileStream(prertt_tr_file_name.c_str());
// }

void EnqueueForRttTrace(std::string context, Ptr<const WifiMacQueueItem> item){
    IntegerValue value;
    enqueuecount.GetValue(value);
    double predictforenqueue;
    if(value.Get()==99){
        DoubleValue zhiqianshijian;
        timeold.GetValue(zhiqianshijian);
        predictforenqueue = (Simulator::Now().GetSeconds()-zhiqianshijian.Get())/100;
        Config::SetGlobal("datachange",DoubleValue(predictforenqueue));
        GlobalValue::BindFailSafe ("enqueuecount", IntegerValue(0));
    }
    else if(value.Get()==0){
        Config::SetGlobal("timeoldforenqueue",DoubleValue(Simulator::Now().GetSeconds()));
        uint32_t a = value.Get();
        a=a+1;
        GlobalValue::BindFailSafe ("enqueuecount", IntegerValue(a));
    }
    else{
        uint32_t a = value.Get();
        a=a+1;
        GlobalValue::BindFailSafe ("enqueuecount", IntegerValue(a));
    }
}

uint64_t lastSendBytes[5] = {0, 0, 0, 0, 0};
uint64_t tmp[5];
static Ptr<OutputStreamWrapper> thputStream;

void RateByPtpNetDevice(Ptr<PointToPointNetDevice> device, int no, double interval)
{
    Time time = Seconds(interval);
    tmp[no] = device->m_sendBytes - lastSendBytes[no];
    lastSendBytes[no] = device->m_sendBytes;
    double rate = tmp[no] * 8 / (1000 * 1000 * time.GetSeconds());
    *thputStream->GetStream() << Simulator::Now().GetSeconds() << '\t' << rate << "Mbps" << endl;
    Simulator::Schedule(time, &RateByPtpNetDevice, device, no, interval);

}

static void TraceThput(string thput_tr_file_name, Ptr<PointToPointNetDevice> device, int no, double interval)
{
    //结果写到哪个文件中 ，跟踪哪个网卡,跟踪remote发送端
    AsciiTraceHelper ascii;
    thputStream = ascii.CreateFileStream(thput_tr_file_name.c_str());
    RateByPtpNetDevice(device, no, interval);
}




int main(int argc, char *argv[])
{
    uint32_t cc_mode = 2;

    // Config::SetDefault("ns3::WifiMacQueue::MaxSize", QueueSizeValue(QueueSize("250p"))); // default 500p
    // Config::SetDefault ("ns3::WifiMacQueue::MaxDelay", TimeValue (MilliSeconds (1000)));
    if (cc_mode == 0)
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    }
    else if (cc_mode == 1)
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    }

    else if (cc_mode == 2)
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
    }
    else if (cc_mode == 3)
    {
        // Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
    }
    else if (cc_mode == 4)
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpBbr"));
    }
    else if (cc_mode == 5) // bbr不需要加AQM
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpBbr"));
    }

    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(10));      //更改tcp初始拥塞窗口大小
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(33554432)); //传输层用socket来控制，socket里有buffer 先发到buffer里面，默认比较小，所以设置高一些
    // Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(33554432));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(8388608));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448)); //将应用数据分段，每1448字节就添加包头,不能超过1500，ns3有tcp optition，占12字节，data字节数加上tcp，ip包头
    // Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(4));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MilliSeconds(200))); //设置为200ms和linux对齐，在main函数头部部分加设置才有效
    
    // Start create topo
    string rate_1 = "100000Mbps"; // the rate of "remote server" to core switch
    string rate_2 = "40000Mbps";  // the rate of core switch to convergence switch
    string rate_3 = "10000Mbps";  // the rate of convergence switch to access switch
    string rate_4 = "1000Mbps";   // the rate of access switch to ap
    string channel_delay = "0.15ms";
    // string channel_delay = "2ms";
    // create nodes
    NodeContainer remote_servers;
    remote_servers.Create(1);
    NodeContainer core_switches;
    core_switches.Create(1);
    NodeContainer convergence_switches;
    convergence_switches.Create(1);
    NodeContainer access_switches;
    access_switches.Create(1);

    NodeContainer ap;
    uint32_t ap_num = 1;
    ap.Create(ap_num);
    vector<NodeContainer> wifi_nodes(ap_num);
    for (uint32_t i = 0; i < ap_num; i++)
    { //每个ap下创建两个wifi节点
        wifi_nodes[i].Create(2);
    }

    // install stack
    InternetStackHelper stack;
    stack.Install(remote_servers);
    stack.Install(core_switches);
    stack.Install(convergence_switches);
    stack.Install(access_switches);
    stack.Install(ap);

    for (uint32_t i = 0; i < ap_num; i++)
    {
        stack.Install(wifi_nodes[i]);
    }

    // create wired connect
    PointToPointHelper p2p_1;
    p2p_1.SetDeviceAttribute("DataRate", StringValue(rate_1)); //设置网卡速率
    p2p_1.SetChannelAttribute("Delay", StringValue("9ms"));
    // p2p_1.SetChannelAttribute("Delay", StringValue("25ms")); //
    NetDeviceContainer p2p_1_devices;
    p2p_1_devices = p2p_1.Install(remote_servers.Get(0), core_switches.Get(0));

    PointToPointHelper p2p_2;
    p2p_2.SetDeviceAttribute("DataRate", StringValue(rate_2));
    p2p_2.SetChannelAttribute("Delay", StringValue(channel_delay));
    NetDeviceContainer p2p_2_devices;
    p2p_2_devices = p2p_2.Install(core_switches.Get(0), convergence_switches.Get(0));

    PointToPointHelper p2p_3;
    
    p2p_3.SetDeviceAttribute("DataRate", StringValue(rate_3));
    p2p_3.SetChannelAttribute("Delay", StringValue(channel_delay));
    NetDeviceContainer p2p_3_devices;
    p2p_3_devices = p2p_3.Install(convergence_switches.Get(0), access_switches.Get(0));

    PointToPointHelper p2p_4;
    p2p_4.SetDeviceAttribute("DataRate", StringValue(rate_4));
    p2p_4.SetChannelAttribute("Delay", StringValue(channel_delay));

    vector<NetDeviceContainer> access_devices(ap_num);
    for (uint32_t i = 0; i < ap_num; i++)
    {
        access_devices[i] = p2p_4.Install(access_switches.Get(0), ap.Get(i));
    }

    // create wifi connect
    vector<YansWifiChannelHelper> wifi_channel(ap_num);
    vector<YansWifiPhyHelper> wifi_phy(ap_num); //创建无线的phy层
    for (uint32_t i = 0; i < ap_num; i++)
    {
        wifi_channel[i] = YansWifiChannelHelper::Default();

        // wifi_channel[i].AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(10));
        // wifi_channel[i].AddPropagationLoss("ns3::RandomPropagationLossModel");
        wifi_phy[i].SetChannel(wifi_channel[i].Create());
        // transmission power: 27dBm
        wifi_phy[i].Set("TxPowerStart", DoubleValue(27));
        wifi_phy[i].Set("TxPowerEnd", DoubleValue(27));
        wifi_phy[i].Set("TxPowerLevels", UintegerValue(1));
    }
    //配置mac协议

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211ax_5GHZ);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", //解码编码方式固定
                                 "DataMode", StringValue("HeMcs6"),
                                 "ControlMode", StringValue("HeMcs6")); //
    // wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");

    WifiMacHelper wifi_mac;
    vector<NetDeviceContainer> wifi_nodes_devices(ap_num);
    vector<NetDeviceContainer> ap_devices(ap_num);
    for (uint32_t i = 0; i < ap_num; i++)
    { //在这里添加qos
        string ssid_name = "ap" + std::to_string(i);
        Ssid ssid = Ssid(ssid_name);
        wifi_mac.SetType("ns3::StaWifiMac",
                         "Ssid", SsidValue(ssid));
        wifi_nodes_devices[i] = wifi.Install(wifi_phy[i], wifi_mac, wifi_nodes[i]);

        wifi_mac.SetType("ns3::ApWifiMac",
                         "Ssid", SsidValue(ssid));
        ap_devices[i] = wifi.Install(wifi_phy[i], wifi_mac, ap.Get(i));
    }
    // Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::WifiMac/QosSupported", BooleanValue (false));
    // std::cout<<DynamicCast<ApWifiMac>(DynamicCast<WifiNetDevice>(ap_devices[0].Get(0))->GetMac())->GetQosSupported()<<std::endl;
    Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/ChannelNumber", UintegerValue(50)); // set channel width 160MHz
    // std::cout<<DynamicCast<WifiNetDevice>(ap_devices[0].Get(0))->GetPhy()->GetChannelWidth()<<std::endl;//频道宽度
    Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HeConfiguration/GuardInterval", TimeValue(NanoSeconds(800))); //设置间隔 间隔越小 吞吐越高

    // Ptr<WifiMac> yyqwifiMac = ap_devices.Get(0)->GetObject<WifiMac>();
    // yyqwifiMac->SetAttribute ("RxWindowSize", UintegerValue (2048));


    // mobility
    vector<MobilityHelper> mobility(ap_num);
    vector<Ptr<ListPositionAllocator>> positionAlloc(ap_num);
    for (uint32_t i = 0; i < ap_num; i++)
    {
        positionAlloc[i] = CreateObject<ListPositionAllocator>();
        positionAlloc[i]->Add(Vector(0.0, i * 50.0, 0.0));
        positionAlloc[i]->Add(Vector(-3.0, i * 50.0, 0.0));
        positionAlloc[i]->Add(Vector(3.0, i * 50.0, 0.0));
        mobility[i].SetPositionAllocator(positionAlloc[i]);
        mobility[i].SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility[i].Install(ap.Get(i));
        mobility[i].Install(wifi_nodes[i]);
    }

    // MobilityHelper mobility1;
    // mobility1.SetPositionAllocator("ns3::GridPositionAllocator",
    //                                "MinX", DoubleValue(0.0),
    //                                "MinY", DoubleValue(0.0),
    //                                "DeltaX", DoubleValue(5.0),
    //                                "DeltaY", DoubleValue(5.0),
    //                                "GridWidth", UintegerValue(1),
    //                                "LayoutType", StringValue("RowFirst"));
    // mobility1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    // mobility1.Install(ap.Get(0));

    // MobilityHelper mobility2;
    // mobility2.SetPositionAllocator("ns3::GridPositionAllocator",
    //                                "MinX", DoubleValue(0.0),
    //                                "MinY", DoubleValue(0.0),
    //                                "DeltaX", DoubleValue(5.0),
    //                                "DeltaY", DoubleValue(5.0),
    //                                "GridWidth", UintegerValue(1),
    //                                "LayoutType", StringValue("RowFirst"));
    // mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    // mobility2.Install(wifi_nodes[0].Get(0));

    // MobilityHelper mobility3;
    // mobility3.SetPositionAllocator("ns3::GridPositionAllocator",
    //                                "MinX", DoubleValue(10.0),
    //                                "MinY", DoubleValue(0.0),
    //                                "DeltaX", DoubleValue(0.0),
    //                                "DeltaY", DoubleValue(0.0),
    //                                "GridWidth", UintegerValue(1),
    //                                "LayoutType", StringValue("RowFirst"));

    // mobility3.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
    //                            "Mode", StringValug+++  e("Distance"),
    //                            "Distance", DoubleValue(1.0),
    //                            "Speed", StringValue("ns3::ConstantRandomVariable[Constant=100.0]"),
    //                            "Bounds", StringValue("-9|11|-0.5|0.5"));
    // mobility3.Install(wifi_nodes[0].Get(1));
    //  install queue disc
    //  To install a queue disc other than the default one, it is necessary to install such queue disc before an IP address is assigned to the device

    // TrafficControlHelper tchWifi;
    // tchWifi.SetRootQueueDisc ("ns3::FifoQueueDisc", "MaxSize", StringValue ("5p"));
    // vector<QueueDiscContainer> qdiscs(ap_num);
    // for(uint32_t i = 0; i < ap_num; i++) {
    //     qdiscs[i] = tchWifi.Install(ap_devices[i]);
    // }
    //入队事件的跟踪：
//  std::string enqueue_path= "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/*/$ns3::OcbWifiMac/*/Queue/Enqueue";

//  void EnqueueTrace(std::string context, Ptr<const WifiMacQueueItem> item){
//   Time entry_time = Simulator::Now();
//   std::cout<<"Packet entered queue at"<<entry_time<<std::end
//  }

    TrafficControlHelper tch; //软件队列，硬件队列是fifo
    uint16_t handle = tch.SetRootQueueDisc("ns3::MqQueueDisc");
    TrafficControlHelper::ClassIdList cls = tch.AddQueueDiscClasses(handle, 4, "ns3::QueueDiscClass"); //为根队列规则添加子队列规则
    if (cc_mode == 0)
    {
        tch.AddChildQueueDiscs(handle, cls, "ns3::FifoQueueDisc");
    }
    else if (cc_mode == 1)
    {
        tch.AddChildQueueDiscs(handle, cls, "ns3::RedQueueDisc");
    }
    else if (cc_mode == 2)
    {
        tch.AddChildQueueDiscs(handle, cls, "ns3::FifoQueueDisc");
    }
    else if (cc_mode == 3)
    {
        tch.AddChildQueueDiscs(handle, cls, "ns3::RedQueueDisc");
    }
    else if (cc_mode == 4)
    {
        tch.AddChildQueueDiscs(handle, cls, "ns3::FifoQueueDisc");
    }
    else if (cc_mode == 5) // bbr不需要加red
    {
        tch.AddChildQueueDiscs(handle, cls, "ns3::RedQueueDisc");
    }

    tch.Install(ap_devices[0]);

    Ptr<QueueDisc> root_qdisc = ap.Get(0)->GetObject<TrafficControlLayer>()->GetRootQueueDiscOnDevice(ap_devices[0].Get(0));
    root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetMaxSize(QueueSize("7500p")); //设置队列的最大长度

    if (cc_mode == 0)
    {
    }
    else if (cc_mode == 1)
    {
        // root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetMaxSize(QueueSize("7500p"));
        root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetAttribute("MinTh", DoubleValue(50));
        root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetAttribute("MaxTh", DoubleValue(50));
    }
    else if (cc_mode == 2)
    {
        // Ptr<TrafficControlLayer> tc = ap.Get(0)->GetObject<TrafficControlLayer>();
        // tc->SetAttribute("Ccmode", UintegerValue(2));
        // tc->SetAttribute("Kmin", UintegerValue(500));
    }
    else if (cc_mode == 3)
    {
        root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetAttribute("MinTh", DoubleValue(150));
        root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetAttribute("MaxTh", DoubleValue(150));
        // Ptr<TrafficControlLayer> tc = ap.Get(0)->GetObject<TrafficControlLayer>();
        // // root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetMaxSize(QueueSize("7500p"));
        // tc->SetAttribute("Ccmode", UintegerValue(3));
        // tc->SetAttribute("Kmin", UintegerValue(300));
        // tc->SetAttribute("DeviceRate", UintegerValue(650));
    }
    else if (cc_mode == 4)
    {
        // Ptr<TrafficControlLayer> tc = ap.Get(0)->GetObject<TrafficControlLayer>();
        // // root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetMaxSize(QueueSize("7500p"));
        // tc->SetAttribute("Ccmode", UintegerValue(3));
        // tc->SetAttribute("Kmin", UintegerValue(300));
        // tc->SetAttribute("DeviceRate", UintegerValue(650));
    }
    else if (cc_mode == 5)
    {
        root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetAttribute("MinTh", DoubleValue(150));
        root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetAttribute("MaxTh", DoubleValue(150));
        // Ptr<TrafficControlLayer> tc = ap.Get(0)->GetObject<TrafficControlLayer>();
        // // root_qdisc->GetQueueDiscClass(0)->GetQueueDisc()->SetMaxSize(QueueSize("7500p"));
        // tc->SetAttribute("Ccmode", UintegerValue(3));
        // tc->SetAttribute("Kmin", UintegerValue(300));
        // tc->SetAttribute("DeviceRate", UintegerValue(650));
    }
    // assign IP address 分配ip
    Ipv4AddressHelper address;
    address.SetBase("192.168.200.0", "255.255.255.0");
    Ipv4InterfaceContainer p2p_1_interfaces = address.Assign(p2p_1_devices);
    address.SetBase("192.168.201.0", "255.255.255.0");
    Ipv4InterfaceContainer p2p_2_interfaces = address.Assign(p2p_2_devices);
    address.SetBase("192.168.202.0", "255.255.255.0");
    Ipv4InterfaceContainer p2p_3_interfaces = address.Assign(p2p_3_devices);
    vector<Ipv4InterfaceContainer> access_interfaces(ap_num);

    for (uint32_t i = 0; i < ap_num; i++)
    {
        std::ostringstream subnet;
        subnet << "192.168.2" << i + 10 << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        access_interfaces[i] = address.Assign(access_devices[i]);
    }

    vector<Ipv4InterfaceContainer> ap_interfaces(ap_num);
    vector<Ipv4InterfaceContainer> wifi_nodes_interfaces(ap_num);
    for (uint32_t i = 0; i < ap_num; i++)
    {
        std::ostringstream subnet;
        subnet << "192.168.2" << i + 30 << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        ap_interfaces[i] = address.Assign(ap_devices[i]);
        wifi_nodes_interfaces[i] = address.Assign(wifi_nodes_devices[i]);
    }
    
    // create error model
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    double errRate = 0.00;
    em->SetAttribute("ErrorRate", DoubleValue(errRate));
    em->SetAttribute("ErrorUnit", EnumValue(0)); //错误字节的比率

    

    // for (uint32_t i = 0; i < ap_num; i++)
    // {
    //     access_devices[i].Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    //     access_devices[i].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    // }

    // p2p_1_devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    // p2p_1_devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    // p2p_2_devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    // p2p_2_devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    // p2p_3_devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    // p2p_3_devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    // end create error model

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    // End create topo

    // Start create flow 创建app
    double simulationTime = 10.0; // seconds
    double appStartTime = 2.0;
    uint32_t flownum = 10;
    vector<ApplicationContainer> sinkAppA(ap_num * flownum);
    vector<ApplicationContainer> sourceAppA(ap_num * flownum);
 

    for (uint32_t i = 0; i < ap_num; i++)
    {
        for (uint32_t j = 0; j < flownum; j++)
        {
            uint16_t port = 50000 + j;
            PacketSinkHelper sinkHelperA("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port)); //创建接受的app
            sinkAppA[i] = sinkHelperA.Install(wifi_nodes[i].Get(j % 2));
            sinkAppA[i].Start(Seconds(appStartTime - 1));
            sinkAppA[i].Stop(Seconds(simulationTime + appStartTime));
            InetSocketAddress remoteAddressA = InetSocketAddress(wifi_nodes_interfaces[i].GetAddress(j % 2), port);
            // remoteAddressA.SetTos(0x14);channe
            BulkSendHelper sourceHelperA("ns3::TcpSocketFactory", remoteAddressA); //创建发送app
            sourceHelperA.SetAttribute("SendSize", UintegerValue(1000));
            sourceHelperA.SetAttribute("MaxBytes", UintegerValue(0));
            sourceAppA[i].Add(sourceHelperA.Install(remote_servers.Get(0)));
            sourceAppA[i].Start(Seconds(appStartTime));
            sourceAppA[i].Stop(Seconds(simulationTime + appStartTime));
        }
    }
    // for (uint32_t i = 0; i < ap_num; i++)
    // {
    //     uint16_t port = 50000;
    //     PacketSinkHelper sinkHelperA("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port)); //创建接受的app
    //     sinkAppA[i] = sinkHelperA.Install(wifi_nodes[i].Get(0));
    //     sinkAppA[i].Start(Seconds(appStartTime - 1));
    //     sinkAppA[i].Stop(Seconds(simulationTime + appStartTime));
    //     InetSocketAddress remoteAddressA = InetSocketAddress(wifi_nodes_interfaces[i].GetAddress(0), port);
    //     // remoteAddressA.SetTos(0x14);
    //     OnOffHelper onoffA1("ns3::TcpSocketFactory", remoteAddressA);

    //     onoffA1.SetAttribute("PacketSize", UintegerValue(1500));
    //     onoffA1.SetConstantRate(DataRate("1000Mb/s"));
    //     // BulkSendHelper sourceHelperA("ns3::TcpSocketFactory", remoteAddressA); //创建发送app
    //     onoffA1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=8]"));
    //     onoffA1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=5]"));
    //     sourceAppA[i].Add(onoffA1.Install(remote_servers.Get(0)));
    //     sourceAppA[i].Start(Seconds(appStartTime));
    //     sourceAppA[i].Stop(Seconds(simulationTime + appStartTime));
    // }

    
    
    Ptr<TcpL4Protocol> protocolyyq = ap.Get(0)->GetObject<TcpL4Protocol>();
    Config::SetGlobal("apwindow",PointerValue(protocolyyq));                           
    // End create flow

    ostringstream oss;
    oss<<"/NodeList/"<< ap.Get(0)->GetId()<<"/DeviceList/*/$ns3::WifiNetDevice/*/$ns3::ApWifiMac/*/Queue/Dequeue";
    // oss<<"/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/*/$ns3::ApWifiMac/*/Queue/Dequeue";
    Config::Connect(oss.str(),MakeCallback(&DequeueTrace));

    ostringstream ossenqueue;
    ossenqueue<<"/NodeList/"<< ap.Get(0)->GetId()<<"/DeviceList/*/$ns3::WifiNetDevice/*/$ns3::ApWifiMac/*/Queue/Enqueue";
    Config::Connect(ossenqueue.str(),MakeCallback(&EnqueueForRttTrace));

    // Start Trace
    Ptr<PointToPointNetDevice> device1 = DynamicCast<PointToPointNetDevice>(p2p_1_devices.Get(0)); //每间隔一段时间去记录
    // double intvl = 0.05;
    double intvl = 0.1;
    // double intvl = 0.2;
    Simulator::Schedule(Seconds(intvl), &TraceThput, "result/thput.csv", device1, 0, intvl); //记录吞吐，在01s后调用tarce函数
    // void RateByWifiNetDevice(Ptr<WifiNetDevice> device, int no, double interval);
    Ptr<WifiNetDevice> device2 = DynamicCast<WifiNetDevice>(ap_devices[0].Get(0));
    // Simulator::Schedule(Seconds(intvl), &RateByWifiNetDevice, device2, 1, intvl);

    Simulator::Schedule(Seconds(appStartTime + 0.00001), &TraceRtt, "result/rtt.csv"); //时延
    // Simulator::Schedule(Seconds(appStartTime + 0.00001), &Tracepre, "result/prertt.csv");
    ossprertt.open("result/prertt.csv");
    qlenOutput.open("result/qlen.csv"); //队列长度
    Ptr<ns3::WifiMacQueue> queue = DynamicCast<RegularWifiMac>(device2->GetMac())->GetTxopQueue(AC_BE);
    queue->TraceConnectWithoutContext("PacketsInQueue", MakeBoundCallback(&DevicePacketsInQueueTrace, root_qdisc));


    // Ptr<WifiNetDevice> wifi_nodedevice = DynamicCast<WifiNetDevice>(ap_devices[0].Get(0));
    // // Ptr<WifiNetDevice> wifi_nodedevice = DynamicCast<WifiNetDevice>(wifi_nodes_devices[0].Get(0));

    // Ptr<ns3::WifiPhy> wifi_yyq = DynamicCast<ns3::WifiPhy>(wifi_nodedevice->GetPhy());
    // wifi_yyq->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&PhyRxDrop));

    // void printCurrentSize(Ptr<QueueDisc> root);
    // Simulator::Schedule(Seconds(0.1), &printCurrentSize, root);
    // End Trace

    // Start Run Simulation
    Simulator::Stop(Seconds(simulationTime + appStartTime + 0.00001));
    time_t start = time(nullptr);
    Simulator::Run();
    time_t end = time(nullptr);
    std::cout << "Simulate time : " << end - start << " seconds" << std::endl;
    Simulator::Destroy();
    // End Run Simulation

    uint64_t rxBytes = DynamicCast<PacketSink>(sinkAppA[0].Get(0))->GetTotalRx(); // pure data: 1448B per packet
    double throughput = (rxBytes * 8) / (simulationTime * 1000000.0);             // Mbit/s
    std::cout << "Receive APP throughput : " << throughput << " Mbps" << std::endl;

    return 0;
}

void printCurrentSize(Ptr<QueueDisc> root)
{
    Time time = Seconds(0.001);
    cout << Simulator::Now().GetSeconds() << '\t' << root->GetQueueDiscClass(0)->GetQueueDisc()->GetCurrentSize() << endl;
    Simulator::Schedule(time, &printCurrentSize, root);
}

void RateByWifiNetDevice(Ptr<WifiNetDevice> device, int no, double interval)
{
    Time time = Seconds(interval);
    uint64_t sendBytes = DynamicCast<RegularWifiMac>(device->GetMac())->GetTxopQueue(AC_BE)->m_sendBytes;
    tmp[no] = sendBytes - lastSendBytes[no];
    lastSendBytes[no] = sendBytes;
    double rate = tmp[no] * 8 / (1000 * 1000 * time.GetSeconds());
    cout << Simulator::Now().GetSeconds() << '\t' << rate << "Mbps" << endl;
    Simulator::Schedule(time, &RateByWifiNetDevice, device, no, interval);
}
