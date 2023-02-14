using Bonsai;
using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;

[Combinator]
[Description("")]
[WorkflowElementCategory(ElementCategory.Source)]
public class CreateList
{

    public List<float> ThisList { get; set; }

    public IObservable<List<float>> Process()
    {
        List<float> output = ThisList;
        return Observable.Return(output);
    }
}
