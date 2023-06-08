using Bonsai;
using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;

[Combinator]
[Description("")]
[WorkflowElementCategory(ElementCategory.Transform)]
public class FindListIndex
{
    private int output;

    public IObservable<int> Process(IObservable<Tuple<IList<float>, float>> source)
    {
        return source.Select(value => 
        {
            IList<float> theList = value.Item1;
            float theValue = value.Item2;
            int listLength = theList.Count();

            for (int i=0; i<listLength; i++)
            {
                if (theList[i]==theValue)
                {
                    output = i;
                }
            }

            return output;
        });
    }
}
