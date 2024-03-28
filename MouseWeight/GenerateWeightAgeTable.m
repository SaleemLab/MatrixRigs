%% Tomaso Muzzu - UCL - 08/09/2017

%% Plot weight-age curve for the mice.

% Original curve based on the data taken from Jackson Lab website: https://www.jax.org/jax-mice-and-services/strain-data-sheet-pages/body-weight-chart-000664#

% The rationale is to weigh an animal at a certain age and compute a curve parallel to the original that passes through this point.

function GenerateWeightAgeTable % weight in grammes, age in weeks, sex is either M or F
%% request info of the animal
confirm = 'N';
while sum(strncmp(confirm,'y',1) || strncmp(confirm,'y',1))==0
    weight_IN = input('Weight of the mouse (in grammes): ');
    confirm = input('Do you confirm your entry?','s');
end
confirm = 'N';
while sum(strncmp(confirm,'y',1) || strncmp(confirm,'y',1))==0
    age_IN = input('Age of the mouse when weight was taken (in weeks): ');
    confirm = input('Do you confirm your entry?','s');
end
confirm = 'N';
while sum(strncmp(confirm,'y',1) || strncmp(confirm,'y',1))==0
    sex = input('Is it a Male (0) or Female (1)?');
    confirm = input('Do you confirm your entry?','s');
end
confirm = 'N';
while sum(strncmp(confirm,'y',1) || strncmp(confirm,'y',1))==0
    mousename = input('Insert name of the mouse please: ', 's');
    confirm = input('Do you confirm your entry?','s');
end

%% Original data for reference weights
W_ref_M(:,1) = 3:20; % male mice
W_ref_M(:,2) = [9.7 15.7 19.4 21.1 22.9 24 25 25.6 26.7 27.7 28.4 29.1 29.7 30.1 30.7 31.1 31.4 31.8];
W_ref_M(:,3) = [1.9 2.2 1.8 1.5 1.5 1.5 1.6 1.7 1.7 1.7 1.9 1.9 2.2 2.1 2.2 2.3 2.4 2.5];

W_ref_F(:,1) = 3:20; % female mice
W_ref_F(:,2) = [9.3 14.1 16.9 17.5 18.2 18.7 19.3 19.8 20.2 20.7 21.7 22 22.3 22.6 22.6 23 23.5 23.6];
W_ref_F(:,3) = [1.7 1.8 1.2 1 1.1 1.2 1.2 1.3 1.4 1.4 1.5 1.6 1.7 1.8 2 2.1 2.3 2.3];

%% compute curve for the new animal; as input takes weight and age at weight recording

% adjust curve
switch sex
    case 0
        W_fit(:,1) = linspace(2,52,52*7); % create time vector up to 52 weeks with daily resolution
        [fitresult, gof] = CreateFit(W_ref_M(:,1), W_ref_M(:,2)); % compute fitting curve with parameters obtained from Matlab "Curve Fitting" app
        W_fit(:,2) = fitresult.a*(W_fit(:,1).^fitresult.b)+fitresult.c;
        % fit upper limit of std 
        [fitresult_up, gof] = createFit_posSTD(W_ref_M(:,1), W_ref_M(:,2)+W_ref_M(:,3)); % compute fitting curve with parameters obtained from Matlab "Curve Fitting" app
        W_fit(:,3) = fitresult_up.a*(W_fit(:,1).^fitresult_up.b)+fitresult_up.c;
        % fit bottom limit of std 
        [fitresult_down, gof] = createFit_negSTD(W_ref_M(:,1), W_ref_M(:,2)-W_ref_M(:,3)); % compute fitting curve with parameters obtained from Matlab "Curve Fitting" app
        W_fit(:,4) = fitresult_down.a*(W_fit(:,1).^fitresult_down.b)+fitresult_down.c;
        % compute z score
        [V,ind] = min(abs(W_fit(:,1)-age_IN));      
        Zscore = (weight_IN-W_fit(ind,2)) / ((abs(W_fit(ind,4)-W_fit(ind,2))+abs(W_fit(ind,3)-W_fit(ind,2)))/2);
        [v,ind] = min(abs(W_fit(:,1)-age_IN));
        CurveAdj = Zscore*((abs(W_fit(:,4)-W_fit(:,2))+abs(W_fit(:,3)-W_fit(:,2)))/2) + W_fit(:,2); % curve z-score-adjusted for the mouse of interest
        figure
        set(gcf,'units','pixels','position',[100,100,1200,600])
        h1 = plot(W_fit(:,1),CurveAdj,'--k','MarkerSize',12, 'LineWidth',3 ); % perspective mouse weight
        hold on
        h3 = plot(W_fit(:,1),CurveAdj*0.8,'--r','MarkerSize',12, 'LineWidth',3 ); % 0.8 is threshold for weight reduction - end point
        hold on
        h2 = plot(W_fit(:,1),CurveAdj*0.85,'--','MarkerSize',4, 'LineWidth',1,'Color', [255/255 ,165/255 ,0/255] ); % 0.85 is threshold for weight reduction - be careful
        hold on
        h4 = plot(age_IN, weight_IN,'ok','MarkerSize',10, 'LineWidth',3)
        title('mouse weight reference table'); xlabel('age - weeks'); ylabel('weight - grammes');
        axis([0 53 0 40]); set(gca,'TickDir','out','box','off'); grid on
        legend([h1 h2 h3 h4],{['extrapolated weight up to 52 weeks for ' mousename],...
                            '85% weight: WATCH OUT', ...
                            'minimum allowed weight: END POINT', ...
                            'original weight recording'},'Location','southeast')
        % save data onto txt file to be copied on Google sheet
        fileID = fopen([cd '\MouseData\' mousename '_weightVSage' '.txt'],'w');
        fprintf(fileID, '%10s %12s \r\n','age (weeks)','weight (g)');
        fprintf(fileID, '%6.5f \t %6.5f \r\n',[W_fit(:,1), CurveAdj]');
        fclose(fileID);
    case 1
        W_fit(:,1) = linspace(2,52,52*7); % create time vector up to 52 weeks with daily resolution
        [fitresult, gof] = CreateFit(W_ref_F(:,1), W_ref_F(:,2)); % compute fitting curve with parameters obtained from Matlab "Curve Fitting" app
        W_fit(:,2) = fitresult.a*(W_fit(:,1).^fitresult.b)+fitresult.c;
        % fit upper limit of std 
        [fitresult_up, gof] = createFit_posSTD(W_ref_F(:,1), W_ref_F(:,2)+W_ref_F(:,3)); % compute fitting curve with parameters obtained from Matlab "Curve Fitting" app
        W_fit(:,3) = fitresult_up.a*(W_fit(:,1).^fitresult_up.b)+fitresult_up.c;
        % fit bottom limit of std 
        [fitresult_down, gof] = createFit_negSTD(W_ref_F(:,1), W_ref_F(:,2)-W_ref_F(:,3)); % compute fitting curve with parameters obtained from Matlab "Curve Fitting" app
        W_fit(:,4) = fitresult_down.a*(W_fit(:,1).^fitresult_down.b)+fitresult_down.c;
        % compute z score
        [V,ind] = min(abs(W_fit(:,1)-age_IN));      
        Zscore = (weight_IN-W_fit(ind,2)) / ((abs(W_fit(ind,4)-W_fit(ind,2))+abs(W_fit(ind,3)-W_fit(ind,2)))/2);
        [v,ind] = min(abs(W_fit(:,1)-age_IN));
        CurveAdj = Zscore*((abs(W_fit(:,4)-W_fit(:,2))+abs(W_fit(:,3)-W_fit(:,2)))/2) + W_fit(:,2); % curve z-score-adjusted for the mouse of interest
        figure
        set(gcf,'units','pixels','position',[100,100,1200,600])
        h1 = plot(W_fit(:,1),CurveAdj,'--k','MarkerSize',12, 'LineWidth',3 ); % perspective mouse weight
        hold on
        h3 = plot(W_fit(:,1),CurveAdj*0.8,'--r','MarkerSize',12, 'LineWidth',3 ); % 0.8 is threshold for weight reduction - end point
        hold on
        h2 = plot(W_fit(:,1),CurveAdj*0.85,'--','MarkerSize',4, 'LineWidth',1,'Color', [255/255 ,165/255 ,0/255] ); % 0.85 is threshold for weight reduction - be careful
        hold on
        h4 = plot(age_IN, weight_IN,'ok','MarkerSize',10, 'LineWidth',3)
        title('mouse weight reference table'); xlabel('age - weeks'); ylabel('weight - grammes');
        axis([0 53 0 40]); set(gca,'TickDir','out','box','off'); grid on
        legend([h1 h2 h3 h4],{['extrapolated weight up to 52 weeks for ' mousename],...
                            '85% weight: WATCH OUT', ...
                            'minimum allowed weight: END POINT', ...
                            'original weight recording'},'Location','southeast')
        % save data onto txt file to be copied on Google sheet
        fileID = fopen([cd '\MouseData\' mousename '_weightVSage' '.txt'],'w');
        fprintf(fileID, '%10s %12s \r\n','age (weeks)','weight (g)');
        fprintf(fileID, '%6.5f \t %6.5f \r\n',[W_fit(:,1), CurveAdj]');
        fclose(fileID);
end
SaveFile = [cd '\MouseData\' mousename '_weightVSage'];
screen2jpeg(SaveFile)
end