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

Domain *Domain::join(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit) return new Domain(*E2);
  if (E2->Value == Uninit) return new Domain(*E1);
  
  if (E1->Value == MaybeNull || E2->Value == MaybeNull)
    return new Domain(MaybeNull);

  if (E1->Value == Null && E2->Value == Null)
    return new Domain(Null);

  if (E1->Value == NotNull && E2->Value == NotNull)
    return new Domain(NotNull);

  // One is Null, the other is NotNull
  return new Domain(MaybeNull);
}

bool Domain::equal(Domain E1, Domain E2) {
  return E1.Value == E2.Value;
}

void Domain::print(raw_ostream &O) {
  switch (Value) {
    case Uninit:
      O << "Uninit   ";
      break;
    case NonNull:
      O << "NonNull  ";
      break;
    case Null:
      O << "Null     ";
      break;
    case MaybeNull:
      O << "MaybeNull";
      break;
  }
}

raw_ostream &operator<<(raw_ostream &O, Domain V) {
  V.print(O);
  return O;
}

};  // namespace dataflow
