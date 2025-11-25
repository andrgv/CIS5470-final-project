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

Domain *Domain::join(Domain *E1, Domain *E2) {
  if (E1->Value == Uninit) return new Domain(*E2);
  if (E2->Value == Uninit) return new Domain(*E1);
  
  if (E1->Value == MaybeNull || E2->Value == MaybeNull)
    return new Domain(MaybeNull);

  if (E1->Value == Null && E2->Value == Null)
    return new Domain(Null);

  if (E1->Value == NonNull && E2->Value == NonNull)
    return new Domain(NonNull);

  // One is Null, the other is NonNull
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
