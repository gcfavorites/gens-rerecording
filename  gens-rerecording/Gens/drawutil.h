#ifndef _DRAWUTIL_H
#define _DRAWUTIL_H

#include "math.h"

typedef unsigned short pix16;
typedef unsigned int pix32;

class DrawUtil
{
public:
	static inline pix32 Blend (pix32 A, pix32 B);
	static inline pix16 Blend (pix16 A, pix16 B);
	static inline pix32 Blend (pix32 A, pix32 B, int AWeight, int BWeight, int Shift);
	static inline pix16 Blend (pix16 A, pix16 B, int AWeight, int BWeight, int Shift);
	static inline pix32 Blend (pix32 To, pix32 From, int Opac); // 256 Opac means To is returned
	static inline pix16 Blend (pix16 To, pix16 From, int Opac); // 0 Opac means From is returned

	static inline pix32 Blend (pix32 A, pix32 B, pix32 C, pix32 D);
	static inline pix16 Blend (pix16 A, pix16 B, pix16 C, pix16 D);
	static inline pix32 Blend_3_1 (pix32 A, pix32 B);
	static inline pix16 Blend_3_1 (pix16 A, pix16 B);

	static inline pix32 Add (pix32 A, pix32 B);
	static inline pix16 Add (pix16 A, pix16 B);

	static inline pix32 Pix16To32 (pix16 Src);
	static inline pix16 Pix32To16 (pix32 Src);

	enum
	{
		RMASK_16 = 0xF800,
		GMASK_16 = 0x07E0,
		BMASK_16 = 0x001F,
		RBMASK_16 = (RMASK_16|BMASK_16),
		RMASK_32 = 0x00FF0000,
		GMASK_32 = 0x0000FF00,
		BMASK_32 = 0x000000FF,
		RBMASK_32 = (RMASK_32|BMASK_32),
	};
};

// macro to apply some code to the R, G, and B components of a 16 or 32 bit pixel and return the resulting pixel value
// every reference to pixel x in <code> must be of the form (x & MASK)
// assumes the range (per component) of outputs has no bits that are more than 1 full component away from the closest bit in the largest valid input (e.g. does not shift the input more than 8 bits left or right if 32-bit)
#define DO_RGB(depth, code) \
	pix##depth rv; \
	{ enum{MASK = RBMASK_##depth}; rv = (code)&MASK; } \
	{ enum{MASK =  GMASK_##depth}; rv |= (code)&MASK; } \
	return rv;

inline pix16 DrawUtil::Blend (pix16 A, pix16 B) {
	DO_RGB(16, (((A & MASK) + (B & MASK)) >> 1) ); }
inline pix32 DrawUtil::Blend (pix32 A, pix32 B) {
	DO_RGB(32, (((A & MASK) + (B & MASK)) >> 1) ); }

inline pix16 DrawUtil::Blend (pix16 A, pix16 B, int AWeight, int BWeight, int Shift) {
	DO_RGB(16, (((A & MASK)*AWeight + (B & MASK)*BWeight) >> Shift) ); }
inline pix32 DrawUtil::Blend (pix32 A, pix32 B, int AWeight, int BWeight, int Shift) {
	DO_RGB(32, (((A & MASK)*AWeight + (B & MASK)*BWeight) >> Shift) ); }

inline pix16 DrawUtil::Blend (pix16 To, pix16 From, int Opac) {
	DO_RGB(16, (((To & MASK) * Opac + (From & MASK) * (256-Opac) ) >> 8) ); }
inline pix32 DrawUtil::Blend (pix32 To, pix32 From, int Opac) {
	DO_RGB(32, (((To & MASK) * Opac + (From & MASK) * (256-Opac) ) >> 8) ); }

inline pix16 DrawUtil::Blend (pix16 A, pix16 B, pix16 C, pix16 D) {
	return Blend(Blend(A,B),Blend(C,D)); }
inline pix32 DrawUtil::Blend (pix32 A, pix32 B, pix32 C, pix32 D) {
	return Blend(Blend(A,B),Blend(C,D)); }

inline pix16 DrawUtil::Blend_3_1 (pix16 A, pix16 B) {
	return Blend(A,B, 3,1, 2); }
inline pix32 DrawUtil::Blend_3_1 (pix32 A, pix32 B) {
	return Blend(A,B, 3,1, 2); }

inline pix16 DrawUtil::Add (pix16 A, pix16 B) {
	DO_RGB(16, ((A & MASK) + (B & MASK)) ); }
inline pix32 DrawUtil::Add (pix32 A, pix32 B) {
	DO_RGB(32, ((A & MASK) + (B & MASK)) ); }


#undef DO_RGB


// from: xxxxxxxxRRRRRxxxGGGGGGxxBBBBBxxx
//   to:                 RRRRRGGGGGGBBBBB
inline pix16 DrawUtil::Pix32To16 (pix32 Src)
{
	int rm = Src & 0xF80000;
	int gm = Src & 0xFC00;
	int bm = Src & 0xF8;
	return (rm >> 8) | (gm >> 5) | (bm >> 3);
}

// from:                 RRRRRGGGGGGBBBBB
//   to: 00000000RRRRR000GGGGGG00BBBBB000
inline pix32 DrawUtil::Pix16To32 (pix16 Src)
{
	int rm = Src & 0xF800;
	int gm = Src & 0x7E0;
	int bm = Src & 0x1F;
	return (rm << 8) | (gm << 5) | (bm << 3);
}

#endif
