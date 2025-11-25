#ifndef DOMAIN_H
#define DOMAIN_H

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace dataflow {

//===----------------------------------------------------------------------===//
// Abstract Domain Implementation
//===----------------------------------------------------------------------===//

/*
 * Implement your abstract domain.
 */
class Domain {
 public:
  enum Element {
    Uninit,
    // For divide-by-zero analysis
    Zero,
    NonZero,
    MaybeZero,
    // For null pointer analysis
    NonNull,
    Null,
    MaybeNull,
    // For overflow analysis
    Overflow,
    MaybeOverflow,
    NotOverflow
  };
  Domain();
  Domain(Element V);
  Element Value;

  // Arithmetic operations for divide-by-zero analysis
  static Domain *add(Domain *E1, Domain *E2);
  static Domain *sub(Domain *E1, Domain *E2);
  static Domain *mul(Domain *E1, Domain *E2);
  static Domain *div(Domain *E1, Domain *E2);

  // Arithmetic operations for overflow analysis
  static Domain *addOverflow(Domain *E1, Domain *E2);
  static Domain *subOverflow(Domain *E1, Domain *E2);
  static Domain *mulOverflow(Domain *E1, Domain *E2);
  static Domain *shlOverflow(Domain *E1, Domain *E2);

  static Domain *join(Domain *E1, Domain *E2);
  static bool equal(Domain E1, Domain E2);
  void print(raw_ostream &O);
};

raw_ostream &operator<<(raw_ostream &O, Domain V);

}  // namespace dataflow

#endif  // DOMAIN_H
