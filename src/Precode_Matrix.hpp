/*
 * Copyright (c) 2015, Luca Fulchir<luca@fulchir.it>, All rights reserved.
 *
 * This file is part of "libRaptorQ".
 *
 * libRaptorQ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * libRaptorQ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and a copy of the GNU Lesser General Public License
 * along with libRaptorQ.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RAPTORQ_PRECODE_MATRIX_HPP
#define RAPTORQ_PRECODE_MATRIX_HPP

#include "common.hpp"
#include "Parameters.hpp"
#include "multiplication.hpp"
#include <Eigen/Dense>
#include <deque>
#include <memory>
#include "Operation.hpp"

namespace RaptorQ {
namespace Impl {

// track the bitmask of holes for received symbols.
// also make it easy to know how many non-repair symbols we are missing.
class RAPTORQ_API Bitmask
{
public:
	const uint16_t _max_nonrepair;

	Bitmask (const uint16_t symbols);

	void add (const size_t id);
	bool exists (const size_t id) const;
	uint16_t get_holes () const;
private:
	// NOTE: this is not just vector<bool> 'cause this gives us a bit more
	// breating space and less reallocations, but really, why don't we just
	// use vector<bool> with more allocated space?
	std::vector<size_t> mask;
	uint16_t holes;
};

using DenseMtx = Eigen::Matrix<Octet, Eigen::Dynamic, Eigen::Dynamic,
															Eigen::RowMajor>;
enum class RAPTORQ_API Save_Computation : uint8_t {
	OFF = 0,
	ON = 1
};

template<Save_Computation IS_OFFLINE>
class RAPTORQ_API Precode_Matrix
{
	using Op_Vec = std::deque<std::unique_ptr<Operation>>;
public:
	const Parameters _params;

	Precode_Matrix(const Parameters &params)
		:_params (params)
	{}

	void gen (const uint32_t repair_overhead);

	DenseMtx intermediate (DenseMtx &D, Op_Vec &ops, uint16_t &size);
	DenseMtx intermediate (DenseMtx &D, const Bitmask &mask,
										const std::vector<uint32_t> &repair_esi,
										Op_Vec &ops, uint16_t &size);
	DenseMtx encode (const DenseMtx &C, const uint32_t iSI) const;

private:
	DenseMtx A;
	uint32_t _repair_overhead = 0;

	// indenting here prepresent which function needs which other.
	// not standard, ask me if I care.
	void init_LDPC1 (const uint16_t S, const uint16_t B);
	void init_LDPC2 (const uint16_t skip, const uint16_t rows,
														const uint16_t cols);
	void add_identity (const uint16_t size, const uint16_t skip_row,
													const uint16_t skip_col);

	void init_HDPC ();
		DenseMtx make_MT() const;		// rfc 6330, pgg 24, used for HDPC
		DenseMtx make_GAMMA() const;	// rfc 6330, pgg 24, used for HDPC
	void add_G_ENC ();


	void decode_phase0 (const Bitmask &mask,
									const std::vector<uint32_t> &repair_esi);
	std::tuple<bool, uint16_t, uint16_t> decode_phase1 (DenseMtx &X,DenseMtx &D,
										std::vector<uint16_t> &c, Op_Vec &ops);
	bool decode_phase2 (DenseMtx &D, const uint16_t i,const uint16_t u,
																Op_Vec &ops);
	void decode_phase3 (const DenseMtx &X, DenseMtx &D, const uint16_t i,
																Op_Vec &ops);
	void decode_phase4 (DenseMtx &D, const uint16_t i, const uint16_t u,
																Op_Vec &ops);
	void decode_phase5 (DenseMtx &D, const uint16_t i, Op_Vec &ops);
};

}	// namespace Impl
}	// namespace RaptorQ

#include "Precode_Matrix_Init.hpp" // above template implementation
#include "Precode_Matrix_Solver.hpp" // above template implementation

#endif
