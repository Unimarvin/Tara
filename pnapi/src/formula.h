#ifndef PNAPI_FORMULA_H
#define PNAPI_FORMULA_H

#include <list>
#include <map>
#include <string>

using std::list;
using std::map;
using std::string;

namespace pnapi
{

/// forward declaration needed by Formula
class PetriNet;
class Place;
class Node;
class Marking;

  /*!
   * \brief   Final Condition Formulas
   */
  namespace formula {

  class Formula
  {
  public:
    // destructor
    virtual ~Formula() {}

    // evaluating the formula
    virtual bool evaluate(Marking &m) const = 0;

    // output method for formulas
    virtual const string toString() const = 0;

    virtual Formula * flatCopy() const = 0;

    virtual Formula * deepCopy(map<Place *, Place *> &newP) const = 0;

  };




  /***********************************************
   *               Atomic Formulas               *
   ***********************************************/
  class AtomicFormula : public Formula
  {
  public:
    /// standard constructor
    AtomicFormula(Place &p, unsigned int k);

    virtual ~AtomicFormula() {}

    virtual bool evaluate(Marking &m) const = 0;

    Place & getPlace();

    unsigned int getNumber();

    virtual const string toString() const = 0;

    virtual AtomicFormula * flatCopy() const = 0;

    virtual AtomicFormula * deepCopy(map<Place *, Place *> &newP) const = 0;

  protected:
    /// copy constructor
    AtomicFormula(const AtomicFormula &f);

    Place &place_;

    unsigned int number_;
  };

  class FormulaEqual : public AtomicFormula
  {
  public:
    /// standard constructor
    FormulaEqual(Place &p, unsigned int k);

    virtual ~FormulaEqual() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaEqual * flatCopy() const;

    FormulaEqual * deepCopy(map<Place *, Place *> &newP) const;
  };

  class FormulaNotEqual : public AtomicFormula
  {
  public:
    /// standard constructor
    FormulaNotEqual(Place &p, unsigned int k);

    /// copy constructor
    //FormulaNotEqual(const FormulaNotEqual &f);

    virtual ~FormulaNotEqual() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaNotEqual * flatCopy() const;

    FormulaNotEqual * deepCopy(map<Place *, Place *> &newP) const;
  };

  class FormulaGreater : public AtomicFormula
  {
  public:
    /// standard constructor
    FormulaGreater(Place &p, unsigned int k);

    /// copy constructor
    //FormulaGreater(const FormulaGreater &f);

    virtual ~FormulaGreater() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaGreater * flatCopy() const;

    FormulaGreater * deepCopy(map<Place *, Place *> &newP) const;
  };

  class FormulaGreaterEqual : public AtomicFormula
  {
  public:
    /// standard constructor
    FormulaGreaterEqual(Place &p, unsigned int k);

    /// copy constructor
    //FormulaGreaterEqual(const FormulaGreaterEqual &f);

    virtual ~FormulaGreaterEqual() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaGreaterEqual * flatCopy() const;

    FormulaGreaterEqual * deepCopy(map<Place *, Place *> &newP) const;
  };

  class FormulaLess : public AtomicFormula
  {
  public:
    /// standard constructor
    FormulaLess(Place &p, unsigned int k);

    ///copy constructor
    //FormulaLess(const FormulaLess &f);

    virtual ~FormulaLess() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaLess * flatCopy() const;

    FormulaLess * deepCopy(map<Place *, Place *> &newP) const;
  };

  class FormulaLessEqual : public AtomicFormula
  {
  public:
    /// standard constructor
    FormulaLessEqual(Place &p, unsigned int k);

    /// copy constructor
    //FormulaLessEqual(const FormulaLessEqual &f);

    virtual ~FormulaLessEqual() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaLessEqual * flatCopy() const;

    FormulaLessEqual * deepCopy(map<Place *, Place *> &newP) const;
  };




  /************************************************
   *          Unary Boolean Formulas              *
   ************************************************/
  class UnaryBooleanFormula : public Formula
  {
  public:
    /// standard constructor
    UnaryBooleanFormula(Formula *f);

    UnaryBooleanFormula(const UnaryBooleanFormula &f);

    virtual ~UnaryBooleanFormula() {}

    virtual bool evaluate(Marking &m) const = 0;

    virtual const string toString() const = 0;

    virtual UnaryBooleanFormula * flatCopy() const = 0;

    virtual UnaryBooleanFormula * deepCopy(map<Place *, Place *> &newP) const = 0;

  protected:
    Formula *sub_;
  };

  class FormulaNot : public UnaryBooleanFormula
  {
  public:
    /// standard constructor
    FormulaNot(Formula *f);

    FormulaNot(const Formula &);

    /// copy constructor
    //FormulaNot(const FormulaNot &f);

    virtual ~FormulaNot() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaNot * flatCopy() const;

    FormulaNot * deepCopy(map<Place *, Place *> &newP) const;
  };




  /************************************************
   *           n ary Boolean Formulas             *
   ************************************************/
  class NaryBooleanFormula : public Formula
  {
  public:
    /// standard constructors
    NaryBooleanFormula(Formula *l, Formula *r);

    NaryBooleanFormula(list<Formula *> &flst);

    virtual ~NaryBooleanFormula() {}

    virtual bool evaluate(Marking &m) const = 0;

    virtual const string toString() const = 0;

    void addSubFormula(Formula *s);

    virtual NaryBooleanFormula * flatCopy() const = 0;

    virtual NaryBooleanFormula * deepCopy(map<Place *, Place *> &newP) const = 0;

  protected:
    list<Formula *> subs_;

    /// copy constructor
    NaryBooleanFormula(const NaryBooleanFormula &f);
  };

  class FormulaAnd : public NaryBooleanFormula
  {
  public:
    /// standard constructors
    FormulaAnd(Formula *l, Formula *r);

    FormulaAnd(const Formula &l, const Formula &r);

    FormulaAnd(list<Formula *> &flst);

    virtual ~FormulaAnd() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaAnd * flatCopy() const;

    FormulaAnd * deepCopy(map<Place *, Place *> &newP) const;
  };

  class FormulaOr : public NaryBooleanFormula
  {
  public:
    /// standard constructor
    FormulaOr(Formula *l, Formula *r);

    FormulaOr(const Formula &l, const Formula &r);

    FormulaOr(list<Formula *> &flst);

    virtual ~FormulaOr() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    FormulaOr * flatCopy() const;

    FormulaOr * deepCopy(map<Place *, Place *> &newP) const;
  };




  /************************************************
   *                Empty Formula                 *
   ************************************************/
  class True : public Formula
  {
  public:
    virtual ~True() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    True * flatCopy() const;

    True * deepCopy(map<Place *, Place *> &newP) const;
  };

  class False : public Formula
  {
  public:
    virtual ~False() {}

    bool evaluate(Marking &m) const;

    const string toString() const;

    False * flatCopy() const;

    False * deepCopy(map<Place *, Place *> &newP) const;
  };


  /// formula construction operator
  FormulaEqual operator==(Place &, unsigned int);

  /// formula construction operator
  FormulaNotEqual operator!=(Place &, unsigned int);

  /// formula construction operator
  FormulaGreater operator>(Place &, unsigned int);

  /// formula construction operator
  FormulaGreaterEqual operator>=(Place &, unsigned int);

  /// formula construction operator
  FormulaLess operator<(Place &, unsigned int);

  /// formula construction operator
  FormulaLessEqual operator<=(Place &, unsigned int);

  /// formula construction operator
  FormulaAnd operator&&(const Formula &, const Formula &);

  /// formula construction operator
  FormulaOr operator||(const Formula &, const Formula &);

  /// formula construction operator
  FormulaNot operator!(const Formula &);


  } /* namespace formula */

} /* namespace pnapi */

#endif /* FORMULA_H */
