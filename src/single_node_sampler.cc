#include "single_node_sampler.h"

namespace dd {

SingleNodeSampler::SingleNodeSampler(std::unique_ptr<CompiledFactorGraph> pfg_,
                                     const Weight weights[], int nthread,
                                     int nodeid, const CmdParser &opts)
    : pfg(std::move(pfg_)),
      opts(opts),
      fg(*pfg),
      infrs(fg, weights),
      nthread(nthread),
      nodeid(nodeid) {}

void SingleNodeSampler::sample(int i_epoch) {
  numa_run_on_node(nodeid);
  threads.clear();
  for (int i = 0; i < nthread; ++i) {
    threads.push_back(std::thread([this, i]() {
      SingleThreadSampler sampler(fg, infrs, opts);
      sampler.sample(i, nthread);
    }));
  }
}

void SingleNodeSampler::wait() {
  for (auto &t : threads) t.join();
}

void SingleNodeSampler::sample_sgd(double stepsize) {
  numa_run_on_node(nodeid);
  threads.clear();
  for (int i = 0; i < nthread; ++i) {
    threads.push_back(std::thread([this, i, stepsize]() {
      SingleThreadSampler sampler(fg, infrs, opts);
      sampler.sample_sgd(i, nthread, stepsize);
    }));
  }
}

void SingleNodeSampler::wait_sgd() {
  for (auto &t : threads) t.join();
}

}  // namespace dd
