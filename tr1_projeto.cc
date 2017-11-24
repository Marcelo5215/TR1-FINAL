/******************************************
* Universidade de Brasília
* Departamento de Ciência da Computação
* 204315 Teleinformática e Redes 1
* Profa. Priscila Solís Barreto
*******************************************
* Alunos:
* Davi Rabbouni de Carvalho Freitas - 15/0033010
* Marcelo de Araújo Lopes Júnior - 15/0016794
* Rafael de Lima Chehab - 15/0045123
*******************************************
* Projeto Final de TR1
* a)quatro LANs Ethernet (padrão 802.3) ou duas redes LANs Ethernet (802.3)
* e duas redes celulares (3G)
* b) duas LANs Wi-Fi sem fio (padrão 802.11x)
* c) uma WAN, em qualquer padrão, interligando todas as
* LANs. A topologia e distribuição das redes fica a critério dos projetistas.
* d) um mínimo de 10 clientes em cada uma das redes.
* O número de nós na rede de transporte fica a critério dos projetistas.
* Em uma das redes 802.3 deve existir um servidor de aplicação que
* precisa ser acessado por nós/clientes das outras redes.
*******************************************
// Default Network Topology (Mesh - todas ligam com tdas)
//
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//
//                     Wifi_1 10.1.16.0
//                                                   AP
// *      *     *     *     *     *    *    *    *    *
// |      |     |     |     |     |    |    |    |    |          10.1.1.0
// n14   n13   n12   n11   n10   n9   n8   n7   n6   n0 ------------------------------  n1  n24   n25    n25   n26   n27   n28   n29   n30   n31
//                                                    -----   point-to-point             |    |     |      |     |     |     |     |     |     |
//                                         point-to-point |                              =======================================================
//                                          10.1.4.0      |                              |                 LAN_1 10.1.18.0
//                                                        |                              |
//                                                        |                              |
//                 LAN_2 10.1.19.0                        |                              |   10.1.6.0
//  =======================================================                              |   point-to-point
//  |     |     |     |     |     |     |     |     |     |        10.1.2.0              |
// n40   n39   n38   n37   n36   n35   n34   n33   n32   n2 --------------------------  n3     n41   n42   n43   n44   n45   n46   n47   n48   n49
//                                                        |  point-to-point              |      |     |     |     |     |     |     |     |     |
//                                     point-to-point     |                              =======================================================
//                                      10.1.5.0          |                              |               LAN_3 10.1.20.0
//                                                        |                              |
//                                                        AP                             | 10.1.7.0
//                  Wifi_2 10.1.17.0                      |                              | point-to-point
//  *     *     *     *     *     *     *     *     *     *                              |
//  |     |     |     |     |     |     |     |     |     |    10.1.3.0                  |
// n23   n22   n21   n20   n19   n18   n17   n16   n15   n4 --------------------------  n5    n50   n51   n52   n53   n54   n55   n56   n57   n58
//                                                           point-to-point              |     |     |     |     |     |     |     |     |     |
//                                                                                       =======================================================
//                                                                                                          LAN_4 10.1.21.0
//
********************************************************************************
// Para simplificar representação, não se representara todas 15 ligações - apesar de estarem implementadas
********************************************************************************
//  ´Protocolos : IPV4,UDP, Point-to-Point, CSMA,
//   802.11 Wi-Fi, 802.3 Ethernet
*/

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

//Evitar uso das estruturas do ns3 como ns3::
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ProjFinalTR1");

//Struct que contem os componentes necessarios
//que caracterizara as 4 redes 802.3
typedef struct  {
  NodeContainer nodeContainer;
  NetDeviceContainer deviceContainer;
  Ipv4InterfaceContainer interfaceContainer;
}CSMAContainer;

//Contem componentes necessarios para caracterizar
//uma rede wifi.
//Precisa de caracteristicas para o ponto acesso (AP)
//e os nos extras (Stations)
typedef struct  {
  NodeContainer nodeAp;
  NetDeviceContainer containerAp;
  //Ipv4InterfaceContainer interfaceApContainer;
  NodeContainer nodeSta;
  NetDeviceContainer containerSta;
  //Ipv4InterfaceContainer interfaceStaContainer;
}WifiContainer;

//Funcao que cria uma struct caracteristica do CSMA
//colocando o NodeContainer (== vetor de Ptr<Node>) node como
//roteador da rede com nCsma nos extras
CSMAContainer createCSMA (CsmaHelper csma, Ptr<Node> node, uint32_t nCsma);
//Cria uma struct caracteristica do Wifi,
//colocando o NodeContainer como ponto de acesso
//e nWifi como estacoes da rede
WifiContainer createWifi (YansWifiPhyHelper wifi, Ptr<Node> node, uint32_t nWifi);


int main (int argc, char *argv[]){

  //Permite debugger
  bool verbose = true;
  bool tracing = true;
  //10 nos = 1 roteador + 9 nos extras
  uint32_t nCsma = 9;
  uint32_t nWifi = 9;

  //Permite alteracao de parametros por linhas de comando
  CommandLine cmd;
  cmd.AddValue ("nCsma", "Numero de \"extra\" nos/dispositivos CSMA", nCsma);
  cmd.AddValue ("nWifi", "Numero de devices STA", nWifi);
  cmd.AddValue ("verbose", "Se verdadeiro, aplicacoes echo vao transmitir log", verbose);
  cmd.AddValue ("tracing", "Permite traceamento pcap", tracing);

  cmd.Parse (argc,argv);

//Se a variavel verbose tiver setada, enable logging do
//cliente e servidor UDP
  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  //Medicao em nanosegundos
  Time::SetResolution (Time::NS);
  //Criacao de 6 nos, para as 6 redes a serem
  //implementadas
  NodeContainer p2pNodes;
  p2pNodes.Create (6);

  //Valores similares aos usados nos tutoriais do ns3
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  //Criacao dos canais de conexao entre as redes (15 canais)
  NetDeviceContainer p2pDevices_1,p2pDevices_2,p2pDevices_3,p2pDevices_4,p2pDevices_5,
  p2pDevices_6,p2pDevices_7,p2pDevices_8,p2pDevices_9,p2pDevices_10,
  p2pDevices_11,p2pDevices_12,p2pDevices_13,p2pDevices_14,p2pDevices_15;

  //Conexao dos 15 p2p
  p2pDevices_1 = pointToPoint.Install(p2pNodes.Get(0),p2pNodes.Get(1));
  p2pDevices_2 = pointToPoint.Install(p2pNodes.Get(2),p2pNodes.Get(3));
  p2pDevices_3 = pointToPoint.Install(p2pNodes.Get(4),p2pNodes.Get(5));
  p2pDevices_4 = pointToPoint.Install(p2pNodes.Get(0),p2pNodes.Get(2));
  p2pDevices_5 = pointToPoint.Install(p2pNodes.Get(2),p2pNodes.Get(4));
  p2pDevices_6 = pointToPoint.Install(p2pNodes.Get(1),p2pNodes.Get(3));
  p2pDevices_7 = pointToPoint.Install(p2pNodes.Get(3),p2pNodes.Get(5));
  p2pDevices_8 = pointToPoint.Install(p2pNodes.Get(0),p2pNodes.Get(3));
  p2pDevices_9 = pointToPoint.Install(p2pNodes.Get(0),p2pNodes.Get(4));
  p2pDevices_10 = pointToPoint.Install(p2pNodes.Get(0),p2pNodes.Get(5));
  p2pDevices_11 = pointToPoint.Install(p2pNodes.Get(1),p2pNodes.Get(2));
  p2pDevices_12 = pointToPoint.Install(p2pNodes.Get(1),p2pNodes.Get(4));
  p2pDevices_13 = pointToPoint.Install(p2pNodes.Get(1),p2pNodes.Get(5));
  p2pDevices_14 = pointToPoint.Install(p2pNodes.Get(2),p2pNodes.Get(5));
  p2pDevices_15 = pointToPoint.Install(p2pNodes.Get(3),p2pNodes.Get(4));

  //Caracteristicas de canal presentes nos exemplos
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

//Criacao das redes de acordo com a topologia no inicio do codigo
  //Criacao das redes CSMA
  CSMAContainer CSMA_1, CSMA_2, CSMA_3, CSMA_4;
  CSMA_1 = createCSMA(csma, p2pNodes.Get(1),nCsma);
  CSMA_2 = createCSMA(csma, p2pNodes.Get(2),nCsma);
  CSMA_3 = createCSMA(csma, p2pNodes.Get(3),nCsma);
  CSMA_4 = createCSMA(csma, p2pNodes.Get(5),nCsma);

  //Criacao das redes Wifi
  YansWifiPhyHelper wifi = YansWifiPhyHelper::Default();
  WifiContainer Wifi_1, Wifi_2;
  Wifi_1 = createWifi(wifi, p2pNodes.Get(0), nWifi);
  Wifi_2 = createWifi(wifi, p2pNodes.Get(4), nWifi);

//Instalacao das pilhas de protocolo em todas as redes criadas
  InternetStackHelper stack;
  //CSMA
  stack.Install(CSMA_1.nodeContainer);
  stack.Install(CSMA_2.nodeContainer);
  stack.Install(CSMA_3.nodeContainer);
  stack.Install(CSMA_4.nodeContainer);
  //Wifi
  stack.Install(Wifi_1.nodeAp);
  stack.Install(Wifi_1.nodeSta);
  stack.Install(Wifi_2.nodeAp);
  stack.Install(Wifi_2.nodeSta);

  //Criacao dos endereco IP para os elementos de rede
  Ipv4AddressHelper address;

  //Primeiro, atribuicao para as interfaces de comunicacao
  //10.1.1.0 -> no 0 ---> no 1
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_1;
  p2pInterfaces_1 = address.Assign (p2pDevices_1);
  /**********************************************/
  //10.1.2.0 -> no 2 ---> no 3
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_2;
  p2pInterfaces_2 = address.Assign (p2pDevices_2);
  /**********************************************/
  //10.1.3.0 -> no 4 ---> no 5
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_3;
  p2pInterfaces_3 = address.Assign (p2pDevices_3);
  /**********************************************/
  //10.1.4.0 -> no 0 ---> no 2
  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_4;
  p2pInterfaces_4 = address.Assign (p2pDevices_4);
  /**********************************************/
  //10.1.5.0 -> no 2 ---> no 4
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_5;
  p2pInterfaces_5 = address.Assign (p2pDevices_5);
  /**********************************************/
  //10.1.6.0 -> no 1 ---> no 3
  address.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_6;
  p2pInterfaces_6 = address.Assign (p2pDevices_6);
  /**********************************************/
  //10.1.7.0 -> no 3 ---> no 5
  address.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_7;
  p2pInterfaces_7 = address.Assign (p2pDevices_7);
  /**********************************************/
  //10.1.8.0 -> no 0 ---> no 3
  address.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_8;
  p2pInterfaces_8 = address.Assign (p2pDevices_8);
  /**********************************************/
  //10.1.9.0 -> no 0 ---> no 4
  address.SetBase ("10.1.9.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_9;
  p2pInterfaces_9 = address.Assign (p2pDevices_9);
  /**********************************************/
  //10.1.10.0 -> no 0 ---> no 5
  address.SetBase ("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_10;
  p2pInterfaces_10 = address.Assign (p2pDevices_10);
  /**********************************************/
  //10.1.11.0 -> no 1 ---> no 2
  address.SetBase ("10.1.11.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_11;
  p2pInterfaces_11 = address.Assign (p2pDevices_11);
  /**********************************************/
  //10.1.12.0 -> no 1 ---> no 4
  address.SetBase ("10.1.12.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_12;
  p2pInterfaces_12 = address.Assign (p2pDevices_12);
  /**********************************************/
  //10.1.13.0 -> no 1 ---> no 5
  address.SetBase ("10.1.13.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_13;
  p2pInterfaces_13 = address.Assign (p2pDevices_13);
  /**********************************************/
  //10.1.14.0 -> no 2 ---> no 5
  address.SetBase ("10.1.14.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_14;
  p2pInterfaces_14 = address.Assign (p2pDevices_14);
  /**********************************************/
  //10.1.15.0 -> no 3 ---> no 4
  address.SetBase ("10.1.15.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_15;
  p2pInterfaces_15 = address.Assign (p2pDevices_15);
  /**********************************************/
  //Atribuicao de enderecos para as redes Wifi
  //Wifi_1
  address.SetBase ("10.1.16.0", "255.255.255.0");
  address.Assign(Wifi_1.containerSta);
  address.Assign(Wifi_1.containerAp);
  //Wifi_2
  address.SetBase ("10.1.17.0", "255.255.255.0");
  address.Assign(Wifi_2.containerSta);
  address.Assign(Wifi_2.containerAp);
  /**********************************************/
  //Atribuicao de enderecos para as redes CSMA
  //CSMA_1
  address.SetBase ("10.1.18.0", "255.255.255.0");
  CSMA_1.interfaceContainer = address.Assign (CSMA_1.deviceContainer);
  //CSMA_2
  address.SetBase ("10.1.19.0", "255.255.255.0");
  CSMA_2.interfaceContainer = address.Assign (CSMA_2.deviceContainer);
  //CSMA_3
  address.SetBase ("10.1.20.0", "255.255.255.0");
  CSMA_3.interfaceContainer = address.Assign (CSMA_3.deviceContainer);
  //CSMA_4
  address.SetBase ("10.1.21.0", "255.255.255.0");
  CSMA_4.interfaceContainer = address.Assign (CSMA_4.deviceContainer);

  //Uma vez que os enderecos foram atribuidos às redes,
  //vamos colocar o servidor de eco na rede LAN_1 (10.1.18.0)
  //O servidor ficara no nó mais a "direita", para que as transmissoes
  //passem por todos os nos da rede

  //Servidor de eco na porta 9
  UdpEchoServerHelper echoServer (9);

  //Servidor de aplicacao UDP instalado na rede CSMA_1
  ApplicationContainer serverApps = echoServer.Install (CSMA_1.nodeContainer.Get(nCsma));
  //Hora de inicio e termino da aplicacao (em segundos)
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (60.0));

  //Criacao dos clientes apontando para a CSMA_1
  UdpEchoClientHelper echoClient (CSMA_1.interfaceContainer.GetAddress(nCsma), 9);
  //Atributos do cliente (conforme visto nos exemplos do NS3)
  echoClient.SetAttribute ("MaxPackets", UintegerValue (100));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  //Criacao de um container com os clientes para o servidor,
  //os quais serao todas as redes restantes
  //Clientes ficarao no ultimo elemento de cada rede
  ApplicationContainer clientApps = echoClient.Install (Wifi_1.nodeSta.Get(nWifi - 1));
  //Instalacao nas redes restantes e
  //adicao do ponteiro delas ao ApplicationContainer
  clientApps.Add(echoClient.Install (Wifi_2.nodeSta.Get(nWifi - 1)));
  clientApps.Add(echoClient.Install (CSMA_1.nodeContainer.Get(nCsma - 1)));
  clientApps.Add(echoClient.Install (CSMA_2.nodeContainer.Get(nCsma - 1)));
  clientApps.Add(echoClient.Install (CSMA_3.nodeContainer.Get(nCsma - 1)));
  clientApps.Add(echoClient.Install (CSMA_4.nodeContainer.Get(nCsma - 1)));

  //Tempo de inicio e parada (cliente deve comecar dps do servidor)
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (60.0));

  //Ativa roteamento inter-redes
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //Manda simulador parar (senao roda infinitamente)
  Simulator::Stop (Seconds (60.0));

  if (tracing == true){
  //Rastreamento para as redes
  pointToPoint.EnablePcapAll("ProjetoTR1");
  wifi.EnablePcap("Projeto_TR1_Wifi_1", Wifi_1.containerAp.Get(0));
  wifi.EnablePcap("Projeto_TR1_Wifi_2", Wifi_2.containerAp.Get(0));
  csma.EnablePcap("Projeto_TR1_CSMA_1", CSMA_1.deviceContainer.Get(0), true);
  csma.EnablePcap("Projeto_TR1_CSMA_2", CSMA_2.deviceContainer.Get(0), true);
  csma.EnablePcap("Projeto_TR1_CSMA_3", CSMA_3.deviceContainer.Get(0), true);
  csma.EnablePcap("Projeto_TR1_CSMA_4", CSMA_4.deviceContainer.Get(0), true);
  }

  //Execucao e destruicao da simulacao
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

//Funcao de criacao das caracteristicas da rede CSMA
CSMAContainer createCSMA (CsmaHelper csma, Ptr<Node> node, uint32_t nCsma){
  NodeContainer csmaNodes;
  csmaNodes.Add(node);
  csmaNodes.Create(nCsma);

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  //Atribuicao do que foi criado ao CSMAContainer
  CSMAContainer csmaContainer;
  csmaContainer.nodeContainer = csmaNodes;
  csmaContainer.deviceContainer = csmaDevices;

  //Sem atribuicao do campo interface, retorna CSMAContainer criado
  return csmaContainer;
}

//Funcao de criacao das caracteristicas da rede Wifi
WifiContainer createWifi (YansWifiPhyHelper wifi, Ptr<Node> node, uint32_t nWifi){
  //Cria rede com 9 nos estacao
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(nWifi);
  //Ponto de acesso passado por parametro
  NodeContainer wifiApNode = node;

  //Configura canal
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  wifi.SetChannel (channel.Create());

  ///Usar WifiHelper
  WifiHelper wifi_helper;
  wifi_helper.SetRemoteStationManager ("ns3::AarfWifiManager");

  //MAC com nao-QoS
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  //Instalacao de dispositivos wifi nas estacoes
  NetDeviceContainer staDevices;
  staDevices = wifi_helper.Install (wifi, mac, wifiStaNodes);

  //Configuracao pra AP
  mac.SetType ("ns3::ApWifiMac",
  "Ssid", SsidValue (ssid));

  //Instalacao de dispositivo wifi no ponto de acesso
  NetDeviceContainer apDevices;
  apDevices = wifi_helper.Install (wifi, mac, wifiApNode);


  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  //Permite mobilidade pra estacoes
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  //Mobilidade limitado pro AP
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  //Criacao e atribuicao dos valores do WifiContainer
  WifiContainer wifiContainer;
  wifiContainer.nodeAp = wifiApNode;
  wifiContainer.containerAp = apDevices;
  wifiContainer.nodeSta = wifiStaNodes;
  wifiContainer.containerSta = staDevices;

  return wifiContainer;
}
