// test application for ciupClientDll
// example of typical use for C#

using System;

namespace ciupClientTest_csc
{
    class Program
    {
        enum logLevel { debug, trace, warning, error };
        static logLevel logFilter = logLevel.debug;

        static Boolean performance=false;
        static int tPrev = Environment.TickCount;
        static int cPrev = 0;
        static int msgcount = 0;

        // callback for incoming data
        // json: incoming data in json string format
        // id: numeric id of the receiver (as returned by ciupcStartReceiver)
        // fromAddr: string IP of sender
        // fromPort: sender port
        static void ciupDataCb(int msgtype, String json, int id)
        {
            if (performance)
            {
                int tNow = Environment.TickCount;
                msgcount++;

                if (tNow - tPrev > 1000)
                {
                    // FIXME: don't care overflow
                    double mS = (msgcount - cPrev) / ((tNow - tPrev) / 1000.0);

                    Console.WriteLine("{0}: {1} msg/s ({2} msg in {3} mS)", id, mS, msgcount - cPrev , tNow - tPrev);
                    PrintLog(logLevel.error, id.ToString(), ": ", mS.ToString(), " msg/s (", (msgcount - cPrev).ToString(), "msg in ", (tNow - tPrev).ToString(), " mS)");

                    tPrev = Environment.TickCount;
                    cPrev = msgcount;
                }
            }
            else
            {
                PrintLog(logLevel.trace, id.ToString(), ": (", msgtype.ToString(), ") ", json);

                Console.WriteLine("{0}: ({1}) {2}", id, msgtype, json);
            }
        }

        // callback for errors
        // code: numeric code of error
        // descriprion: string description of error
        // id: numeric id of the receiver (as returned by ciupcStartReceiver)
        static void ciupErrorCb(int code, String description, int id)
        {
            PrintLog(logLevel.error, id.ToString(), ": error:", code.ToString(), " ", description);
            Console.WriteLine("{0}: error:{1} {2}", id, code, description);
        }

        static string _logPath = "";

        private static void print_usage()
        {
            Console.WriteLine("Unified Protocol client test application, C# version");
            Console.WriteLine("usage: ciupClientTest-csc ip port");
            Console.WriteLine("  -l PATH : enable logfile");
            Console.WriteLine("  -f X : filter logfile (E:errors, W:warnings, T:trace, D:debug)");
            Console.WriteLine(" - p : performance mode");
        }

        static void Main(string[] args)
        {
            bool run = true;

            // parse command line arguments
            int expectedArgc = 2;
            for (int i = 0; i < args.Length; i++)
            {
                // print help
                if (args[i] == "-h")
                {
                    print_usage();
                    return;
                }

                // enable log file
                if (args[i] == "-l")
                {
                    if (i >= args.Length - 1)
                    {
                        print_usage();
                        return;
                    }
                    _logPath = args[i + 1];
                    expectedArgc += 2;
                }

                // set loglevel filter
                if (args[i] == "-f")
                {
                    if (i >= args.Length - 1)
                    {
                        print_usage();
                        return;
                    }

                    switch (args[i + 1][0])
                    {
                        case 'E': logFilter = logLevel.error; break;
                        case 'W': logFilter = logLevel.warning; break;
                        case 'T': logFilter = logLevel.trace; break;
                        case 'D': logFilter = logLevel.debug; break;
                        default:
                            print_usage();
                            return;
                    }

                    expectedArgc += 2;
                }

                // performance mode
                if (args[i] == "-p")
                {
                    performance = true;
                    expectedArgc += 1;
                }
            }

            // min expected arguments is 2 (IP and port)
            if (args.Length < expectedArgc)
            {
                print_usage();
                return;
            }

            string addr;
            ushort port;

            try
            {
                addr = args[expectedArgc - 2];
                port = Convert.ToUInt16(args[expectedArgc - 1]);
            }
            catch
            {
                print_usage();
                return;
            }

            PrintLog(logLevel.trace, "Connecting to ", addr, ":", Convert.ToString(port));
            Console.WriteLine("Connecting to {0}:{1}", addr, port);

            try
            {
                // connect to the ciupServer
                NativeMethods.ciupDataCbDelegate pDataCb = ciupDataCb;
                NativeMethods.ciupErrorCbDelegate pErrorCb = ciupErrorCb;

                int id = NativeMethods.ciupcConnect(addr, port, pDataCb, pErrorCb);
                if (id < 0)
                {
                    PrintLog(logLevel.error, "Cannot connect");
                    Console.WriteLine("Cannot connect");
                    return;
                }

                PrintLog(logLevel.trace, "Connection id: ", id.ToString());
                Console.WriteLine("Connection id: {0}", id);

                // intercept [CTRL][c] while sleeping
                Console.CancelKeyPress += delegate (object sender, ConsoleCancelEventArgs e) 
                {
                    PrintLog(logLevel.trace, "[CTRL][c]");
                    Console.WriteLine("[CTRL][c]");
                    e.Cancel = true;
                    run = false;
                };

                while (run)
                {
                    System.Threading.Thread.Sleep(5000);
                    // discomment to receive some serverinfo packets
                    NativeMethods.ciupcInfo(id);
                }

                // close connection
                NativeMethods.ciupcStop(id);
                NativeMethods.ciupcDisconnect(id);
            }
            catch (Exception e)
            {
                PrintLog(logLevel.trace, e.ToString());
                Console.WriteLine($"Error: {e}");
            }
        }

        static void PrintLog(logLevel lev, params string[] args)
        {
            if (_logPath.Length == 0) return;
            if (lev < logFilter) return;

            string label = "D";

            switch (lev){
                case logLevel.trace:   label = "T"; break;
                case logLevel.warning: label = "W"; break;
                case logLevel.error:   label = "E"; break;
            }

            try
            {
                System.IO.StreamWriter w = System.IO.File.AppendText(_logPath);
                w.WriteLine("{0:yyyy-MM-dd HH:mm:ss} {1} {2}", DateTime.UtcNow, label , String.Join(" ", args));
                w.Close();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Cannot write to {0} {1}", _logPath, ex);
            }
        }
           
    }
}
