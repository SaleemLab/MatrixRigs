%% generate LUT mapping reward weight to open times

filename = 'LPCAL_Rig_TrinityTest_Lickport_l_2023-06-20T17_49_14.csv';

valueRange = []; % in mg, default is range collected from scales
resolution = 0.1; % in mg
plotFlag=true;

%% load data

opts = delimitedTextImportOptions("NumVariables", 3);

% Specify range and delimiter
opts.DataLines = [2 inf];
opts.Delimiter = ",";

% Specify column names and types
opts.VariableNames = ["OpenTime", "ScaleReading", "nReps"];
opts.VariableTypes = ["double", "double", "double"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Import the data
tbl = readtable(filename, opts);


%% get mean weight for each open time

tbl.TotalWeight = diff(cat(1,0,tbl.ScaleReading));
tbl.MeanWeight = (tbl.TotalWeight./tbl.nReps)*1000; % convert to mg

%% interpolate values

% get the range to interpolate over
if isempty(valueRange)
    valueRange = [min(tbl.MeanWeight), max(tbl.MeanWeight)];
end

% round the value range according to the resolution setting
expstr = @(x) [x*10.^floor(1-log10(abs(x)))  floor(log10(abs(x)))];
b = expstr(resolution);
roundArg = b(2)*-1;
valueRange=round(valueRange,roundArg);

% do the actual interpolation
x = tbl.MeanWeight;
v = tbl.OpenTime;
xq = valueRange(1):resolution:valueRange(2);
vq = interp1(x,v,xq, 'linear','extrap');

%% optional plot

if plotFlag
    figure, hold on
    plot(x,v,'ro')
    plot(xq,vq,'k.')
    legend({'original values', 'interpolated values'},'location','northwest')
    xlabel('weight (mg)'), ylabel('open time (ms)')
    xlim([valueRange(1)-resolution, valueRange(2)+resolution])
    ylim([min(vq)-5, max(vq)+5])
end

%% create output file
out_tbl=table;
out_tbl.OpenTime = vq(:); 
out_tbl.Weight = xq(:);

outputFilename = ['Bonsai_', filename];

writetable(out_tbl,outputFilename,'WriteVariableNames',0)


