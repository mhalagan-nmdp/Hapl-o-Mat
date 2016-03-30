
#ifndef Exceptions_header
#define Exceptions_header

#include <exception>
#include <string>

class MultipleAlleleCodeException : public std::exception{

 public:
  explicit MultipleAlleleCodeException(const std::string in_multipleAlleleCode)
    : exception(),
    errorMessage("Multiple allele code " + in_multipleAlleleCode + " not found.")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class MissingAlleleException : public std::exception{

 public:
  explicit MissingAlleleException(const std::string in_missingAllele,
				  const std::string in_fileName)
    : exception(),
    errorMessage("Allele " + in_missingAllele + " does not exist in " + in_fileName + ".")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class AlleleResolutionException : public std::exception{

 public:
  explicit AlleleResolutionException(const std::string in_allele)
    : exception(),
    errorMessage("Resolution of " + in_allele + " not known.")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class ResolutionException : public std::exception{


 public:
  explicit ResolutionException(const std::string in_wantedResolution, const std::string in_locus)
    : exception(),
    errorMessage("Resolution " + in_wantedResolution + " specified for locus " + in_locus + " not known.")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class ParameterAssignmentException : public std::exception{

 public:
  explicit ParameterAssignmentException(const std::string in_line)
    : exception(),
    errorMessage("Wrong format for assignment of " + in_line + ".")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class InputFormatException : public std::exception{

 public:
  explicit InputFormatException()
    : exception(),
    errorMessage("Wrong input format (MA, GLC, GL, READ).")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class FileException : public std::exception{

 public:
  explicit FileException(const std::string in_filename)
    : exception(),
    errorMessage("Could not open " + in_filename + ".")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};


class NotMatchingLociException_MA : public std::exception{

 public:
  explicit NotMatchingLociException_MA(const std::string in_locus)
    : exception(),
    errorMessage("Specified locus " + in_locus + " not found.")
    {}
  
  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class NotMatchingLociException_GLC : public std::exception{

 public:
  explicit NotMatchingLociException_GLC(const std::string in_locus,
					const std::string in_id)
    : exception(),
    errorMessage("Specified locus " + in_locus + " not found in id " + in_id + ".")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class MissingGenotypeException : public std::exception{
  
  virtual const char* what() const throw()
  {
    return "Encountered missing genotype.";
  }
};

class MissingGlidException : public std::exception{

 public:
  explicit MissingGlidException(const size_t in_glid)
    : exception(),
    errorMessage("GL-id " + std::to_string(in_glid) + " not found in glid-file.")
    {}

  virtual const char* what() const throw()
  {
    return errorMessage.c_str();
  }

 private:
  std::string errorMessage;
};

class SplittingGenotypeException : public std::exception{

 public:
  virtual const char* what() const throw()
  {
    return "Too broad splitting of genotype.";
  }
};

class InputLineException : public std::exception{

 public:
  virtual const char* what() const throw()
  {
    return "Wrong line length.";
  }
};

#endif