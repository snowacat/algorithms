namespace Pp3
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.IO;
    using System.Threading;

    using System.Text;
    using System.Text.RegularExpressions;

    using MPI;

    class Program
    {
        static void Main(string[] args)
        {
            using (new MPI.Environment(ref args))
            {
                Intracommunicator communicator = Communicator.world;

                // If the main process
                if (communicator.Rank == 0)
                {
                    Console.Write("Input word for search: ");
                    String worldSearh = Console.ReadLine();

                    // Get pattern
                    String pattern = "^";
                    for (Int32 i = 0; i < worldSearh.Length; i++)
                    {
                        if (worldSearh[i] == '*')
                        {
                            pattern += "\\w{1,}";
                        }
                        else if (worldSearh[i] == '?')
                        {
                            pattern += "\\w{1}";
                        }
                        else
                        {
                            pattern += worldSearh[i];
                        }
                    }
                    pattern += "$";

                    // Get words array
                    String text = new StreamReader("text.txt").ReadToEnd();
                    String[] words = text.Split(new Char[] { ' ', '.', ',', '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);

                    // Get part size
                    Double sizePaket = Convert.ToDouble(words.Count()) / Convert.ToDouble(communicator.Size);
                    if (sizePaket > 1)
                    {
                        sizePaket = Math.Floor(sizePaket);
                    }
                    else
                    {
                       sizePaket = Math.Ceiling(sizePaket);
                    }
                    Int32 partSize = Convert.ToInt32(sizePaket);

                    // Send data
                    if (communicator.Size > 1)
                    {
                        Int32 sourceIndex = 0;
                        DateTime timeStart = DateTime.Now;

                        for (int i = 1; i < communicator.Size; i++)
                        {
                            communicator.Send(pattern, i, 0);

                            // Send array size
                            communicator.Send(partSize, i, 0);

                            // Send array words
                            if (sourceIndex < words.Length)
                            {
                                String[] subArray = new String[partSize];
                                Array.Copy(words, sourceIndex, subArray, 0, partSize);
                                sourceIndex += partSize;

                                communicator.Send(subArray, i, 0);
                            }
                        }

                        // Get task for main
                        Int32 summary = words.Length - partSize * (communicator.Size - 1);
                        String[] mainArray = new String[summary];
                        Array.Copy(words, partSize * (communicator.Size - 1), mainArray, 0, summary);

                        Int32 countMatchesMain = 0;
                        Boolean flagThredStop = false;
                        if (summary > 0)
                        {
                            new Thread(new ThreadStart(() =>
                            {
                                foreach (var item in mainArray)
                                {
                                    Regex newReg = new Regex(pattern);
                                    MatchCollection matches = newReg.Matches(item);
                                    foreach (Match mat in matches)
                                    {
                                        countMatchesMain++;
                                        break;
                                    }
                                }

                                flagThredStop = true;
                                Thread.CurrentThread.Abort();
                            })).Start();
                        }

                        // Wait answer
                        Int32 countMatches = 0;
                        Int32 countAnswers = 0;

                        while (true)
                        {
                            Int32 answer;
                            communicator.Receive<Int32>(Communicator.anySource, 0, out answer);
                            countAnswers += 1;

                            if (answer != 0)
                            {
                                countMatches += answer;
                                //Console.WriteLine("Count mathces: {0}", answer);
                                answer = 0;
                            }

                            if (countAnswers == communicator.Size - 1)
                            {
                                break;
                            }
                        }

                        if (summary > 0)
                        {
                            if (!flagThredStop)
                            {
                                while (!flagThredStop){}
                            }
                            countMatches += countMatchesMain;
                        }

                        DateTime timeEnd = DateTime.Now;

                        Console.WriteLine("All mathces: {0}. Summury time: {1}", countMatches, (timeEnd - timeStart).Milliseconds);
                    }
                }
                else
                {
                    // Receive pattern
                    String pattern;
                    communicator.Receive<String>(0, 0, out pattern);
                    //Console.WriteLine("Process Rank: {0}, Search message pattern: {1}", communicator.Rank, pattern);

                    // Receive array size
                    Int32 arraySize;
                    communicator.Receive<Int32>(0, 0, out arraySize);

                    // Receive array
                    String[] words = new String[arraySize];
                    communicator.Receive(0, 0, ref words);

                    //Console.WriteLine("Rank: {0}, Received words count: {1}", communicator.Rank, words.Count());

                    // Search
                    Int32 countMatches = 0;
                    foreach (var item in words)
                    {
                        Regex newReg = new Regex(pattern);
                        MatchCollection matches = newReg.Matches(item);
                        foreach (Match mat in matches)
                        {
                            // Console.WriteLine("Rank: {0}, Value word searhced: {1}", communicator.Rank, mat.Value);
                            countMatches++;
                            break;
                        }
                    }

                    // Send answer
                    communicator.Send(countMatches, 0, 0);
                    //Console.WriteLine("Rank: {0}, Count mathes: {1}", communicator.Rank, countMatches);
                }
            }
        }
    }
}
