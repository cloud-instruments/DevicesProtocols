// P/Invoke import function for ciupClientDll to C#

using System;

// required for DllImport
using System.Runtime.InteropServices;
using System.Text;

// TODO: define msgtype constants

namespace ciupClientTest_csc
{
    internal static class NativeMethods
    {
        public delegate void ciupDataCbDelegate(int msgtype, String json, int id);
        public delegate void ciupErrorCbDelegate(int code, String description, int id);

        [DllImport("ciupClientDll.dll")]
        public static extern int ciupcConnect(String addr, ushort port, [MarshalAs(UnmanagedType.FunctionPtr)]ciupDataCbDelegate dataCb, [MarshalAs(UnmanagedType.FunctionPtr)]ciupErrorCbDelegate errorCb);

        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcInfo(int ID);

        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcStart(int ID);

        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcStop(int ID);

        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcDisconnect(int ID);
    }
}
