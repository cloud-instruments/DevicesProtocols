// test application for ciupClientDll

using System;
using System.Text;

namespace ciupClientTest_csc
{
    class Program
    {
        // callback for incoming data
        // json: incoming data in json string format
        // id: numeric id of the receiver (as returned by ciupcStartReceiver)
        // fromAddr: string IP of sender
        // fromPort: sender port
        static void ciupDataCb(String json, int id, String fromAddr, ushort fromPort)
        {
            printLog(logPath, "T", id.ToString(),"(", fromAddr, ":", Convert.ToString(fromPort),": ",json);
            Console.WriteLine("{0}({1}:{2}): {3}",id,fromAddr,fromPort,json);
        }

        // callback for errors
        // code: numeric code of error
        // descriprion: string description of error
        // id: numeric id of the receiver (as returned by ciupcStartReceiver)
        static void ciupErrorCb(int code, String description, int id)
        {
            printLog(logPath, "E", id.ToString(), ": error:", code.ToString(), " ", description);
            Console.WriteLine("{0}: error:{1} {2}", id, code, description);
        }

        static string logPath = "";

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
            }catch
            {
                print_usage();
                return;
            }

            printLog(logPath, "T", "Starting client for ", addr, ":", Convert.ToString(port));
            Console.WriteLine("Starting client for {0}:{1}", addr, port);

            StringBuilder json = new StringBuilder(4096);
            if (SafeNativeMethods.ciupcGetServerInfo(addr, port, json, json.Capacity) == 0)
            {
                printLog(logPath, "T", "server info: ", json.ToString());
                Console.WriteLine("server info: {0}", json);
            }
            else
            {
                printCiupError("ciupcGetServerInfo");
                return;
            }


            printLog(logPath, "T", "Starting receiver ");
            Console.WriteLine("Starting receiver {0}");
            SafeNativeMethods.ciupDataCbDelegate pDataCb = ciupDataCb;
            SafeNativeMethods.ciupErrorCbDelegate pErrorCb = ciupErrorCb;
            int id = SafeNativeMethods.ciupcStartReceiver(addr, port, pDataCb, pErrorCb);
            if (id < 0)
            {
                printCiupError("ciupcStartReceiver");
                return;
            }

            // intercept [CTRL][c] while sleeping
            Console.CancelKeyPress += delegate (object sender, ConsoleCancelEventArgs e) {
                printLog(logPath, "T", "[CTRL][c]");
                Console.WriteLine("[CTRL][c]");
                e.Cancel = true;
                run = false;
            };

            // sleep until ctrl-c
            while (run) System.Threading.Thread.Sleep(100);

            SafeNativeMethods.ciupcStopAllReceivers();
        }

        static void print_usage()
        {

            string codeBase = System.Reflection.Assembly.GetExecutingAssembly().CodeBase;
            string name = System.IO.Path.GetFileName(codeBase);

            Console.WriteLine("Cloud Instruments Unified Protocol client test application");
            Console.WriteLine("usage: ciupClientTest-csc ip port");
            Console.WriteLine("  -l path : enable logfile");
        }

        static void printCiupError(String msg)
        {
            StringBuilder errdescr = new StringBuilder(4096);
            int errcode = SafeNativeMethods.ciupcGetLastError(errdescr, errdescr.Capacity);
            Console.WriteLine("{0} error:{1} {2}", msg, errcode, errdescr);
        }

        static void printLog(string path, string label, params string[] args)
        {
            if (path.Length == 0) return;
            System.IO.File.AppendAllText(path, DateTime.UtcNow.ToString("yyyy-MM-dd HH:mm:ss") + " " + label + " " + String.Join("", args) + "\n");
        }
    }
}
