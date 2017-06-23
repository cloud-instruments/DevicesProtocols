// P/Invoke import function for ciupClientDll to C#
// ciupClientDll.dll must be linked via project properties

using System;

// required for DllImport
using System.Runtime.InteropServices;

namespace ciupClientTest_csc
{
    internal static class NativeMethods
    {
        // values for msgtype in ciupDataCbDelegate
        public const int msgtype_serverinfo = 1;
        public const int msgtype_datapoint = 2;

        // callback function called by ciupClientDll to pass real-time incoming datapoints
        public delegate void ciupDataCbDelegate(int msgtype, String json, int id);

        // callback function called by ciupClientDll to notify errors
        public delegate void ciupErrorCbDelegate(int code, String description, int id);

        // connect to a ciupServer and send start command
        // return the id of connection (identifier in case of multiple connection)
        // return <0 for errors
        [DllImport("ciupClientDll.dll")]
        public static extern int ciupcConnect(
            String addr,                                                          // ip address of the server
            ushort port,                                                          // TCP port of the server
            [MarshalAs(UnmanagedType.FunctionPtr)]ciupDataCbDelegate dataCb,      // data callback
            [MarshalAs(UnmanagedType.FunctionPtr)]ciupErrorCbDelegate errorCb);   // errors callback

        // send a request for server info
        // answer will be passed to data callback
        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcInfo(
            int ID);   // id of the connection (returned by ciupcConnect)

        // send start message to server 
        // server wil start sending available datapoints
        // start is required only after a stop (connect starts automatically)
        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcStart(
            int ID);  // id of the connection (returned by ciupcConnect)

        // send stop message to server 
        // server wil stop sending datapoints
        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcStop(
            int ID);  // id of the connection (returned by ciupcConnect)

        // close the connection
        [DllImport("ciupClientDll.dll")]
        public static extern void ciupcDisconnect(
            int ID);  // id of the connection (returned by ciupcConnect)
    }
}
