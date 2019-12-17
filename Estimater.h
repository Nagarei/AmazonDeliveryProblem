#pragma once
#include <vector>
#include <memory>
#include <shared_mutex>
#include "utility.h"
#include "solver.h"

using TreeIndexType = int16_t;
#define NOT_COPY_BASETREE


class Bruteforce
{
public:
	struct Data final {
		std::shared_ptr<std::vector<std::array<std::array<int64_t, 20>, M_MAX+1>>> dp_ptr;
		std::shared_ptr<std::vector<int32_t>> remv_ptr;
	};
private:
	Data prevdata;
	int64_t get_next_bruteforce_dp(int8_t have, size_t posi, uint32_t mask, const bool(&finished)[2 * N_MAX]);
	int64_t get_next_bruteforce_impl(int8_t prevstate_have, const bool(&prevstate_finished)[2 * N_MAX], int32_t nextv)const;
public:
	static Bruteforce Init(Data&& prevdata, int8_t have, const std::vector<int32_t>& remv, const bool(&finished)[2 * N_MAX]);
	template<typename T>
	int64_t get_next(const StateT<T>& prevstate, const std::vector<int32_t>&, const bool(&finished)[2 * N_MAX], int32_t nextv)const
	{
		return prevstate.cost + distance[prevstate.pos][nextv] + get_next_bruteforce_impl(prevstate.havenum, finished, nextv);
	}
	const Data& get_prevdata()const { return prevdata; }
};


class NoEstimate final
{
public:
	struct Data final {
	private:
		friend NoEstimate;
		using State = StateT<Data>;
		int64_t total_cost;
		Data()noexcept :total_cost(){}
	public:
		Data(const StateT<Data>&) :total_cost(0) {}//root初期化
		Data(Data&& data, const StateT<Data>&) :total_cost(data.total_cost) {}

		bool operator<(const Data& r)const {
			return (total_cost) < (r.total_cost);
		}
		bool operator>(const Data& r)const {
			return (total_cost) > (r.total_cost);
		}
	};
	using State = Data::State;
public:
	NoEstimate() {}
	static NoEstimate& Init(Data&&, const State&, const std::vector<int32_t>&, const bool(&)[2 * N_MAX]) {
		static NoEstimate ins;
		return ins;
	}

	//スレッドセーフであること！
	Data get_next(const State& prevstate, const std::vector<int32_t>&, bool(&)[2 * N_MAX], int32_t nextv)const {
		Data data;
		data.total_cost = prevstate.cost + distance[prevstate.pos][nextv];
		return data;
	}
};
class MinimumSpanningTree final
{
public:
	struct Data final {
	private:
		friend MinimumSpanningTree;
		using State = StateT<Data>;
#ifndef NOT_COPY_BASETREE
		std::vector<std::vector<TreeIndexType>> basetree;
#endif
		int64_t treecost;
		int64_t total_cost;
		Bruteforce::Data bruteforce;
		Data()noexcept:treecost(), total_cost() {}
	public:
		Data(const StateT<Data>& state);//root初期化
		Data(Data&& estimater_data, const StateT<Data>& state);

		bool operator<(const Data& r)const {
			return (total_cost) < (r.total_cost);
		}
		bool operator>(const Data& r)const {
			return (total_cost) > (r.total_cost);
		}
	};
	using State = Data::State;
private:
	Bruteforce bruteforce;
	Data data;
	std::vector<int64_t> subtree_dist_data;//二次元のならし
	ArrayView<int64_t, 2> st_to_st_dist;//subtree同士の最短距離
#ifndef NOT_COPY_BASETREE
	std::vector<std::pair<int16_t, int16_t>> st_to_st_pair_data;//二次元のならし
	ArrayView<std::pair<int16_t, int16_t>, 2> st_to_st_pair;//st_to_st_distが何と何のペアかを保存
#endif
	std::vector<int32_t> v_to_stnum;//subtree[root_v][parent_v]のindexはv_to_stnum[root_v] + (find(basetree[root_v], parent_v) - basetree.begin())
#ifdef NOT_COPY_BASETREE
	std::vector<std::vector<TreeIndexType>>* basetree;
#endif

	//temporary in constructor
	std::vector<int64_t> v_to_st_dist_data;//二次元のならし
	ArrayView<int64_t, 2> v_to_st_dist;//頂点とsubtreeの最短距離
#ifndef NOT_COPY_BASETREE
	std::vector<int16_t> v_to_st_pair_data;//二次元のならし
	ArrayView<int16_t, 2> v_to_st_pair;//v_to_st_distが何とのペアかを保存
#endif

	void build_v_to_stnum(int nowv, int& now_index, int parent);
	size_t build_v_to_st(int nowv, int parent, int from);
	size_t build_st_to_st(int nowv, int parent, int to_st, int depth = 0);

private:
	MinimumSpanningTree();
	void Init_impl(Data&& data, const State& state, const std::vector<int32_t>& remv, const bool(&finished)[2 * N_MAX]);
public:
	static MinimumSpanningTree& Init(Data&& data, const State& state, const std::vector<int32_t>& remv, const bool(&finished)[2 * N_MAX]) {
		static MinimumSpanningTree ins;
		ins.Init_impl(std::move(data), state, remv, finished);
		return ins;
	}

	//スレッドセーフであること！
	Data get_next(const State& prevstate, const std::vector<int32_t>& remv, bool(&finished)[2 * N_MAX], int32_t nextv)const;
};

std::vector<std::vector<TreeIndexType>> Prim(std::vector<int32_t> remv, int32_t remvmax, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum);
std::vector<std::vector<TreeIndexType>>* Prim_threadunsafe(std::vector<int32_t> remv, int64_t& out_distsum);