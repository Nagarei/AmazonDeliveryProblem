#pragma once
#include <cstdint>
#include <vector>
#include <bitset>

// main global
constexpr int32_t N_MAX = 1000;
constexpr int64_t DISTANCE_MAX = 10000000;
constexpr int64_t INF = 10100010000000;
extern int32_t N;
extern int32_t M;
extern int64_t start[2];
extern int64_t tasks[N_MAX][2][2];

// utility global
//index tasks[i][a] => 2*i + a
//      start       => 2*N
extern int64_t distance[2 * N_MAX + 1][2 * N_MAX + 1];
void init_distance();

template<typename EstimateData>
struct StateT {
	using This_T = StateT<EstimateData>;
	int64_t cost;
	int32_t pos;
	int8_t havenum;
	std::vector<int16_t> route;
	EstimateData estimater;
	StateT(int64_t cost_, int32_t pos_, int8_t havenum_, std::vector<int16_t> route_, EstimateData estimater_data)
		: cost(cost_)
		, pos(pos_)
		, havenum(havenum_)
		, route(std::move(route_))
		, estimater(std::move(estimater_data), *this)
	{}
	StateT(int64_t cost_, int32_t pos_, int8_t havenum_, std::vector<int16_t> route_)
		: cost(cost_)
		, pos(pos_)
		, havenum(havenum_)
		, route(std::move(route_))
		, estimater(*this)
	{}

	decltype(auto) EstimaterInit(const std::vector<int32_t>& remv) {
		return estimater.Init(*this, remv);
	}
};
template<typename Estimater>
inline bool operator<(const StateT<Estimater>& a, const StateT<Estimater>& b) {
	return a.estimater < b.estimater;
}
template<typename Estimater>
inline bool operator>(const StateT<Estimater>& a, const StateT<Estimater>& b) {
	return a.estimater > b.estimater;
}


std::vector<int16_t> solver_beamsearch();
