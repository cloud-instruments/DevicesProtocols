// test application for ciupClientDll

using System;

namespace ciupClientTest_csc
{
    class Program
    {
        enum logLevel { debug, trace, warning, error };
        static logLevel logFilter = logLevel.debug;

        // callback for incoming data
        // json: incoming data in json string format
        // id: numeric id of the receiver (as returned by ciupcStartReceiver)
        // fromAddr: string IP of sender
        // fromPort: sender port
        static void ciupDataCb(int msgtype, String json, int id)
        {
            printLog(logLevel.trace, id.ToString(), ": (", msgtype.ToString(), ") ", json);
            Console.WriteLine("{0}: ({1}) {2}",id, msgtype, json);
        }

        // callback for errors
        // code: numeric code of error
        // descriprion: string description of error
        // id: numeric id of the receiver (as returned by ciupcStartReceiver)
        static void ciupErrorCb(int code, String description, int id)
        {
            printLog(logLevel.error, id.ToString(), ": error:", code.ToString(), " ", description);
            Console.WriteLine("{0}: error:{1} {2}", id, code, description);
        }

        static string logPath = "";

        static void print_usage()
        {
            Console.WriteLine("Cloud Instruments Unified Protocol client test application, C# version");
            Console.WriteLine("usage: ciupClientTest-csc ip port");
            Console.WriteLine("  -l PATH : enable logfile");
            Console.WriteLine("  -f X : filter logfile (E:errors, W:warnings, T:trace, D:debug)");
        }

        static void Main(string[] args)
        {
            bool run = true;

            // parse command line arguments
            int expected_argc = 2;
            for (int i = 0; i < args.Length; i++)
            {
                // enable log file
                if (args[i] == "-l")
                {
                    if (i >= args.Length - 1)
                    {
                        print_usage();
                        return;
                    }
                    logPath = args[i + 1];
                    expected_argc += 2;
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
                        case 'E': logFilter = logLevel.error;  break;
                        case 'W': logFilter = logLevel.warning; break;
                        case 'T': logFilter = logLevel.trace; break;
                        case 'D': logFilter = logLevel.debug; break;
                        default:
                            print_usage();
                            return;
                    }
                    expected_argc += 2;
                }

            }

            // min expected arguments is 2 (IP and port)
            if (args.Length < expected_argc)
            {
                print_usage();
                return;
            }

            string addr;
            ushort port;

            try
            {
                addr = args[expected_argc - 2];
                port = Convert.ToUInt16(args[expected_argc - 1]);
            }
            catch
            {
                print_usage();
                return;
            }

            printLog(logLevel.trace, "Connecting to ", addr, ":", Convert.ToString(port));
            Console.WriteLine("Connecting to {0}:{1}", addr, port);

            NativeMethods.ciupDataCbDelegate pDataCb = ciupDataCb;
            NativeMethods.ciupErrorCbDelegate pErrorCb = ciupErrorCb;
            int id = NativeMethods.ciupcConnect(addr, port, pDataCb, pErrorCb);
            if (id < 0)
            {
                printLog(logLevel.error, "Cannot connect");
                Console.WriteLine("Cannot connect");
                return;
            }

            printLog(logLevel.trace, "Connection id: ", id.ToString());
            Console.WriteLine("Connection id: {0}", id);

            // intercept [CTRL][c] while sleeping
            Console.CancelKeyPress += delegate (object sender, ConsoleCancelEventArgs e) {
                printLog(logLevel.trace, "[CTRL][c]");
                Console.WriteLine("[CTRL][c]");
                e.Cancel = true;
                run = false;
            };

            while (run)
            {
                System.Threading.Thread.Sleep(5000);
                NativeMethods.ciupcInfo(id);
            }

            NativeMethods.ciupcStop(id);
            NativeMethods.ciupcDisconnect(id);
        }

        static void printLog(logLevel lev, params string[] args)
        {
            if (logPath.Length == 0) return;
            if (lev < logFilter) return;

            string label = "D";
            switch (lev){
                case logLevel.trace:   label = "T"; break;
                case logLevel.warning: label = "W"; break;
                case logLevel.error:   label = "E"; break;
            }

            try
            {
                //System.IO.File.AppendAllText(logPath, DateTime.UtcNow.ToString("yyyy-MM-dd HH:mm:ss") + " " + label + " " + String.Join("", args) + "\r\n");
                System.IO.StreamWriter w = System.IO.File.AppendText(logPath);
                w.WriteLine("{0} {1} {2}", DateTime.UtcNow.ToString("yyyy-MM-dd HH:mm:ss"), label , String.Join("", args));
                //w.Flush();
                w.Close();
            }
            catch
            {
                Console.WriteLine("Cannot write to {0}", logPath);
            }
        }
           
    }
}
