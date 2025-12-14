// include/DomainOverflow.h
#ifndef DOMAIN_OVERFLOW_H
#define DOMAIN_OVERFLOW_H

#include "llvm/Support/raw_ostream.h"

namespace overflow {

class DomainOverflow {
public:
  // Special bounds for +/- infinity in the abstract domain.
  static const long long NEG_INF;
  static const long long POS_INF;

  bool      isBottom;  // true => ⊥ (unreachable)
  long long low;       // lower bound (NEG_INF means -inf)
  long long high;      // upper bound (POS_INF means +inf)

  DomainOverflow();                         // bottom
  DomainOverflow(long long L, long long H); // interval [L, H]

  // Factory helpers
  static DomainOverflow bottom(); // ⊥
  static DomainOverflow top();    // [-inf, +inf]

  // Interval arithmetic (mathematical, ignoring machine wraparound)
  static DomainOverflow add(const DomainOverflow &A,
                            const DomainOverflow &B);
  static DomainOverflow sub(const DomainOverflow &A,
                            const DomainOverflow &B);
  static DomainOverflow mul(const DomainOverflow &A,
                            const DomainOverflow &B);
  static DomainOverflow shl(const DomainOverflow &A,
                            const DomainOverflow &B);

  // Lattice operations
  static DomainOverflow join(const DomainOverflow &A,
                             const DomainOverflow &B);
  static DomainOverflow widen(const DomainOverflow &Old,
                              const DomainOverflow &New);
  static bool equal(const DomainOverflow &A,
                    const DomainOverflow &B);

  void print(llvm::raw_ostream &O) const;
};

} // namespace overflow

#endif // DOMAIN_OVERFLOW_H
