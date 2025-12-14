#include "DomainOverflow.h"

#include <algorithm>

using namespace llvm;

namespace overflow {

// Choose big sentinels for +/- infinity in the abstract domain.
const long long DomainOverflow::NEG_INF = -(1LL << 62);
const long long DomainOverflow::POS_INF =  (1LL << 62);

DomainOverflow::DomainOverflow()
    : isBottom(true), low(0), high(0) {}

DomainOverflow::DomainOverflow(long long L, long long H)
    : isBottom(false), low(L), high(H) {}

DomainOverflow DomainOverflow::bottom() {
  return DomainOverflow();
}

DomainOverflow DomainOverflow::top() {
  return DomainOverflow(NEG_INF, POS_INF);
}

// ---- helpers on bounds with infinities ----

static long long addBound(long long a, long long b) {
  if (a == DomainOverflow::NEG_INF || b == DomainOverflow::NEG_INF)
    return DomainOverflow::NEG_INF;
  if (a == DomainOverflow::POS_INF || b == DomainOverflow::POS_INF)
    return DomainOverflow::POS_INF;
  return a + b;
}

static long long subBound(long long a, long long b) {
  if (a == DomainOverflow::NEG_INF || b == DomainOverflow::POS_INF)
    return DomainOverflow::NEG_INF;
  if (a == DomainOverflow::POS_INF || b == DomainOverflow::NEG_INF)
    return DomainOverflow::POS_INF;
  return a - b;
}

static long long mulBound(long long a, long long b) {
  // Anything * 0 is 0 in the abstraction.
  if (a == 0 || b == 0)
    return 0;

  if (a == DomainOverflow::NEG_INF || a == DomainOverflow::POS_INF ||
      b == DomainOverflow::NEG_INF || b == DomainOverflow::POS_INF) {
    bool neg = (a < 0) ^ (b < 0);
    return neg ? DomainOverflow::NEG_INF : DomainOverflow::POS_INF;
  }

  return a * b;
}

// ---- interval operations ----

DomainOverflow DomainOverflow::add(const DomainOverflow &A,
                                   const DomainOverflow &B) {
  if (A.isBottom || B.isBottom)
    return bottom();

  long long l = addBound(A.low,  B.low);
  long long h = addBound(A.high, B.high);
  return DomainOverflow(l, h);
}

DomainOverflow DomainOverflow::sub(const DomainOverflow &A,
                                   const DomainOverflow &B) {
  if (A.isBottom || B.isBottom)
    return bottom();

  long long l = subBound(A.low,  B.high);
  long long h = subBound(A.high, B.low);
  return DomainOverflow(l, h);
}

DomainOverflow DomainOverflow::mul(const DomainOverflow &A,
                                   const DomainOverflow &B) {
  if (A.isBottom || B.isBottom)
    return bottom();

  long long c1 = mulBound(A.low,  B.low);
  long long c2 = mulBound(A.low,  B.high);
  long long c3 = mulBound(A.high, B.low);
  long long c4 = mulBound(A.high, B.high);

  long long l = std::min(std::min(c1, c2), std::min(c3, c4));
  long long h = std::max(std::max(c1, c2), std::max(c3, c4));
  return DomainOverflow(l, h);
}

DomainOverflow DomainOverflow::shl(const DomainOverflow &A,
                                   const DomainOverflow &B) {
  if (A.isBottom || B.isBottom)
    return bottom();

  // We only handle non-negative shift amounts; otherwise, go to top.
  if (B.low < 0)
    return top();

  // If the shift amount can be huge, just give up and go to top.
  if (B.high > 60)
    return top();

  long long minShift = B.low;
  long long maxShift = B.high;

  auto shiftBound = [](long long x, long long s) -> long long {
    if (x == DomainOverflow::NEG_INF) return DomainOverflow::NEG_INF;
    if (x == DomainOverflow::POS_INF) return DomainOverflow::POS_INF;
    return x << s;
  };

  long long c1 = shiftBound(A.low,  minShift);
  long long c2 = shiftBound(A.low,  maxShift);
  long long c3 = shiftBound(A.high, minShift);
  long long c4 = shiftBound(A.high, maxShift);

  long long l = std::min(std::min(c1, c2), std::min(c3, c4));
  long long h = std::max(std::max(c1, c2), std::max(c3, c4));
  return DomainOverflow(l, h);
}

// ---- lattice ops ----

DomainOverflow DomainOverflow::join(const DomainOverflow &A,
                                    const DomainOverflow &B) {
  if (A.isBottom) return B;
  if (B.isBottom) return A;

  long long l = std::min(A.low,  B.low);
  long long h = std::max(A.high, B.high);
  return DomainOverflow(l, h);
}

DomainOverflow DomainOverflow::widen(const DomainOverflow &Old,
                                     const DomainOverflow &New) {
  // Widening operator: forces convergence by jumping to infinity
  // when bounds keep growing

  if (Old.isBottom) return New;
  if (New.isBottom) return Old;

  // If the new interval is wider than the old one, jump to infinity
  long long l = (New.low < Old.low) ? NEG_INF : Old.low;
  long long h = (New.high > Old.high) ? POS_INF : Old.high;

  return DomainOverflow(l, h);
}

bool DomainOverflow::equal(const DomainOverflow &A,
                           const DomainOverflow &B) {
  if (A.isBottom != B.isBottom)
    return false;
  if (A.isBottom)
    return true;
  return A.low == B.low && A.high == B.high;
}

// ---- printing ----

void DomainOverflow::print(raw_ostream &O) const {
  if (isBottom) {
    O << "⊥";
    return;
  }

  auto printBound = [&](long long v) {
    if (v == NEG_INF) O << "-inf";
    else if (v == POS_INF) O << "+inf";
    else O << v;
  };

  O << "[";
  printBound(low);
  O << ", ";
  printBound(high);
  O << "]";
}

} // namespace overflow
