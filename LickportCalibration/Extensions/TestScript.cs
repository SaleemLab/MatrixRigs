using Bonsai;
using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;

[Combinator]
[Description("")]
[WorkflowElementCategory(ElementCategory.Transform)]
public class TestScript
{
    public IObservable<Tuple<List<float>,List<float>>> Process(IObservable<int> source)
    {
        return source.Select(value => 
        {
            List<float> output = new List<float>();
            output.Add((float)value);

            for (int i=0; i<10; i++)
            {
                output.Add((float)i);
            }

            List<float> output2 = new List<float>();
            output2.Add((float)value);

            var result = new Tuple<List<float>,List<float>>(output, output2);
            return result;

        });
    }
}
