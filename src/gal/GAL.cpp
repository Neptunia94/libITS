#include "GAL.hh"


using std::endl;

namespace its {

  bool Assignment::operator==(const Assignment &other) const {
    return var_.equals(other.var_) && expr_.equals(other.expr_);
  }
  
  void Assignment::print (std::ostream & os) const {
    var_.print(os);
    os <<  " = " ;
    expr_.print(os);    
  }

  bool GuardedAction::operator==(const GuardedAction &other) const {
    bool result = guard_ == other.guard_;
    actions_it ait, bit;
    for (ait = begin(), bit = other.begin() ; result && ait != end() && bit!= other.end() ; ++ait, ++bit) {
      result = (*ait) == (*bit);
    }
    if (ait == end() && bit != other.end()) {
      return false;
    }
    if (bit == other.end() && ait != end()) {
      return false;
    }
    return  result;
  }
  
  void  GuardedAction::print (std::ostream & os) const {
    os << "  transition " << getName() ;
    os << "  [ " ;
    guard_.print(os);
    os << " ] \n";
    os << "     { " ;
    for (actions_it it = begin(); it != end() ; /*done in loop */ ) {
      it->print(os) ;
      os << ";";
      if (++it != end() ) {
	os << "\n       ";
      }
    }
    os << "    }" << endl;
  }

  std::ostream & operator<< ( std::ostream & os, const GAL & gal) {
    os << "GAL " << gal.getName() << endl;
    os << "{" << endl;
    os << "  //variables"<<endl;
    for (GAL::vars_it it = gal.vars_begin() ; it != gal.vars_end() ; ++it ) {
      os  << "  " << it->getName() << "=" << gal.getVarValue(it->getName()) << ";" << endl;
    }

    os << "  //transitions "<<endl;
    for (GAL::trans_it it = gal.trans_begin() ; it != gal.trans_end() ; ++it ) {
      it->print(os);
      os << endl;
    }

    os << "\n}\n"<<endl;;
    return os;
  }



}
