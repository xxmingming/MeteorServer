using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace KcpClient
{
    class Program
    {
        private static IPEndPoint remote;
        private static UdpClient udp;
        private static KCP kcp;
        private static int updateTime;
        private static bool willDisconnect;
        public static long ConvertDateTimeToInt(System.DateTime time)
        {
            System.DateTime startTime = TimeZone.CurrentTimeZone.ToLocalTime(new System.DateTime(1970, 1, 1, 0, 0, 0, 0));
            long t = (time.Ticks - startTime.Ticks) / 10000;   //除10000调整为13位      
            return t;
        }

        static void Main(string[] args)
        {
            Connect(9000);
            long tick = ConvertDateTimeToInt(DateTime.Now);
            while (true)
            {
                System.Threading.Thread.Sleep(1);
                Update();
            }
        }

        static byte[] buff = new byte[2];
        static bool sended = false;
        public static void Update()
        {
            if (!Connected)
            {
                return;
            }

            if (willDisconnect)
            {
                Disconnect();
                willDisconnect = false;
                return;
            }
            updateTime += 20;
            kcp.Update((uint)updateTime);
            if (length != 0)
            {
                byte[] b = new byte[length];
                Array.Copy(data_src, b, length);
                kcp.Input(b);
                length = 0; 
            }
            int n = kcp.Recv(buffer);
            if (n > 0)
            {
                //packetProxy.SetBuffer(buffer);
                //packetProxy.Analysis(n, Packet);
                Console.WriteLine(buffer);
            }

            //if (!sended)
            {
                buff[0] = 1;
                buff[1] = 0;
                kcp.Send(buff);
                sended = true;
            }
        }

        private static void SendWrap(byte[] data, int size)
        {
            try
            {
                udp.BeginSend(data, size, SendCallback, null);
            }
            catch (SocketException)
            {
                Disconnect();
            }
        }

        private static void SendCallback(IAsyncResult ar)
        {
            udp.EndSend(ar);
        }

        public static bool Disconnect()
        {
            if (!Connected)
            {
                return false;
            }
            Connected = false;
            udp.Close();
            return true;
        }

        public static bool Connected
        {
            get;
            private set;
        }

        public static bool Connect(int port)
        {
            if (Connected)
            {
                return false;
            }

            udp = new UdpClient("127.0.0.1", port);
            kcp = new KCP(1, SendWrap);
            kcp.NoDelay(1, 10, 2, 1);
            kcp.WndSize(128, 128);
            
            Receive();
            updateTime = 0;
            Connected = true;

            return true;
        }

        private static void Receive()
        {
            udp.BeginReceive(ReceiveCallback, null);
        }

        static byte[] buffer = new byte[4096];
        static byte[] data_src = new byte[4096];
        static int length = 0;
        private static void ReceiveCallback(IAsyncResult ar)
        {
            try
            {
                var data = udp.EndReceive(ar, ref remote);

                if (data != null)
                {
                    length = data.Length;
                    Array.Copy(data, data_src, data.Length);
                }

                Receive();
            }
            catch (SocketException)
            {
                willDisconnect = true;
            }
        }
    }
}
