using Bonsai;
using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;

[Combinator]
[Description("")]
[WorkflowElementCategory(ElementCategory.Transform)]
public class UpdateProbDistArray
{
    public float scalingFactor {get; set;}

    public IObservable<List<float[]>> Process(IObservable<Tuple<IList<int[]>, IList<float[]>>> source)
    {
    
        return source.Select(value =>
        {
            List<int[]> TrialCounters = new List<int[]>(value.Item1);
            List<float[]> OriginalProbDistArray = new List<float[]>(value.Item2);
            float[] tempDist = OriginalProbDistArray[0];
            int nStim = tempDist.Length;
            int nStates = OriginalProbDistArray.Count();
            
            List<float[]> newProbDistArray = new List<float[]>(value.Item2);

          //  for (int istim=0; istim<nStim; istim++)
           //     {
           //         Console.WriteLine("targetDist" + istim + ": " + OriginalProbDistArray[0][istim]);
             //   }

            // output 
            List<float[]> output = new List<float[]>();

            
            for (int istate=0; istate<nStates; istate++)
            {
                float[] outputDist = new float[nStim];

                float[] targetDist = OriginalProbDistArray[istate];
                int[] tempCounters = TrialCounters[istate];
                var theseCounters = tempCounters.Select(x=>(float)x).ToArray();  // convert to float[]
            


                // get multiplication factor (normalsie to sum=100)
                float totalCounts = theseCounters.Sum();
                if (totalCounts<1)
                {
                    totalCounts=100;
                }
                float multFactor = 100f/totalCounts;
                float[] normCounters = theseCounters.Select(r=> r * multFactor).ToArray();
                float normSum = normCounters.Sum();
                //Console.WriteLine("sum check " + normSum);

            
                float[] diffArray = new float[nStim];




                for (int istim=0; istim<nStim; istim++)
                {
                    diffArray[istim] = targetDist[istim]-normCounters[istim]; // if less than wanted then +ve, if more than wanted then -ve
                    outputDist[istim] = targetDist[istim]+(diffArray[istim]*scalingFactor);
                    if (outputDist[istim]<0) {outputDist[istim]=0;};
                    //if (outputDist[istim]>=100) {outputDist[istim]=90;};
                    //newProbDistArray[istate][istim] = OriginalProbDistArray[istate][istim]+diffArray[istim];
                }

                //Console.WriteLine("diffArray:");                
                //Console.WriteLine("[{0}]", string.Join(", ", diffArray));



               // Console.WriteLine("temp output dist: ");
                //Console.WriteLine("[{0}]", string.Join(", ", outputDist));


               // normalise arrays back to 100
            
                float temp_sum = outputDist.Sum();
                float multFactor2 = 100f/temp_sum;
                outputDist = outputDist.Select(r=> r * multFactor2).ToArray();
                //if (istate==0)
                //{   
                //    Console.WriteLine("output dist:");
                //    Console.WriteLine("[{0}]", string.Join(", ", outputDist));
               // }
                //for (int istim=0; istim<nStim; istim++)
                //{Console.WriteLine("op: " + outputDist[istim]);};
                //float normSum = normDist.Sum();
                //Console.WriteLine("sum check " + normSum)

                if (outputDist.Any(float.IsNaN))
                {
                    outputDist = targetDist;
                }

                output.Add(outputDist);
            }


        

                    

                    //diffArray[istim] = normCounters[istim]-targetDist[istim];
        
                    
       //             if (newProbDistArray[istate][istim]<0)
        //            {
         //               newProbDistArray[istate][istim]=0;
          //          }

            //        if (newProbDistArray[istate][istim]>=100)
             //       {
              //          newProbDistArray[istate][istim]=90;
             //       }
             //       if (istate==0)
             //       {
                        //Console.WriteLine("state: " + istate + ", stim: " + istim + ", diffArray: " + diffArray[istim]);
                        //Console.WriteLine("state: " + istate + ", stim: " + istim + ", trialCount: " + tempCounters[istim]);
                        //Console.WriteLine("state: " + istate + ", stim: " + istim + ", targetDist: " + targetDist[istim]);
            //        }
           //     }

           // }
            
            // normalise arrays back to 100
         //   for (int istate=0; istate<nStates; istate++)
        //    {
        //        float[] targetDist = newProbDistArray[istate];
        //        float temp_sum = targetDist.Sum();
        //        float multFactor = 100f/temp_sum;
        //        float[] normDist = targetDist.Select(r=> r * multFactor).ToArray();
        //        newProbDistArray[istate] = normDist;
                //float normSum = normDist.Sum();
                //Console.WriteLine("sum check " + normSum);


         //   }

            return output;

        }
        );
    }
}
