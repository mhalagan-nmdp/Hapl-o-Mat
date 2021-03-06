/*
 * Hapl-o-Mat: A software for haplotype inference
 *
 * Copyright (C) 2016, DKMS gGmbH 
 *
 * Christian Schaefer
 * Kressbach 1
 * 72072 Tuebingen, Germany
 *
 * T +49 7071 943-2063
 * F +49 7071 943-2090
 * cschaefer(at)dkms.de
 *
 * This file is part of Hapl-o-Mat
 *
 * Hapl-o-Mat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Hapl-o-Mat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hapl-o-Mat; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <iostream> 

#include "Haplotype.h"
#include "Phenotype.h"
#include "Utility.h"

std::size_t Haplotypes::computeSizeInBytes(){

  size_t sizeInBytes = 0;
  for(auto haplo : hashList){
    sizeInBytes += sizeof(haplo.first);
    sizeInBytes += sizeof(haplo.second);
  }
  sizeInBytes += sizeof(hashList);
  
  return sizeInBytes;
}

void Haplotypes::EMAlgorithm(Phenotypes & phenotypes){
 
  std::ofstream epsilonFile;
  openFileToWrite(epsilonFileName, epsilonFile);
  epsilonFile.precision(14);
  epsilonFile << std::scientific;

  bool stop;
  size_t counter = 0;
  do{
    counter ++;
    stop = false;

    phenotypes.expectationStep(*this);
    double largestEpsilon = 0.;
    maximizationStep(phenotypes, largestEpsilon);

    phenotypes.expectationStep(*this);
    double logLikelihood = phenotypes.computeLogLikelihood();

    if(largestEpsilon - epsilon < ZERO ){
      stop = true;
    }
    epsilonFile << largestEpsilon << "\t" << logLikelihood << std::endl;
  } while(!stop);
  std::cout << "\t Used " << counter <<" steps" << std::endl;
  epsilonFile.close();
}

void Haplotypes::initialiseFrequencies(const Phenotypes & phenotypes){
  
  switch(initType){
  case Parameters::initialisationHaplotypeFrequencies::numberOccurrence:
    {
      initialiseNumberOccurrence(phenotypes);
      break;
    }
  case Parameters::initialisationHaplotypeFrequencies::perturbation:
    {
      initialiseNumberOccurrence(phenotypes);
      initialisePerturbation();
      break;
    }
  case Parameters::random:
    {
      double normalisation = 0.;
      for(auto it = listBegin();
	  it != listEnd();
	  it ++){
	double number = rng() + 1.;
	normalisation += number;
	it->second.setFrequency(number);
      }
      for(auto it = listBegin();
	  it != listEnd();
	  it ++){
	double newFreq = it->second.getFrequency()/normalisation;
	it->second.setFrequency(newFreq);
      }
      break;
    }
  case Parameters::equal:
    { 
      double factor = 1. / static_cast<double>(hashList.size());
      for(auto haplo = listBegin();
	  haplo != listEnd();
	  haplo ++){
	haplo->second.setFrequency(factor);
      }
    }
  }//switch
}

void Haplotypes::initialiseNumberOccurrence(const Phenotypes & phenotypes){

  double factor = static_cast<double>(pow(2, numberLoci));
  factor *= static_cast<double>(numberDonors);

  auto itPhenoEnd = phenotypes.c_listEnd();
  for(auto itPheno = phenotypes.c_listBegin();
      itPheno != itPhenoEnd;
      itPheno++)
    {
      double numberPhenotype = itPheno->second.getNumInDonors() / factor;
      auto itDiploEnd = itPheno->second.c_diplotypeListEnd();
      for(auto itDiplo = itPheno->second.c_diplotypeListBegin();
          itDiplo != itDiploEnd;
          itDiplo ++)
        {
	  auto itHaplo1 = hashList.find(itDiplo->id1);
	  auto itHaplo2 = hashList.find(itDiplo->id2);
	  itHaplo1->second.addFrequency(numberPhenotype);
	  itHaplo2->second.addFrequency(numberPhenotype);
	}
    }
}

void Haplotypes::initialisePerturbation(){

  std::uniform_real_distribution<> dis(0, 1);
  double sum = 0.;
  for(auto it = listBegin();
      it != listEnd();
      it ++){
    double oldFreq = it->second.getFrequency();
    double newFreq = oldFreq + dis(rng) * oldFreq / 10.;

    it->second.addFrequency(newFreq);
    sum += newFreq;
  }

  double normalisation = 1./sum;
  for(auto it = listBegin();
      it != listEnd();
      it ++){
    it->second.multiplyFrequency(normalisation);
  }
}

void Haplotypes::writeFrequenciesToFile() const{

  double sum = computeCuttedHaplotypeFrequencySum();

  std::vector<std::pair<std::string, double>> sortedHaplotypes;

  std::ifstream inFile;
  openFileToRead(haplotypesFileName, inFile);
  std::string code;
  while(inFile >> code){
    size_t hashValue = string_hash(code);
    auto pos = hashList.find(hashValue);

    if(pos != hashList.end()){
      double freq = pos->second.getFrequency();
      if(freq - cutHaplotypeFrequencies > ZERO ){
	if(renormaliseHaplotypeFrequencies){
	  freq /= sum;
	}
	sortedHaplotypes.push_back(std::make_pair(code, freq));
      }
    }
  }
  inFile.close();

  std::sort(sortedHaplotypes.begin(),
	    sortedHaplotypes.end(),
	    [](const std::pair<std::string, double> haploA,
	       const std::pair<std::string, double> haploB)
	    {
	      return haploA.second > haploB.second;
	    });

  std::ofstream outFile;
  openFileToWrite(haplotypeFrequenciesFileName, outFile);
  outFile.precision(14);
  outFile << std::fixed;

  for(auto it : sortedHaplotypes){
    outFile << it.first
	    << "\t";
    outFile << it.second
	    << "\n";
  }

  outFile.close();
}

void Haplotypes::writeFrequenciesAndErrorsToFile(const std::vector<double> errors) const{

  std::ifstream inFile;
  openFileToRead(haplotypesFileName ,inFile);
  std::ofstream outFile;
  openFileToWrite(haplotypeFrequenciesFileName ,outFile);
  outFile.precision(14);
  outFile << std::fixed;

  std::string code;
  while(inFile >> code){
    size_t hashValue = string_hash(code);
    auto pos = hashList.find(hashValue);
    if(pos != hashList.end()){
      double freq = pos->second.getFrequency();
      size_t positionError = distance(hashList.cbegin(), pos);
      double error = errors.at(positionError);
      if(freq - epsilon > ZERO ){
	outFile << code	<< "\t" << freq << "\t" << error << "\n";
      }
    }
  }
  inFile.close();
  outFile.close();
}


void Haplotypes::maximizationStep(const Phenotypes & phenotypes, double & largestEpsilon){

    //save old frequencies, initialize haplotype frequencies to zero
    std::vector<double> oldFrequencies;
    oldFrequencies.reserve(hashList.size());
    auto itEnd = hashList.end();
    for(auto it = hashList.begin();
	it != itEnd;
	it++){
      oldFrequencies.push_back(it->second.getFrequency());
      it->second.setFrequency(0.);
    }

    maximization(phenotypes);

    auto itOld = oldFrequencies.cbegin();
    for(auto it = hashList.begin();
	it != hashList.end();
	it++, itOld ++){
      it->second.multiplyFrequency(.5 / static_cast<double>(numberDonors));
      
      double possibleEpsilon = fabs(it->second.getFrequency() - *itOld);
      if(possibleEpsilon - largestEpsilon > ZERO ){
	largestEpsilon = possibleEpsilon; 
      }
    }

    auto it = hashList.begin();
    while(it != hashList.end()){
      if(it->second.getFrequency() < ZERO){
	it = hashList.erase(it);
      }
      else{
	it ++;
      }
    }
}

void Haplotypes::maximization(const Phenotypes & phenotypes){

  auto itPhenoEnd = phenotypes.c_listEnd();
  for(auto itPheno = phenotypes.c_listBegin();
      itPheno != itPhenoEnd;
      ++ itPheno){

    double factor = itPheno->second.getNumInDonors() / itPheno->second.computeSummedFrequencyDiplotypes();
		
    auto itDiploEnd = itPheno->second.c_diplotypeListEnd();
    for(auto itDiplo = itPheno->second.c_diplotypeListBegin();
	itDiplo != itDiploEnd;
	++ itDiplo)
      { 
	double freq = factor * itDiplo->frequency;
	auto itHaplo1 = hashList.find(itDiplo->id1);
	if(itHaplo1 != hashList.end()){
	  itHaplo1->second.addFrequency(freq);
	}
	if(itDiplo->id1 == itDiplo->id2){
	  itHaplo1->second.addFrequency(freq);
	}
	else{
	  auto itHaplo2 = hashList.find(itDiplo->id2);
	  if(itHaplo2 != hashList.end()){
	    itHaplo2->second.addFrequency(freq);
	  }
	}
      }//for diplo
  }
}

double Haplotypes::computeHaplotypeFrequencySum() const{

  double frequencySum = 0.;
  for(auto haplotype = hashList.begin();
      haplotype != hashList.end();
      haplotype ++){
    frequencySum += haplotype->second.getFrequency();
  }
  return frequencySum;
}

double Haplotypes::computeCuttedHaplotypeFrequencySum() const{

  double frequencySum = 0.;
  for(auto haplotype = hashList.begin();
      haplotype != hashList.end();
      haplotype ++){
    double frequency = haplotype->second.getFrequency();
    if(frequency - cutHaplotypeFrequencies > ZERO ){ 
      frequencySum += frequency;
    }
  }
  return frequencySum;

}

void Haplotypes::deleteHaplotypesFile() const{

  if (remove(haplotypesFileName.c_str()) != 0){
    std::cerr << "Not able to delete " << haplotypesFileName << std::endl;
  }
}
