#include "Estimater.h"
#include <algorithm>
#include <numeric>
#include <execution>
#include <cassert>


static constexpr int FULL_SEARCH_MAX = 14;
#ifdef NOT_COPY_BASETREE
std::vector<int32_t> Pair_MST::v_to_stnum;
std::vector<TreeIndexType> Pair_MST::basetree;
#define MST_basetree basetree
#define MST_v_to_stnum v_to_stnum
#else
#define MST_basetree data.basetree
#define MST_v_to_stnum data.v_to_stnum
#endif
Pair_MST::Data::Data(const State& state)
{
	std::vector<int32_t> remv(2 * N);
	std::iota(remv.begin(), remv.end(), (int32_t)0);
	auto s_to_tree_index = *std::min_element(std::execution::par_unseq, remv.begin(), remv.end(), [](int32_t a, int32_t b) {
		return distance[2 * N][a] < distance[2 * N][b];
		});
#ifndef NOT_COPY_BASETREE
	basetree =
#endif
		Prim_pair(std::move(remv), 2 * N, { (int64_t*)distance, sizeof(distance) / sizeof(*distance), sizeof(*distance) / sizeof(**distance) }, treecost);
#ifndef NOT_COPY_BASETREE
	v_to_stnumを構築
#endif

		total_cost = 2 * distance[2 * N][s_to_tree_index] + treecost;
	total_cost += state.cost;
}

Pair_MST::Data::Data(Data&& estimater_data, const State&)
{
	*this = std::move(estimater_data);
}


Pair_MST::Pair_MST()
	: data()
	, subtree_dist_data(2 * (2 * N - 1) * 2 * (2 * N - 1), INF)
	, st_to_st_dist(subtree_dist_data.data(), 2 * (2 * N - 1), 2 * (2 * N - 1))
#ifndef NOT_COPY_BASETREE
	, st_to_st_pair_data(subtree_dist_data.size())
	, st_to_st_pair(st_to_st_pair_data.data(), 2 * (2 * N - 1), 2 * (2 * N - 1))
#endif
	, v_to_st_dist_data(2 * N * 2 * (2 * N - 1), INF)
	, v_to_st_dist(v_to_st_dist_data.data(), 2 * N, 2 * (2 * N - 1))
#ifndef NOT_COPY_BASETREE
	, v_to_st_pair_data(v_to_st_dist_data.size())
	, v_to_st_pair(v_to_st_pair_data.data(), 2 * N, 2 * (2 * N - 1))
#endif
{}

//@return 自身のsubtree番号
size_t Pair_MST::build_v_to_st(int nowv, int parent, int from)
{
	int64_t distmin = distance[from][nowv];
#ifndef NOT_COPY_BASETREE
	int_fast16_t minpair = nowv;
#endif
	const size_t size_ = MST_v_to_stnum[nowv + 1] - MST_v_to_stnum[nowv];
	auto stnum = MST_v_to_stnum[nowv];
	size_t this_index_add = size_;
	for (size_t i = 0; i < size_; ++i) {
		const auto& e = MST_basetree[stnum + i];
		if (e == parent) {
			this_index_add = i;
			continue;
		}
		auto nextindex = build_v_to_st(e, nowv, from);
		if (v_to_st_dist[from][nextindex] < distmin) {
			distmin = v_to_st_dist[from][nextindex];
#ifndef NOT_COPY_BASETREE
			minpair = v_to_st_pair[from][nextindex];
#endif
		}
	}
	auto this_index = MST_v_to_stnum[nowv] + this_index_add;
	if (this_index_add != size_) {
		v_to_st_dist[from][this_index] = distmin;
#ifndef NOT_COPY_BASETREE
		v_to_st_pair[from][this_index] = (int16_t)minpair;
#endif
	}
	assert(this_index_add != size_ || parent == -1);
	return this_index;
}
size_t Pair_MST::build_st_to_st(int nowv, int parent, int to_st, int depth)
{
	int64_t distmin = v_to_st_dist[nowv][to_st];
#ifndef NOT_COPY_BASETREE
	std::pair<int16_t, int16_t> minpair = { (int16_t)nowv, v_to_st_pair[nowv][to_st] };
#endif
	const size_t size_ = MST_v_to_stnum[nowv + 1] - MST_v_to_stnum[nowv];
	auto stnum = MST_v_to_stnum[nowv];
	size_t this_index_add = size_;
	if (size_ > 3 || (size_ == 3 && depth < 3)) {
		std::mutex mtx;
		++depth;
		std::for_each(std::execution::par, range_iterator<size_t>(0), range_iterator<size_t>(size_), [&](size_t i) {
			//for (size_t i = 0; i < size_; ++i) {
			const auto& e = MST_basetree[stnum + i];
			if (e == parent) {
				this_index_add = i;
				//continue;
				return;
			}
			auto nextindex = build_st_to_st(e, nowv, to_st, depth);
			std::lock_guard lock(mtx);
			if (st_to_st_dist[to_st][nextindex] < distmin) {
				distmin = st_to_st_dist[to_st][nextindex];
#ifndef NOT_COPY_BASETREE
				minpair = st_to_st_pair[to_st][nextindex];
#endif
			}
			});
	}
	else {
		std::for_each(std::execution::seq, range_iterator<size_t>(0), range_iterator<size_t>(size_), [&](size_t i) {
			//for (size_t i = 0; i < size_; ++i) {
			const auto& e = MST_basetree[stnum + i];
			if (e == parent) {
				this_index_add = i;
				//continue;
				return;
			}
			auto nextindex = build_st_to_st(e, nowv, to_st, depth);
			if (st_to_st_dist[to_st][nextindex] < distmin) {
				distmin = st_to_st_dist[to_st][nextindex];
#ifndef NOT_COPY_BASETREE
				minpair = st_to_st_pair[to_st][nextindex];
#endif
			}
			});
	}
	auto this_index = v_to_stnum[nowv] + this_index_add;
	if (this_index_add != size_) {
		st_to_st_dist[to_st][this_index] = distmin;
#ifndef NOT_COPY_BASETREE
		st_to_st_pair[to_st][this_index] = minpair;
#endif
	}
	assert(this_index_add != size_ || parent == -1);
	return this_index;
}

void Pair_MST::Init_impl(Data&& data_pram, const State& state, const std::vector<int32_t>& remv, const bool(&finished)[2 * N_MAX])
{
	data = std::move(data_pram);
	if (remv.size() < FULL_SEARCH_MAX) {
		bruteforce = Bruteforce::Init(std::move(data.bruteforce), state.havenum, remv, finished);
		return;
	}
#ifdef NOT_COPY_BASETREE
	//remvにmoveはつけない
	//Prim_threadunsafe2_pair(remv, data.treecost, basetree, v_to_stnum);
#endif

	//std::for_each(std::execution::par_unseq, remv.begin(), remv.end(), [this, &remv](int32_t i) {
	//	build_v_to_st(i, -1, i);
	//	});
	//std::vector<int_fast16_t> subtreeroot(2 * (remv.size() - 1));
	//for (size_t v = 0; v + 1 < v_to_stnum.size(); ++v) {
	//	for (size_t i = MST_v_to_stnum[v]; i < MST_v_to_stnum[v + 1]; i++)
	//	{
	//		subtreeroot[i] = (int_fast16_t)v;
	//	}
	//}
	//std::for_each(std::execution::par_unseq, range_iterator<int32_t>(0), range_iterator<int32_t>((int32_t)(2 * (remv.size() - 1))), [this, &remv, &subtreeroot](int32_t i) {
	//	build_st_to_st(subtreeroot[i], -1, i);
	//	});

	//v_to_st_dist_data.clear();
	//v_to_st_pair_data.clear();
}

auto Pair_MST::get_next(const State& prevstate, const std::vector<int32_t>& remv, bool(&finished)[2 * N_MAX], int32_t nextv) const->Data
{
	Data res;
	if (remv.size() < FULL_SEARCH_MAX) {
		res.total_cost = bruteforce.get_next(prevstate, remv, finished, nextv);
		res.bruteforce = bruteforce.get_prevdata();
		return std::move(res);
	}

	std::vector<int32_t> remvnext;
	remvnext.reserve(remv.size() - 1);
	for (auto& i : remv) {
		if (i != nextv) {
			remvnext.push_back(i);
		}
	}
	Prim_pair(std::move(remvnext), 2*N, { (int64_t*)distance, sizeof(distance) / sizeof(*distance), sizeof(*distance) / sizeof(**distance) }, res.treecost);

	//端点二つ
	int64_t s_to_tree = INF;
	int64_t v_to_tree = INF;
	for (auto& v : remv) {
		if (v != nextv) {
			s_to_tree = std::min(s_to_tree, distance[2 * N][v]);
			v_to_tree = std::min(v_to_tree, distance[nextv][v]);
		}
	}

	res.total_cost = v_to_tree + s_to_tree + res.treecost;
	res.total_cost = std::max(prevstate.estimater.total_cost - prevstate.cost, res.total_cost);

	res.total_cost += prevstate.cost + distance[nextv][prevstate.pos];
	return std::move(res);
}

