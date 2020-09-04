using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MACAddressGetter
{


    // IPv6 loopback address is constructed using these steps:
    // 1. take the first byte of the mac address and flip the 2nd bit (LSB) 00000000 -> 00000010
    // 2. split the mac in 2 equal parts and add ff:fe between those parts
    // 3. add fe80:: as prefix
    // Here we reverse that
    public static string MacAddressFromAddress(System.Net.IPAddress address)
    {
        // 0  1:2  3:4  5:6  7:8 9:1011:1213:1415
        // fe80:0000:0000:0000:1034:56ff:fe78:9abc
        byte[] addressBytes = address.GetAddressBytes();
        List<byte> mac = new List<byte>();
        addressBytes[8] ^= System.Convert.ToByte(2);
        for (int i = 8; i < 16; i++)
        {
            if (i == 11 || i == 12)
            {
                continue;
            }
            mac.Add(addressBytes[i]);
        }
        return System.BitConverter.ToString(mac.ToArray()).Replace("-", string.Empty); ;
    }
}
