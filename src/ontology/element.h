#pragma once
#include <ostream>
#include "ontology/node.h"
#include "ontology/category.h"

using namespace mbdev;

namespace mbdev_ontology {

/*!
  \brief Element stores a value of a specific Category. Value may affect other values of the Cell
   containing it, as well as Cells connected to the above mentioned cell.
  */
class element: public node {
protected:
   /*!
     \brief determines what kind of Element it is i.e. 'thing', 'situation', etc.
     */
   category* cat;
   /*!
     \brief determines if this element is transitive, i.e. if:
      1) lets call this element A
      2) four cells are connected in pairs, like: a->b c->d
      3) both cell a and cell c have element A in them
      4) cell c, after subtraction of element A is equal to cell b
      then the ontology interprets this as if there was a connection between a and c (a->d)

      example ontology: "add category obj" "add category relation" "add obj a" "add obj b"
         "add obj c" "add relation is" "set is transitive" "a is: b" "b is: c"
      query: "find all that is c"
      program responds: "[obj:a, obj:b]"
     */
   bool transitive;
public:
   element(string& name, category* cat, bool transitive = false);
   element(string& name, string& friendlyName, category* cat, bool transitive = false);
   category getCategory() const;
   category* getCategoryPtr();
   bool isTransitive() const;
   void setTransitive(bool isTransitive);
   friend std::ostream& operator<<(std::ostream& os, const element& e);
};

}
