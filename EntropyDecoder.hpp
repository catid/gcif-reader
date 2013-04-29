/*
	Copyright (c) 2013 Game Closure.  All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.
	* Neither the name of GCIF nor the names of its contributors may be used
	  to endorse or promote products derived from this software without
	  specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ENTROPY_DECODER_HPP
#define ENTROPY_DECODER_HPP

#include "HuffmanDecoder.hpp"
#include "ImageReader.hpp"

/*
 * Game Closure Entropy-Based Decompression
 *
 * Decodes the bitstream generated by EntropyEncoder.hpp
 *
 * See that file for more information.
 */

namespace cat {


//// EntropyDecoder

template<int NUM_SYMS, int ZRLE_SYMS> class EntropyDecoder {
public:
	static const int BZ_SYMS = NUM_SYMS + ZRLE_SYMS;
	static const int AZ_SYMS = NUM_SYMS;
	static const int BZ_TAIL_SYM = BZ_SYMS - 1;
	static const int HUFF_LUT_BITS = 9;

protected:
	int _zeroRun;
	HuffmanDecoder _bz, _az;
	bool _afterZero;

public:
	CAT_INLINE EntropyDecoder() {
	}
	virtual CAT_INLINE ~EntropyDecoder() {
	}

	bool init(ImageReader &reader) {
		if (!_bz.init(BZ_SYMS, reader, HUFF_LUT_BITS)) {
			return false;
		}

		if (!_az.init(AZ_SYMS, reader, HUFF_LUT_BITS)) {
			return false;
		}

		_afterZero = false;
		_zeroRun = 0;

		return true;
	}

	u16 next(ImageReader &reader) {
		// If in a zero run,
		if (_zeroRun > 0) {
			--_zeroRun;
			return 0;
		}

		// If after zero,
		if (_afterZero) {
			_afterZero = false;
			return _az.next(reader);
		}

		// Read before-zero symbol
		u16 sym = (u16)_bz.next(reader);

		// If not a zero run,
		if (sym < NUM_SYMS) {
			return sym;
		}

		// If zRLE is represented by the symbol itself,
		if (sym < BZ_TAIL_SYM) {
			// Decode zero run from symbol
			_zeroRun = sym - NUM_SYMS;
		} else {
			_zeroRun = reader.read255255() + ZRLE_SYMS - 1;
		}

		_afterZero = true;
		return 0;
	}
};

} // namespace cat

#endif // ENTROPY_DECODER_HPP

