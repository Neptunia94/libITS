#include "PBoolExpression.hh"
#include "hashfunc.hh"
#include <cassert>
#include <typeinfo>



namespace its {


// unique storage class
class _PBoolExpression {
  // count references
  mutable size_t refCount;
  // access to refcount for garbage purpose
  friend class PBoolExpressionFactory;
 public :
  // add a ref
  size_t ref () const { 
    return ++refCount;
  }
  // dereference
  size_t deref () const { 
    assert(refCount >0);
    return --refCount;
  }
  // default constructor
  _PBoolExpression (): refCount(0) {}
  // reclaim memory
  virtual ~_PBoolExpression () { assert (refCount==0); };
  
  ///////// Interface functions
  // for hash storage
  virtual size_t hash () const = 0 ;
  virtual bool operator==(const _PBoolExpression & e) const = 0;
  virtual bool operator< (const _PBoolExpression & other) const = 0;


  virtual _PBoolExpression * clone () const = 0;

  // to avoid excessive typeid RTTI calls.
  virtual BoolExprType getType() const =0;

  // Because friend is not transitive. Offer access to PBoolExpression concrete to subclasses.
  static const _PBoolExpression * getConcrete ( const PBoolExpression & e) { return e.concrete ;}

  // pretty print
  virtual void print (std::ostream & os) const =0 ;

  // Evaluate an expression.
  virtual PBoolExpression eval() const = 0;

  virtual PBoolExpression setAssertion (const PAssertion & a) const = 0;

  virtual bool isSupport (int varIndex, int id) const = 0;
  virtual void getSupport(bool * const mark) const = 0;

  virtual PBoolExpression reindexVariables (const PIntExpression::indexes_t & newindexes) const = 0;
  virtual PIntExpression getFirstSubExpr () const = 0;
};


class NaryBoolExpr : public _PBoolExpression {
protected :
  typedef std::vector<PBoolExpression> params_t;
  typedef params_t::const_iterator params_it;
  
  params_t params;
public :
  virtual const char * getOpString() const = 0;

  params_it begin() const { return params.begin(); }
  params_it end() const { return params.end(); }

  NaryBoolExpr (const NaryPBoolParamType & pparams):params(pparams.begin(), pparams.end()) {};

  PBoolExpression eval () const {
    NaryPBoolParamType p ;
    for (params_it it = begin(); it != end() ; ++it ) {
      PBoolExpression e = it->eval();
      if (e.getType() == BOOLCONST) {
	bool val = e.getValue();
	if ( (getType() == AND && ! val) // XXX && false = false
	     || (getType() == OR && val) // XXX || true = true
	     )
	  return e;
	else
	  // XXX && true = XXX
	  // XXX || false = XXX
	  continue;
      } else {
	p.insert(e);
      }
    }
    if (p.empty())
      if (getType() == OR)
	return PBoolExpressionFactory::createConstant(false);
      else
	return PBoolExpressionFactory::createConstant(true);
    else if (p.size() == 1) 
      return *p.begin();
    else 
      return PBoolExpressionFactory::createNary(getType(),p);
  }

  void print (std::ostream & os) const {
    os << "( ";
    for (params_it it = begin() ;  ; ) {
      it->print(os);
      if (++it == end())
	break;
      else
	os << getOpString();
    }
    os << " )";
  }

  bool operator== (const _PBoolExpression & e) const {
    const NaryBoolExpr & other = (const NaryBoolExpr &)e ;
    return other.params == params;
  }

  bool operator< (const _PBoolExpression & e) const {
    const NaryBoolExpr & other = (const NaryBoolExpr &)e ;
    return params < other.params;
  }



  size_t hash () const {
    size_t res = getType();
    for (params_it it = begin() ; it != end()  ; ++it ) {
      res = res*(it->hash() +  10099);
    }
    return res;
  }

  PBoolExpression setAssertion (const PAssertion & a) const {
    NaryPBoolParamType res ;
    bool isUpd = false;
    for (params_it it = begin() ; it != end()  ; ++it ) {
      PBoolExpression aa = (*it) & a;      
      if (aa.getType() == BOOLCONST) {
	bool val = aa.getValue();
	if ( (getType() == AND && ! val) // XXX && false = false
	     || (getType() == OR && val) // XXX || true = true
	     )
	  return aa;
	else
	  // XXX && true = XXX
	  // XXX || false = XXX
	  isUpd = true;
	  continue;
      } else {
	res.insert( aa );
	if (! (aa == *it)) {
	  isUpd = true;
	}
      }
    }
    if (isUpd)
      return PBoolExpressionFactory::createNary(getType(),res).eval();
    else
      return this;
  }

  bool isSupport (int v, int id) const {
    for (params_it it = begin() ; it != end()  ; ++it ) {
      if (it->isSupport(v,id)) 
	return true;
    }
    return false;
  }
  
  void getSupport(bool * const mark) const {
    for (params_it it = begin() ; it != end() ; ++it) {
      it->getSupport(mark);
    }
  }

  PIntExpression getFirstSubExpr () const {
    PIntExpression zero(0);
    for (params_it it = begin() ; it != end()  ; ++it ) {
      PIntExpression tmp = it->getFirstSubExpr();
      if (! tmp.equals(zero)) 
	return tmp;
    }
    return zero;
  }

  PBoolExpression reindexVariables (const PIntExpression::indexes_t & newindex) const {
    NaryPBoolParamType res ;
    for (params_it it = params.begin() ; it != params.end()  ; ++it ) {
      res.insert( it->reindexVariables(newindex));
    }
    return PBoolExpressionFactory::createNary(getType(),res);    
  }


};

class OrExpr : public NaryBoolExpr {

public :
  OrExpr (const NaryPBoolParamType & pparams):NaryBoolExpr(pparams) {};
  BoolExprType getType() const  { return OR; }
  const char * getOpString() const { return " || ";}
  virtual _PBoolExpression * clone () const { return new OrExpr(*this);}
};

class AndExpr : public NaryBoolExpr {

public :
  AndExpr (const NaryPBoolParamType  & pparams):NaryBoolExpr(pparams) {};
  BoolExprType getType() const  { return AND; }
  const char * getOpString() const { return " && ";}
  virtual _PBoolExpression * clone () const { return new AndExpr(*this);}
};

class BinaryBoolComp : public _PBoolExpression {
protected :
  PIntExpression left;
  PIntExpression right;
public :
  virtual const char * getOpString() const = 0;
  BinaryBoolComp (const PIntExpression & lleft, const PIntExpression & rright) : left (lleft),right(rright){};

  virtual bool constEval (int i, int j) const = 0;

  PBoolExpression eval () const {
    PIntExpression  l = left.eval();
    PIntExpression  r = right.eval();

    if (l.getType() == CONST && r.getType() == CONST ) {
      return  PBoolExpressionFactory::createConstant( constEval( l.getValue(),
								r.getValue()) );
    } else {
      return  PBoolExpressionFactory::createComparison( getType(), l, r );
    }
  }

  

  PBoolExpression setAssertion (const PAssertion & a) const {
    PIntExpression l = left & a;
    PIntExpression r = right & a;
    if (l.getType() == CONST && r.getType() == CONST ) {
      return  PBoolExpressionFactory::createConstant( constEval( l.getValue(),
								r.getValue()) );
    }
    return PBoolExpressionFactory::createComparison(getType(),l,r);    
  }


  bool operator==(const _PBoolExpression & e) const{
    const BinaryBoolComp & other = (const BinaryBoolComp &)e ;
    return other.left.equals(left) && other.right.equals(right);
  }
  bool operator<(const _PBoolExpression & e) const{
    const BinaryBoolComp & other = (const BinaryBoolComp &)e ;
    return left.equals(other.left)
      ? right.less(other.right)
      : left.less(other.left) ;
  }



  size_t hash () const {
    size_t res = getType();
    res *= left.hash() *  76303 + right.hash() * 76147;
    return res;
  }

  void print (std::ostream & os) const {
    os << "( ";
    left.print(os);
    os << getOpString();
    right.print(os);
    os << " )";
  }

  bool isSupport (int v, int id) const {
    return left.isSupport(v,id) || right.isSupport(v,id);
  }
  
  void getSupport(bool * const mark) const {
    left.getSupport(mark);
    right.getSupport(mark);
  }

  PIntExpression getFirstSubExpr () const {
    PIntExpression zero(0);
    {
      PIntExpression tmp = left.getFirstSubExpr();
      if (! tmp.equals(left)) 
	return tmp;
    }
    {
      PIntExpression tmp = right.getFirstSubExpr();
      if (! tmp.equals(right)) 
	return tmp;
    }
    return zero;
  }

  PBoolExpression reindexVariables (const PIntExpression::indexes_t & newindex) const {
    return PBoolExpressionFactory::createComparison(getType(),left.reindexVariables(newindex), right.reindexVariables(newindex));    
  }



};

class BoolEq : public BinaryBoolComp {

public :
  BoolEq (const PIntExpression & left, const PIntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return EQ; }
  const char * getOpString() const { return " == ";}

  bool constEval (int i, int j) const {
    return i==j;
  }
  virtual _PBoolExpression * clone () const { return new BoolEq(*this);}
};

class BoolNeq : public BinaryBoolComp {

public :
  BoolNeq (const PIntExpression & left, const PIntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return NEQ; }
  const char * getOpString() const { return " != ";}

  bool constEval (int i, int j) const {
    return i!=j;
  }

  virtual _PBoolExpression * clone () const { return new BoolNeq(*this);}
};

class BoolGeq : public BinaryBoolComp {

public :
  BoolGeq (const PIntExpression & left, const PIntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return GEQ; }
  const char * getOpString() const { return " >= ";}

  bool constEval (int i, int j) const {
    return i>=j;
  }
  virtual _PBoolExpression * clone () const { return new BoolGeq(*this);}
};


class BoolLeq : public BinaryBoolComp {

public :
  BoolLeq (const PIntExpression & left, const PIntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LEQ; }
  const char * getOpString() const { return " <= ";}

  bool constEval (int i, int j) const {
    return i<=j;
  }
  virtual _PBoolExpression * clone () const { return new BoolLeq(*this);}
};

class BoolLt : public BinaryBoolComp {

public :
  BoolLt (const PIntExpression & left, const PIntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LT; }
  const char * getOpString() const { return " < ";}

  bool constEval (int i, int j) const {
    return i<j;
  }
  virtual _PBoolExpression * clone () const { return new BoolLt(*this);}
};

class BoolGt : public BinaryBoolComp {

public :
  BoolGt (const PIntExpression & left, const PIntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return GT; }
  const char * getOpString() const { return " > ";}

  bool constEval (int i, int j) const {
    return i>j;
  }
  virtual _PBoolExpression * clone () const { return new BoolGt(*this);}
};


class NotExpr : public _PBoolExpression {
  PBoolExpression exp;  

public :
  NotExpr (const PBoolExpression & eexp) : exp(eexp){};
  BoolExprType getType() const  { return NOT; }

  PBoolExpression eval () const {
    PBoolExpression e = exp.eval();
    if (e.getType() == BOOLCONST)
      return  PBoolExpressionFactory::createConstant(! e.getValue());
    return  PBoolExpressionFactory::createNot(e);
  }

  PBoolExpression setAssertion (const PAssertion & a) const {
    PBoolExpression e = exp & a;
    if (e.getType() == BOOLCONST)
      return  PBoolExpressionFactory::createConstant(! e.getValue());

    return PBoolExpressionFactory::createNot(e);
  }

  bool operator==(const _PBoolExpression & e) const{
    const NotExpr & other = (const NotExpr &)e ;
    return other.exp == exp;
  }

  bool operator<(const _PBoolExpression & e) const{
    const NotExpr & other = (const NotExpr &)e ;
    return exp < other.exp;
  }


  virtual _PBoolExpression * clone () const { return new NotExpr(*this);}

  size_t hash () const {
    return  7829 * exp.hash();
  }

  void print (std::ostream & os) const {
    os << " ! (" ;
    exp.print(os);
    os << " ) ";
  }

  bool isSupport (int v, int id) const {
    return exp.isSupport(v,id);
  }
  
  void getSupport(bool * const mark) const {
    exp.getSupport(mark);
  }

  PIntExpression getFirstSubExpr () const {
    return exp.getFirstSubExpr();
  }

  PBoolExpression reindexVariables (const PIntExpression::indexes_t & newindex) const {
    return PBoolExpressionFactory::createNot(exp.reindexVariables(newindex));
  }


};

class BoolConstExpr : public _PBoolExpression {
  bool val;

public :
  BoolConstExpr (bool vval) : val(vval) {}
  BoolExprType getType() const  { return BOOLCONST; }

  bool getValue() const { return val;}

  PBoolExpression eval () const {
    return this;
  }
  bool operator==(const _PBoolExpression & e) const {
    return val == ((const BoolConstExpr &)e).val;
  }

  bool operator<(const _PBoolExpression & e) const {
    return val < ((const BoolConstExpr &)e).val;
  }

  virtual _PBoolExpression * clone () const { return new BoolConstExpr(*this);}
  PBoolExpression setAssertion (const PAssertion&) const {
    return this;
  }

  virtual size_t hash () const {
    return val?6829:102547;
  }
  void print (std::ostream & os) const {
    os << val;
  }

  bool isSupport (int,int) const {
    return false;
  }
  void getSupport(bool * const ) const {
    return ;
  }

  PIntExpression getFirstSubExpr () const {
    return 0;
  }

  PBoolExpression reindexVariables (const PIntExpression::indexes_t & ) const {
    return this ;
  }


};


// namespace PBoolExpressionFactory {
UniqueTable<_PBoolExpression>  PBoolExpressionFactory::unique = UniqueTable<_PBoolExpression>();

PBoolExpression PBoolExpressionFactory::createNary (BoolExprType type, const NaryPBoolParamType & params) {
  if (params.size() == 1) {
    return *params.begin();
  }
  switch (type) {
  case OR :
    return unique(OrExpr (params));
  case AND :
    return unique(AndExpr (params));      
  default :
    throw "Operator " + to_string(type) +" is not an N-ary bool op";
  }

}

PBoolExpression PBoolExpressionFactory::createBinary (BoolExprType type, const PBoolExpression & l, const PBoolExpression & r) {
  NaryPBoolParamType params;
  params.insert(l);
  params.insert(r);
  return createNary (type,params);
}


PBoolExpression PBoolExpressionFactory::createNot  (const PBoolExpression & e) {
  return unique(NotExpr(e));
}

PBoolExpression PBoolExpressionFactory::createConstant (bool b) {
  return unique(BoolConstExpr(b));
}

// a comparison (==,!=,<,>,<=,>=) between two integer expressions
PBoolExpression PBoolExpressionFactory::createComparison (BoolExprType type, const PIntExpression & l, const PIntExpression & r) {
  switch (type) {
  case EQ :
    return unique(BoolEq (l,r));
  case NEQ :
    return unique(BoolNeq (l,r));
  case GEQ :
    return unique(BoolGeq (l,r));
  case LEQ :
    return unique(BoolLeq (l,r));
  case LT :
    return unique(BoolLt (l,r));
  case GT :
    return unique(BoolGt (l,r));      
  default :
    throw "not a binary comparison operator !";
  }
}


void PBoolExpressionFactory::destroy (_PBoolExpression * e) {
  if (  e->deref() == 0 ){
    UniqueTable<_PBoolExpression>::Table::iterator ci = unique.table.find(e);
    assert (ci != unique.table.end());
    unique.table.erase(ci);
    delete e;
  }
}


void PBoolExpressionFactory::printStats (std::ostream &os) {
  os << "Boolean expression entries :" << unique.size() << std::endl;
}


// nary constructions
PBoolExpression operator&&(const PBoolExpression & a,const PBoolExpression & b) {
  NaryPBoolParamType p;
  if (a.getType() == AND) {
    const NaryBoolExpr * pa = (const NaryBoolExpr *) a.concrete;
    p.insert(pa->begin(), pa->end());
  } else {
    p.insert(a);
  }
  if (b.getType() == AND) {
    const NaryBoolExpr * pb = (const NaryBoolExpr *) b.concrete;
    p.insert(pb->begin(), pb->end());
  } else {
    p.insert (b);
  }
  return PBoolExpressionFactory::createNary(AND, p);
}

PBoolExpression operator||(const PBoolExpression & a,const PBoolExpression & b) {
  NaryPBoolParamType p;
  if (a.getType() == OR) {
    const NaryBoolExpr * pa = (const NaryBoolExpr *) a.concrete;
    p.insert(pa->begin(), pa->end());
  } else {
    p.insert(a);
  }
  if (b.getType() == OR) {
    const NaryBoolExpr * pb = (const NaryBoolExpr *) b.concrete;
    p.insert(pb->begin(), pb->end());
  } else {
    p.insert (b);
  }
  return PBoolExpressionFactory::createNary(OR, p);
} 

PBoolExpression PBoolExpression::operator!() const {
  return PBoolExpressionFactory::createNot(*this);
} 

PBoolExpression::PBoolExpression (const PIntExpression & expr): concrete(NULL) {
   *this = PBoolExpressionFactory::createComparison(EQ,expr,PIntExpressionFactory::createConstant(1));
}


// binary comparisons
PBoolExpression operator==(const PIntExpression & l, const PIntExpression & r) {
  return  PBoolExpressionFactory::createComparison (EQ,l,r);
}
PBoolExpression operator!=(const PIntExpression & l, const PIntExpression & r) {
  return  PBoolExpressionFactory::createComparison (NEQ,l,r);
}
PBoolExpression operator<=(const PIntExpression & l, const PIntExpression & r) {
  return  PBoolExpressionFactory::createComparison (LEQ,l,r);
}
PBoolExpression operator>=(const PIntExpression & l, const PIntExpression & r) {
  return  PBoolExpressionFactory::createComparison (GEQ,l,r);
}
PBoolExpression operator<(const PIntExpression & l, const PIntExpression & r) {
  return  PBoolExpressionFactory::createComparison (LT,l,r);
}
PBoolExpression operator>(const PIntExpression & l, const PIntExpression & r) {
  return  PBoolExpressionFactory::createComparison (GT,l,r);
}

std::ostream & operator << (std::ostream & os, const PBoolExpression & e) {
  e.print(os);
  return os;
}
// necessary administrative trivia
// refcounting
PBoolExpression::PBoolExpression (const _PBoolExpression * concret): concrete(concret) {
  concrete->ref();
}

PBoolExpression::PBoolExpression (const PBoolExpression & other) {
  if (this != &other) {
    concrete = other.concrete;
    concrete->ref();
  }
}

  bool PBoolExpression::isSupport (int v,int id) const {
    return concrete->isSupport(v,id);
}

void PBoolExpression::getSupport(bool * const mark) const {
  concrete->getSupport(mark);
}

PBoolExpression::~PBoolExpression () {
  // remove const qualifier for delete call
  PBoolExpressionFactory::destroy((_PBoolExpression *) concrete);  
}

PBoolExpression & PBoolExpression::operator= (const PBoolExpression & other) {
  if (this != &other) {
    // remove const qualifier for delete call
    if (concrete)
      PBoolExpressionFactory::destroy((_PBoolExpression *) concrete);
    concrete = other.concrete;
    concrete->ref();
  }
  return *this;
}

bool PBoolExpression::operator== (const PBoolExpression & other) const {
  return concrete == other.concrete ;
}

bool PBoolExpression::operator< (const PBoolExpression & other) const {

// Address based comparison while valid is not stable across runs, which is a real pain.
//  return concrete < other.concrete;

  if (getType() != other.getType()) {
    return getType() < other.getType();
  }
  return *concrete < * other.concrete ;
}


/// To handle nested expressions (e.g. array access). Returns the constant 0 if there are no nested expressions.
PIntExpression PBoolExpression::getFirstSubExpr () const {
  return concrete->getFirstSubExpr();
}


void PBoolExpression::print (std::ostream & os) const {
  concrete->print(os);
}


PBoolExpression PBoolExpression::eval () const {
  return concrete->eval();
}

PBoolExpression PBoolExpression::reindexVariables ( const indexes_t & newindex ) const {
  return concrete->reindexVariables(newindex);
}

/// only valid for CONST expressions
/// use this call only in form : if (e.getType() == CONST) { int j = e.getValue() ; ...etc }
/// Exceptions will be thrown otherwise.
bool PBoolExpression::getValue () const {
  if (getType() != BOOLCONST) {
    throw "Do not call getValue on non constant Boolean expressions.";
  } else {
    return ((const BoolConstExpr *) concrete)->getValue();    
  }
}



PBoolExpression PBoolExpression::operator& (const PAssertion &a) const {
  return concrete->setAssertion(a);
}

BoolExprType PBoolExpression::getType() const {
  return concrete->getType();
}


size_t PBoolExpression::hash () const { 
  return ddd::knuth32_hash(reinterpret_cast<const size_t>(concrete)); 
}


} //namespace


namespace d3 { namespace util {
  template<>
  struct hash<its::_PBoolExpression*> {
    size_t operator()(its::_PBoolExpression * g) const{
      return g->hash();
    }
  };

  template<>
  struct equal<its::_PBoolExpression*> {
    bool operator()(its::_PBoolExpression *g1,its::_PBoolExpression *g2) const{
      return (typeid(*g1) == typeid(*g2) && *g1 == *g2);
    }
  };
} }
