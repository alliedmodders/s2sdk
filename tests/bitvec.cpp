#include "bitvec.h"
#include "const.h"

// Sizes with a specialization, without one, and not a multiple of 32
template class CBitVecT<CFixedBitVecBase<32>>;
template class CBitVecT<CFixedBitVecBase<33>>;
template class CBitVecT<CFixedBitVecBase<ABSOLUTE_PLAYER_LIMIT>>;
template class CBitVecT<CFixedBitVecBase<256>>;
template class CBitVecT<CFixedBitVecBase<MAX_EDICTS>>;
template class CFixedBitVecBase<33>;
template class CBitVec<MAX_EDICTS>;
template class CTypedBitVec<64>;
template class CBitVecT<CVarBitVecBase>;
