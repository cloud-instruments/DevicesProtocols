using System;
// required for DllImport
using System.Runtime.InteropServices;
using System.Text;

namespace ciupClientTest_csc
{
    public static class PInvokeHelper
    {
        [DllImport("ciupClientDll.dll")]
        public static extern int ciupcGetLastError(StringBuilder descr, int maxlen);

        [DllImport("ciupClientDll.dll")]
        public static extern int ciupcGetServerInfo(String addr, ushort port, StringBuilder json, int jsonlen);

        public delegate void ciupDataCbDelegate(String json, int id, String fromAddr, ushort fromPort);

        public delegate void ciupErrorCbDelegate(int code, String description, int id);
        [DllImport("ciupClientDll.dll")]
        public static extern int ciupcStartReceiver(String addr, ushort port, [MarshalAs(UnmanagedType.FunctionPtr)]ciupDataCbDelegate dataCb, [MarshalAs(UnmanagedType.FunctionPtr)]ciupErrorCbDelegate errorCb);

        [DllImport("ciupClientDll.dll")]
        static extern int ciupcStopReceiver(int ID);

        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcStopAllReceivers();
    }
}
