using UnityEngine;
using System.Net.NetworkInformation;
public class UDPBroadcastListen
{
    private string macAddress;
    private UdpConnection connection;


    public UDPBroadcastListen()
    {
        NetworkInterface[] networkInterfaces = NetworkInterface.GetAllNetworkInterfaces();
        int sendPort = 12346;
        int receivePort = 12345;
        connection = new UdpConnection();

        // Because android doesn't give us the mac address
        // we will fetch it using the link local IPv6
        // so we search through the interfaces to get the link local IPv6
        for (int i = 0; i < networkInterfaces.Length; i++)
        {
            foreach (var address in networkInterfaces[i].GetIPProperties().UnicastAddresses)
            {
                if (address.Address.IsIPv6LinkLocal)
                {
                    macAddress = MACAddressGetter.MacAddressFromAddress(address.Address);
                }
            }
        }
        connection.StartConnection("255.255.255.255", sendPort, receivePort);

    }

    public BicycleDetails Update(BicycleDetails bd)
    {
        string[] receivedPackets = connection.getMessages();
        // if we have any packet
        if (receivedPackets.Length > 0)
        {
            // each packet has multiple values seperated with ;
            // a sample packet can be seen here
            // VRADA;{{MAC ADDRESS}};speed=24.4
            // we might have more tags, again seperated with ; and in form key=value
            string[] packet = receivedPackets[0].Split(';');
            if (packet[0] == "VRADA")
            {
                int macAddressIndex = macAddress.CompareTo(packet[1].Replace(":", "").ToUpper());
                if (macAddressIndex == 0)
                {
                    for (int i = 2; i < packet.Length; i++)
                    {
                        string key = packet[i].Split(':')[0];
                        string value = packet[i].Split(':')[1];
                        switch (key)
                        {
                            case "speed":
                                bd.Speed = float.Parse(value);
                                break;
                            case "rpm":
                                bd.RPM = float.Parse(value);
                                break;
                            case "level":
                                bd.Level = int.Parse(value);
                                break;
                            case "distance":
                                bd.Distance = int.Parse(value);
                                break;
                            case "calories":
                                bd.Calories = float.Parse(value);
                                break;
                            case "hf":
                                bd.HF = float.Parse(value);
                                break;
                            case "power":
                                bd.Power = int.Parse(value);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
        return bd;
    }


    private string FormatMACAddress()
    {
        string mac = "";

        for (int i = 0; i < macAddress.Length; i++)
        {
            mac += macAddress[i];
            if (i > 0 && i % 2 == 0 && i - 1 != macAddress.Length)
            {
                mac += ":";
            }
        }
        return mac;
    }
    public void ChangeLevel(int level)
    {
        connection.Send("VRADA;" + FormatMACAddress() + ";level=" + level.ToString());
    }

    ~UDPBroadcastListen()
    {
        connection.Stop();
    }
}
