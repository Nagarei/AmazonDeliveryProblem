#include "solver.h"
#include "Estimater.h"
#include <bitset>
#include <queue>
#include <execution>
#include <mutex>
#include <iostream>

using CostEstimater = MinimumSpanningTree;
using State = StateT<CostEstimater::Data>;



std::vector<int16_t> solver_beamsearch()
{
	constexpr int32_t BEAM_WIDTH = 50;
	init_distance();

	//int32_t queue_size = 0;
	//std::priority_queue<State, std::vector<State>, std::greater<State>> queue;
	std::vector<State> queue;
	queue.reserve(BEAM_WIDTH);

	queue.emplace_back(0, 2*N, (int8_t)0, std::vector<int16_t>{});
	std::push_heap(queue.begin(), queue.end(), std::greater<>{});
	
	while (!queue.empty()) {
		std::pop_heap(queue.begin(), queue.end(), std::greater<>{});
		auto nstate = std::move(queue.back()); queue.pop_back();

#ifdef _DEBUG
		std::cout << "DEBUG: [";
		for (auto& r : nstate.route) {
			std::cout << r << ", ";
		}
		std::cout << "]" << std::endl;
#endif

		if (nstate.route.size() == 2*N) {
			return nstate.route;
		}

		std::vector<int32_t> remv;
		bool finished[2 * N_MAX] = {};
		for (auto& i : nstate.route) {
			finished[i] = true;
		}
		for (int32_t i = 0; i < 2 * N; i++)
		{
			if (!finished[i]) {
				remv.push_back(i);
			}
		}
		std::mutex mtx;
		auto estimater = nstate.EstimaterInit(remv);
		std::for_each(std::execution::par, remv.begin(), remv.end(), [&nstate, &queue, &mtx, &finished, &estimater, &remv](const int32_t& next) {
			int8_t next_have = nstate.havenum;
			if ((next & 1) == 0) {
				//from
				if (nstate.havenum >= M) {
					return;
				}
				++next_have;
			}
			else {
				//to
				if (!finished[next - 1]) {
					return;
				}
				--next_have;
			}
			std::vector<int16_t> nextroute;
			nextroute.reserve(nstate.route.size() + 1);
			nextroute = nstate.route;//copy
			nextroute.push_back((int16_t)next);
			int64_t nextcost = nstate.cost + distance[nstate.pos][next];
			auto&& nextdata = estimater.get_next(nstate, remv, finished, next);
			std::lock_guard<std::mutex> lock(mtx);
			queue.emplace_back(nextcost, next, next_have, std::move(nextroute), std::move(nextdata));
			std::push_heap(queue.begin(), queue.end(), std::greater<>{});
		});
	}

	return {};
}