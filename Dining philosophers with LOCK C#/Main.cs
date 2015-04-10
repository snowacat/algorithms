namespace Lab2
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    
    class Program
    {
        private static List<bool> forks = new List<bool>();
        private static List<Int32> times = new List<Int32>();
        private static List<AutoResetEvent> forkSyncEvents = new List<AutoResetEvent>();
        private static List<PhilStates> states = new List<PhilStates>();
        private static string path = @"C:\cod\log.txt";
        private static object logSyncObj = new object();

        protected static void LogState(object state)
        {
            lock (logSyncObj)
            {
                using (var s = File.AppendText(path))
                {
                    for (int i = 0; i < forks.Count; i++)
                    {
                        var st = (forks[i] ? "  " : "| ") + states[i].ToString() + " ";
                        Console.Write(st);
                        s.Write(st);
                    }
                    s.WriteLine();
                    Console.WriteLine();
                }
            }
        }

        static void Main(string[] args)
        {
            if (File.Exists(path))
            {
                File.Delete(path);
            }

            Console.Write("Enter number of eaters: ");
            var count = Int32.Parse(Console.ReadLine());
            for (int i = 0; i < count; i++)
            {
                forks.Add(false);
                //initially all philosophers want to think
                states.Add(PhilStates.T);
                forkSyncEvents.Add(new AutoResetEvent(true)); // True => set will be auto
            }

            for (int i = 0; i < count; i++)
            {
                var num = i;
                new Thread(new ThreadStart(() =>
                {
                    // Eater number
                    var lf = num;
                    var rf = (num + 1) % count;
                    WaitHandle[] evts = new WaitHandle[] { forkSyncEvents[lf], forkSyncEvents[rf] };

                    while (true)
                    {
                        // Wait for all forks
                        lock (logSyncObj)
                        {
                            states[num] = PhilStates.W;
                        }
                        WaitHandle.WaitAll(evts);

                        // Take forks
                        lock (logSyncObj)
                        {
                            forks[lf] = true;
                            forks[rf] = true;
                            // Eat
                            states[num] = PhilStates.E;
                        }
                        Thread.Sleep(new Random().Next(50) + 20);
                        
                        // Put forks
                        lock (logSyncObj)
                        {
                            forks[lf] = false;
                            forks[rf] = false;
                        }
                        forkSyncEvents[lf].Set();
                        forkSyncEvents[rf].Set();
                        
                        // Thinking
                        lock (logSyncObj)
                        {
                            states[num] = PhilStates.T;
                        }
                        Thread.Sleep(new Random().Next(100));
                    }

                })).Start();
            }

            Timer t = new Timer(LogState, null, 0, 10);
            Console.ReadLine();
        }

        private enum PhilStates
        {
            E,
            T,
            W
        };
    }
}
