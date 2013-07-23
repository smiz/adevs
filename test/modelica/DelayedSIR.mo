model Influenza
  // Parameters; time is in days
  parameter Real MortalityProb = 0.01;
  parameter Real RecoveryTime = 3.0;
  parameter Real MortalityTime = 1.0;
  parameter Real TransmissionProb = 0.15;
  parameter Real EncounterRate = 4.0;
  parameter Real TotalPop = 500000;
  parameter Real InitialInfectious = 1000;
  // In days
  // Start variables
  Real Deceased(start = 0);
  Real Recovered(start = 0);
  Real Infectious(start = InitialInfectious);
  Real Susceptible(start = TotalPop-InitialInfectious);
  Real LivingPopulation;
  Real toRecover(start=0);
  Real toDie(start=0);
  Real R;
  Boolean PastRecovery(start=false);
  Boolean PastMortality(start=false);
equation
  LivingPopulation = Recovered + Infectious + Susceptible;
  R = (TransmissionProb * EncounterRate * Susceptible) / LivingPopulation;
  PastRecovery = if time > RecoveryTime then true else false;
  PastMortality = if time > MortalityTime then true else false;
  toRecover = if edge(PastRecovery) then InitialInfectious*(1-MortalityProb) else 0;
  toDie = if edge(PastMortality) then InitialInfectious*MortalityProb else 0;
  when PastRecovery then
     reinit(Recovered,InitialInfectious*(1-MortalityProb));
     reinit(Infectious,pre(Infectious)-(toRecover+toDie));
  end when;
  when PastMortality then
     reinit(Deceased,InitialInfectious*MortalityProb);
     reinit(Infectious,pre(Infectious)-(toRecover+toDie));
  end when;
  der(Recovered) = if PastRecovery then
     (1-MortalityProb)*delay(-der(Susceptible),RecoveryTime)
     else 0;
  der(Deceased) = if PastMortality then
     MortalityProb*delay(-der(Susceptible),MortalityTime)
     else 0;
  der(Susceptible) = -R * Infectious;
  der(Infectious) = -(der(Susceptible)+der(Recovered)+der(Deceased));
end Influenza;

