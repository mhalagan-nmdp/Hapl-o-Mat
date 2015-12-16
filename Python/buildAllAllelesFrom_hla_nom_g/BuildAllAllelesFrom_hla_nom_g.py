#Christian Schaefer
#Dec 2015
#Extract list of all alleles from hla_nom_g.txt. Thus still confidential alleles are not included in list.

alleles = []
with open('hla_nom_g.txt') as file:
    for line in file:
        if not line.startswith('#'):
            line = line.rstrip('\r\n')
            splittedLine = line.split(';')
            locus = splittedLine[0]
            allelesFromFile = splittedLine[1].split('/')
            codeG = splittedLine[2]

            for alleleFromFile in allelesFromFile:
                allele = locus + alleleFromFile
                alleles.append(allele)

with open('allAlleles.txt', 'w') as out:
    for allele in alleles:
        out.write(allele + '\n')

