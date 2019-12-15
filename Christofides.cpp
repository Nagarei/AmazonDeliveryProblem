#include "Estimater.h"
#include <algorithm>
#include <numeric>
#include <execution>
#include <cassert>

Christofides::Data::Data(const State& state)
{
	std::vector<int32_t> remv(2 * N);
	std::iota(remv.begin(), remv.end(), (int32_t)0);
	s_to_tree_index = *std::min_element(std::execution::par_unseq, remv.begin(), remv.end(), [](int32_t a, int32_t b) {
			return distance[2 * N][a] < distance[2 * N][b];
		});
	basetree = Prim(std::move(remv), 2*N, {(int64_t*)distance, N_MAX, N_MAX}, treecost);

	total_cost = 2*distance[2 * N][s_to_tree_index] + treecost;
	total_cost += state.cost;
}

Christofides::Data::Data(Data&& estimater_data, const State&)
{
	*this = std::move(estimater_data);
}


int64_t Christofides::build_v_to_st(int nowv, int& now_index, int from)
{
	int64_t distmin = distance[from][nowv];
	for (auto& e : data.basetree[nowv]) {
		distmin = std::min(distmin, build_v_to_st(e, now_index, from));
	}
	v_to_st_dist[from][now_index] = distmin;
	++now_index;
	return distmin;
}
int64_t Christofides::build_st_to_st(int nowv, int& now_index, int to_st)
{
	int64_t distmin = v_to_st_dist[nowv][to_st];
	for (auto& e : data.basetree[nowv]) {
		distmin = std::min(distmin, build_st_to_st(e, now_index, to_st));
	}
	st_to_st_dist[to_st][now_index] = distmin;
	++now_index;
	return distmin;
}

Christofides::Christofides(Data& data, const State&, const std::vector<int32_t>& remv)
	: data(data)
	, subtree_dist_data(data.basetree.size()*data.basetree.size(), INF)
	, st_to_st_dist(subtree_dist_data.data(), vnum, vnum)
	, vnum(data.basetree.size())
	, v_to_st_dist_data( vnum*vnum, INF )
	, v_to_st_dist( v_to_st_dist_data.data(), vnum, vnum)
{

	if (data.basetree.empty()) {
		int64_t treecost;
		//remv‚Émove‚Í‚Â‚¯‚È‚¢
		data.basetree = Prim(remv, 2 * N, { (int64_t*)distance, N_MAX, N_MAX }, treecost);
		assert(treecost == data.treecost);
	}

	std::for_each(std::execution::par_unseq, remv.begin(), remv.end(), [this, &remv](int32_t i) {
		int now_index = 0;
		build_v_to_st(remv.back(), now_index, i);
	});
	std::for_each(std::execution::par_unseq, range_iterator<int32_t>(0), range_iterator<int32_t>((int32_t)remv.size()), [this, &remv](int32_t i) {
		int now_index = 0;
		build_st_to_st(remv.back(), now_index, i);
	});
}

auto Christofides::get_next(const State& prevstate, int32_t nextv) const->Data
{

	return Data();
}

