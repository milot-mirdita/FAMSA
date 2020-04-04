/*
This file is a part of FAMSA software distributed under GNU GPL 3 licence.
The homepage of the FAMSA project is http://sun.aei.polsl.pl/REFRESH/famsa

Authors: Sebastian Deorowicz, Agnieszka Debudaj-Grabysz, Adam Gudys

*/
#pragma once
#include "AbstractTreeGenerator.h"

#include "../lcs/lcsbp.h"

#include <cmath>

// overloads for converting sequence type to pointer
inline CSequence* seq_to_ptr(CSequence* x) { return x; }
inline CSequence* seq_to_ptr(CSequence& x) { return &x; }

template <class T, Transformation transformation>
struct Transform {
	T operator()(T x) { return x; }
};

template <class T>
struct Transform<T, Transformation::Reciprocal> {
	T operator()(T x) { return 1.0 / x; }
};

template <class T>
struct Transform<T, Transformation::Inverse> {
	T operator()(T x) { return -x; }
};


// *******************************************************************
/*
similarity_type can be:
	- CSequence,
	- CSequence*,
*/
template <class seq_type, class similarity_type, Transformation transformation>
void AbstractTreeGeneator::calculateSimilarityVector(
	CSequence* ref, 
	seq_type* sequences, 
	size_t n_seqs, 
	similarity_type* out_vector, 
	CLCSBP& lcsbp)
{
	uint32_t lcs_lens[4];
	Transform<similarity_type, transformation> transform;
	
	ref->ComputeBitMasks();

	// process portions of 4 sequences
	for (int j = 0; j < n_seqs / 4; ++j) {
		lcsbp.GetLCSBP(
			ref,
			seq_to_ptr(sequences[j * 4 + 0]),
			seq_to_ptr(sequences[j * 4 + 1]),
			seq_to_ptr(sequences[j * 4 + 2]),
			seq_to_ptr(sequences[j * 4 + 3]),
			lcs_lens[0], lcs_lens[1], lcs_lens[2], lcs_lens[3]);

		for (int k = 0; k < 4; ++k) {
			similarity_type indel = ref->length + seq_to_ptr(sequences[j * 4 + k])->length - 2 * lcs_lens[k];
			out_vector[j * 4 + k] = transform(lcs_lens[k] / std::pow(indel, indel_exp));
		}
	}

	// if there is something left
	size_t n_processed = n_seqs / 4 * 4;
	if (n_processed < n_seqs) {
		lcsbp.GetLCSBP(
			ref,
			(n_processed + 0 < n_seqs) ? seq_to_ptr(sequences[n_processed + 0]) : nullptr,
			(n_processed + 1 < n_seqs) ? seq_to_ptr(sequences[n_processed + 1]) : nullptr,
			(n_processed + 2 < n_seqs) ? seq_to_ptr(sequences[n_processed + 2]) : nullptr,
			(n_processed + 3 < n_seqs) ? seq_to_ptr(sequences[n_processed + 3]) : nullptr,
			lcs_lens[0], lcs_lens[1], lcs_lens[2], lcs_lens[3]);

		for (int k = 0; k < 4 && n_processed + k < n_seqs; ++k)
		{
			similarity_type indel = ref->length + seq_to_ptr(sequences[n_processed + k])->length - 2 * lcs_lens[k];
			out_vector[n_processed + k] = transform(lcs_lens[k] / std::pow(indel, indel_exp));
		}
	}

	ref->ReleaseBitMasks();

}


// *******************************************************************
template <class seq_type, class similarity_type, Transformation transformation>
void AbstractTreeGeneator::calculateSimilarityMatrix(
	seq_type* sequences, 
	size_t n_seq, 
	similarity_type *out_matrix, 
	CLCSBP& lcsbp) {

	for (size_t row_id = 0; row_id < n_seq; ++row_id) {
		
		size_t row_offset = TriangleMatrix::access(row_id, 0);
		
		calculateSimilarityVector<seq_type, similarity_type, transformation>(
			sequences[row_id],
			sequences,
			row_id,
			out_matrix + row_offset,
			lcsbp);
	}
	
}