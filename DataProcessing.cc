/*
 * Hapl-O-mat: A program for HLA haplotype frequency estimation
 *
 * Copyright (C) 2014-2016, DKMS gGmbH 
 *
 * This file is part of Hapl-O-mat
 *
 * Hapl-O-mat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Hapl-O-mat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hapl-O-mat; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <sstream>
#include <iostream>

#include "DataProcessing.h"
#include "Glid.h"
#include "Report.h"
#include "Utility.h"
#include "Phenotype.h"
#include "Haplotype.h"

void HaplotypeCombinations::findCombinations(const size_t size){

  std::vector<bool> combo (size, false);

  bool done = false;
  while (!done){

    //add combo to list                                                                                                                              
    list.push_back(combo);
    //new combo                                                                                                                                     
    auto it=combo.end();
    it --;
    //can last entry be increased                                                                                                                    
    if(*it == false){
      *it = true;
    }
    //look for entry that can be increased                                                                                                        
    else{
      while(*it == true){
        if(it==combo.begin()){
          done = true;
          break;
	}
        it --;
      }
      *it = true;
      //set all entries larger than it to false                                                                                                  
      for(it = it+1;
          it != combo.end();
          it++)
        *it = false;
    }
  }

  //removed negated combos                                                                                                                     
  // 000 - 111, 001 - 110, 010 - 101, ...                                                                                                       
  //exactly the other half of the vector                                                                                                            
  list.resize(list.size()/2);
}

void HaplotypeCombinations::writeCombinations() const {

  for(auto i1 = list.cbegin();
      i1 != list.cend();
      i1++)
    {
      for(auto i2 = i1->cbegin();
          i2 != i1->cend();
          i2++)
        {
	  std::cout << *i2;
        }
      std::cout << std::endl;
    }
}

void InputFile::buildHaploDiploPhenoTypes(Phenotypes & phenotypes,
				     Haplotypes & haplotypes,
				     const std::shared_ptr<BasicReport> pReport,
				     std::ofstream & haplotypesFile){

  std::string phenotypeCode = pReport->buildPhenotypeCode();
  std::pair<Phenotypes::iterator, bool> inserted = phenotypes.add(phenotypeCode);
  inserted.first->second.addToNumInDonors(pReport->getFrequency());
  if(inserted.second)
    pReport->buildHaploAndDiplotypes(inserted.first, haplotypes, haplotypesFile, haplotypeCombinations);
}

void InputFileToEdit::printPhenotypes(const std::shared_ptr<Report> pReport,
				     const size_t numberReports,
				     std::ofstream & phenotypesFile){         
 
  std::string totalType = pReport->evaluateReportType(numberReports);
  std::string phenotypeCode = pReport->buildPhenotypeCode();
  phenotypesFile << pReport->getId() << "\t"
		 << totalType << "\t"
		 << pReport->getFrequency() << "\t"
		 << phenotypeCode
		 << std::endl;
}

void InputFileToEdit::printStatistics(){

  std::cout << "\t Number loci: " << numberLoci << std::endl;
  std::cout << "\t Removed reports: " << numberRemovedDonors << std::endl;
  std::cout << "\t Leftover Reports: " << numberDonors << std::endl;
  std::cout << "\t H0 reports: " << Report::getNumberH0Reports() << std::endl;
  std::cout << "\t H1 reports: " << Report::getNumberH1Reports() << std::endl;
  std::cout << "\t H2 reports: " << Report::getNumberH2Reports() << std::endl;
  std::cout << "\t H2M reports: " << Report::getNumberH2MReports() << std::endl;
  std::cout << "\t I reports: " << Report::getNumberIReports() <<std::endl;
  std::cout << "\t Phenotypes: " << numberPhenotypes << std::endl;
  std::cout << "\t Haplotypes: " << numberHaplotypes << std::endl;
  std::cout << std::endl;
}

std::vector<bool> GL::buildBooleanLociToDo(){
  
  std::vector<bool> booleanLociToDoOut;
  for(auto locus : lociToDo){
    if(locus == "NONE"){
      booleanLociToDoOut.push_back(false);
    }
    else{
      booleanLociToDoOut.push_back(true);
      numberLoci ++;
    }
  }
  return booleanLociToDoOut;
}

strVec_t GL::updateLociToDoViaPullFile() const{

  std::vector<bool> buildAllGenotypesForLocus(lociToDo.size(), false);

  std::ifstream inputFile;
  openFileToRead(inputFileName, inputFile);

  std::string line;
  while(std::getline(inputFile, line)){
    if(line.length() == 1 || line.length() == 0)
      continue;
    strVec_t splittedLine = split(line, ';');
    strVec_t glids = split(splittedLine.at(1), ':');

    auto it_lociToDo = lociToDo.cbegin();
    auto it_buildAllGenotypesForLocus = buildAllGenotypesForLocus.begin();
    for(auto glid : glids){
      if(glid == "0" && *it_lociToDo != "NONE")
	*it_buildAllGenotypesForLocus = true;
      it_buildAllGenotypesForLocus ++;
      it_lociToDo ++;
    }
  }

  strVec_t lociToDoOut;
  lociToDoOut = lociToDo;
  auto it_lociToDo = lociToDoOut.begin();
  for(auto it_buildAllGenotypesForLocus = buildAllGenotypesForLocus.begin();
      it_buildAllGenotypesForLocus != buildAllGenotypesForLocus.end();
      it_buildAllGenotypesForLocus ++){
    if(! *it_buildAllGenotypesForLocus)
      *it_lociToDo = "NONE";
    it_lociToDo ++;
  }

  return lociToDoOut;
}

void GL::dataProcessing(Phenotypes & phenotypes, Haplotypes & haplotypes){
  
  std::ifstream inputFile;
  openFileToRead(inputFileName, inputFile);
  std::ofstream haplotypesFile;
  openFileToWrite(haplotypesFileName, haplotypesFile);
  std::ofstream phenotypesFile;
  openFileToWrite(phenotypesFileName, phenotypesFile);
  phenotypesFile.precision(14);

  haplotypeCombinations.findCombinations(numberLoci);

  std::string line;
  while(std::getline(inputFile, line)){

    if(line.length() == 1 || line.length() == 0)
      continue;

    GLReport report(line, booleanLociToDo, numberLoci, wantedPrecision);
    std::vector<std::shared_ptr<Report>> listOfpReports;
    report.resolve(listOfpReports, glid, minimalFrequency, resolveUnknownGenotype);

    if(listOfpReports.empty())
      numberRemovedDonors ++;
    else{
      numberDonors ++;
      for(auto oneReport : listOfpReports){
	printPhenotypes(oneReport, listOfpReports.size(), phenotypesFile);
	buildHaploDiploPhenoTypes(phenotypes, haplotypes, oneReport, haplotypesFile);
      }
    }
  }//while

  inputFile.close();
  haplotypes.setNumberLoci(numberLoci);
  haplotypes.setNumberDonors(numberDonors);
  numberHaplotypes = haplotypes.getSize();
  numberPhenotypes = phenotypes.getSize();
}

void MA::dataProcessing(Phenotypes & phenotypes, Haplotypes & haplotypes){

  std::ifstream inputFile;
  openFileToRead(inputFileName, inputFile);
  std::ofstream haplotypesFile;
  openFileToWrite(haplotypesFileName, haplotypesFile);
  std::ofstream phenotypesFile;
  openFileToWrite(phenotypesFileName, phenotypesFile);
  phenotypesFile.precision(14);

  std::string line;
  if(std::getline(inputFile, line))
    readLociNames(line);

  haplotypeCombinations.findCombinations(numberLoci);

  while(std::getline(inputFile, line)){

    if(line.length() == 1 || line.length() == 0)
      continue;

    HReport report(line, lociNames, numberLoci, wantedPrecision);
    std::vector<std::shared_ptr<Report>> listOfpReports;
    report.resolve(listOfpReports, minimalFrequency, doH2Filter, expandH2Lines);

    if(listOfpReports.empty())
      numberRemovedDonors ++;
    else{
      numberDonors ++;
      for(auto oneReport : listOfpReports){
	printPhenotypes(oneReport, listOfpReports.size(), phenotypesFile);
	buildHaploDiploPhenoTypes(phenotypes, haplotypes, oneReport, haplotypesFile);
      }
    }
  }//while
    
  inputFile.close();
  haplotypesFile.close();
  phenotypesFile.close();

  haplotypes.setNumberLoci(numberLoci);
  haplotypes.setNumberDonors(numberDonors);
  numberHaplotypes = haplotypes.getSize();
  numberPhenotypes = phenotypes.getSize();
}

void MA::readLociNames(const std::string line){

  std::stringstream ss(line);
  std::string name;
  if (ss >> name){
    while(ss >> name){
      lociNames.push_back(name);
      numberLoci ++;
    }
  }
  numberLoci ++;
  numberLoci /= 2;
}

void InputFileToRead::dataProcessing(Phenotypes & phenotypes, Haplotypes & haplotypes){

  std::ifstream inputFile;
  openFileToRead(inputFileName, inputFile);
  std::ofstream haplotypesFile;
  openFileToWrite(haplotypesFileName, haplotypesFile);

  std::string line;
  if(std::getline(inputFile, line))
    countNumberLoci(line);
  inputFile.clear();
  inputFile.seekg(0, inputFile.beg);

  haplotypeCombinations.findCombinations(numberLoci);

  double decimalNumberDonors = 0.;
  while(std::getline(inputFile, line)){

    if(line.length() == 1 || line.length() == 0)
      continue;

    std::shared_ptr<BasicReport> pReport = std::make_shared<ReadinReport> (line, numberLoci);
    decimalNumberDonors += pReport->getFrequency();

    buildHaploDiploPhenoTypes(phenotypes, haplotypes, pReport, haplotypesFile);
  }//while
    
  inputFile.close();
  haplotypesFile.close();

  numberDonors = static_cast<size_t>(round(decimalNumberDonors));
  haplotypes.setNumberLoci(numberLoci);
  haplotypes.setNumberDonors(numberDonors);
  numberHaplotypes = haplotypes.getSize();
  numberPhenotypes = phenotypes.getSize();
}

void InputFileToRead::printStatistics(){

  std::cout << "\t Number loci: " << numberLoci << std::endl;
  std::cout << "\t Number Reports: " << numberDonors << std::endl;
  std::cout << "\t Phenotypes: " << numberPhenotypes << std::endl;
  std::cout << "\t Haplotypes: " << numberHaplotypes << std::endl;
  std::cout << std::endl;
}

void InputFileToRead::countNumberLoci(const std::string line){

  strVec_t entries = split(line, '\t');
  std::string phenotype = entries[3];
  double numberAsteriks = std::count(phenotype.cbegin(),
			      phenotype.cend(),
			      '*');
  numberLoci = static_cast<size_t>((numberAsteriks + 1.) / 2);
}

