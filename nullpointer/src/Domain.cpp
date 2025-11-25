#include "Domain.h"

//===----------------------------------------------------------------------===//
// Abstract Domain Implementation
//===----------------------------------------------------------------------===//

namespace dataflow {

Domain::Domain() {
  Value = Uninit;
}
Domain::Domain(Element V) {
  Value = V;
}

Domain *Domain::add(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);
  if (E1->Value == Zero && E2->Value == Zero)
    return new Domain(Zero);
  if (E1->Value == Zero && E2->Value == NonZero)
    return new Domain(NonZero);
  if (E1->Value == NonZero && E2->Value == Zero)
    return new Domain(NonZero);
  return new Domain(MaybeZero);
}

Domain *Domain::sub(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);
  if (E1->Value == Zero && E2->Value == Zero)
    return new Domain(Zero);
  if (E1->Value == Zero && E2->Value == NonZero)
    return new Domain(NonZero);
  if (E1->Value == NonZero && E2->Value == Zero)
    return new Domain(NonZero);
  return new Domain(MaybeZero);
}

Domain *Domain::mul(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);
  if (E1->Value == Zero || E2->Value == Zero)
    return new Domain(Zero);
  if (E1->Value == NonZero && E2->Value == NonZero)
    return new Domain(NonZero);
  return new Domain(MaybeZero);
}

Domain *Domain::div(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);
  if (E2->Value == Zero || E2->Value == MaybeZero)
    return new Domain(Uninit);
  if (E1->Value == NonZero)
    return new Domain(MaybeZero);
  if (E1->Value == Zero)
    return new Domain(Zero);
  return new Domain(MaybeZero);
}

// Overflow analysis operations
Domain *Domain::addOverflow(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);

  // If either operand is already marked as overflow, result is overflow
  if (E1->Value == Overflow || E2->Value == Overflow)
    return new Domain(Overflow);

  // If either operand might overflow, result might overflow
  if (E1->Value == MaybeOverflow || E2->Value == MaybeOverflow)
    return new Domain(MaybeOverflow);

  // If both operands are NotOverflow, conservatively assume NotOverflow
  // A proper implementation would use interval analysis here
  if (E1->Value == NotOverflow && E2->Value == NotOverflow)
    return new Domain(NotOverflow);

  // Unknown case
  return new Domain(MaybeOverflow);
}

Domain *Domain::subOverflow(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);

  // If either operand is already marked as overflow, result is overflow
  if (E1->Value == Overflow || E2->Value == Overflow)
    return new Domain(Overflow);

  // If either operand might overflow, result might overflow
  if (E1->Value == MaybeOverflow || E2->Value == MaybeOverflow)
    return new Domain(MaybeOverflow);

  // If both operands are NotOverflow, conservatively assume NotOverflow
  if (E1->Value == NotOverflow && E2->Value == NotOverflow)
    return new Domain(NotOverflow);

  return new Domain(MaybeOverflow);
}

Domain *Domain::mulOverflow(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);

  // If either operand is already marked as overflow, result is overflow
  if (E1->Value == Overflow || E2->Value == Overflow)
    return new Domain(Overflow);

  // If either operand might overflow, result might overflow
  if (E1->Value == MaybeOverflow || E2->Value == MaybeOverflow)
    return new Domain(MaybeOverflow);

  // If both operands are NotOverflow, conservatively assume NotOverflow
  if (E1->Value == NotOverflow && E2->Value == NotOverflow)
    return new Domain(NotOverflow);

  return new Domain(MaybeOverflow);
}

Domain *Domain::shlOverflow(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit || E2->Value == Uninit)
    return new Domain(Uninit);

  // If either operand is already marked as overflow, result is overflow
  if (E1->Value == Overflow || E2->Value == Overflow)
    return new Domain(Overflow);

  // If either operand might overflow, result might overflow
  if (E1->Value == MaybeOverflow || E2->Value == MaybeOverflow)
    return new Domain(MaybeOverflow);

  // If both operands are NotOverflow, conservatively assume NotOverflow
  if (E1->Value == NotOverflow && E2->Value == NotOverflow)
    return new Domain(NotOverflow);

  return new Domain(MaybeOverflow);
}

static bool isOverflowLattice(Domain::Element E) {
  return E == Domain::Overflow || E == Domain::MaybeOverflow || E == Domain::NotOverflow;
}

static bool isZeroLattice(Domain::Element E) {
  return E == Domain::Zero || E == Domain::NonZero || E == Domain::MaybeZero;
}

static bool isNullLattice(Domain::Element E) {
  return E == Domain::Null || E == Domain::NonNull || E == Domain::MaybeNull;
}

Domain *Domain::join(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit) return new Domain(*E2);
  if (E2->Value == Uninit) return new Domain(*E1);

  // Handle zero lattice (for divide-by-zero analysis)
  if (isZeroLattice(E1->Value) && isZeroLattice(E2->Value)) {
    if (E1->Value == MaybeZero || E2->Value == MaybeZero)
      return new Domain(MaybeZero);
    if (E1->Value == Zero && E2->Value == Zero)
      return new Domain(Zero);
    if (E1->Value == NonZero && E2->Value == NonZero)
      return new Domain(NonZero);
    return new Domain(MaybeZero);
  }

  // Handle null lattice (for null pointer analysis)
  if (isNullLattice(E1->Value) && isNullLattice(E2->Value)) {
    if (E1->Value == MaybeNull || E2->Value == MaybeNull)
      return new Domain(MaybeNull);
    if (E1->Value == Null && E2->Value == Null)
      return new Domain(Null);
    if (E1->Value == NonNull && E2->Value == NonNull)
      return new Domain(NonNull);
    // One is Null, the other is NonNull
    return new Domain(MaybeNull);
  }

  // Handle overflow lattice (for overflow analysis)
  if (isOverflowLattice(E1->Value) && isOverflowLattice(E2->Value)) {
    if (E1->Value == Overflow || E2->Value == Overflow)
      return new Domain(Overflow);
    if (E1->Value == MaybeOverflow || E2->Value == MaybeOverflow)
      return new Domain(MaybeOverflow);
    return new Domain(NotOverflow);
  }

  // Default: return MaybeZero for mixed lattices
  return new Domain(Uninit);
}

bool Domain::equal(Domain E1, Domain E2) {
  return E1.Value == E2.Value;
}

void Domain::print(raw_ostream &O) {
  switch (Value) {
    case Uninit:
      O << "Uninit         ";
      break;
    case Zero:
      O << "Zero           ";
      break;
    case NonZero:
      O << "NonZero        ";
      break;
    case MaybeZero:
      O << "MaybeZero      ";
      break;
    case Null:
      O << "Null           ";
      break;
    case NonNull:
      O << "NonNull        ";
      break;
    case MaybeNull:
      O << "MaybeNull      ";
      break;
    case Overflow:
      O << "Overflow       ";
      break;
    case MaybeOverflow:
      O << "MaybeOverflow  ";
      break;
    case NotOverflow:
      O << "NotOverflow    ";
      break;
  }
}

raw_ostream &operator<<(raw_ostream &O, Domain V) {
  V.print(O);
  return O;
}

};  // namespace dataflow
