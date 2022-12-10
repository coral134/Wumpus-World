#ifndef _LOGIC_ENGINE_H
#define _LOGIC_ENGINE_H

#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <cassert>

template<class key_type, class state_type>
class LogicEngine {
private:
    std::list<std::map<key_type, state_type>> configs_;
    std::map<key_type, state_type> known_;
    std::vector<state_type> possible_states_;

public:
    LogicEngine() = default;
    LogicEngine(const std::vector<state_type> &possible_states) : possible_states_(possible_states) {}

    bool find_by_state(const state_type &state, key_type &key) {
        deduce();
        for(auto i : known_) {
            if(i.second == state) {
                key = i.first;
                return true;
            }
        }
        return false;
    }

    void set_known(const key_type &key, const state_type &state) {
        if(configs_contains(key)) {
            if(configs_.front().size() == 1) configs_.clear();
            else for(auto &config: configs_) config.erase(key);
        }
        known_[key] = state;
    }

    bool highest_prob(const std::vector<state_type> &states, key_type &key) {
        deduce();
        if(configs_.empty()) return false;
        int highestCount = 0;
        for(auto &element : configs_.front()) {
            int c = 0;
            for(auto &config : configs_) {
                state_type state = config[element.first];
                for(state_type s : states) {
                    if(state == s) {
                        ++c;
                        break;
                    }
                }
            }
            if(c > highestCount) {
                highestCount = c;
                key = element.first;
            }
        }
        return highestCount > 0;
    }

    void add(const key_type &key) {
        if(contains(key)) return;
        if(configs_.empty()) configs_.emplace_back();

        std::vector<std::list<std::map<std::pair<int, int>, state_type>>> configs_copies;
        for(int i = 0; i < (int)possible_states_.size(); ++i) {
            configs_copies.push_back(configs_);
            for(auto &map : configs_copies[i])
                map.insert({key, possible_states_[i]});
        }

        for(int i = 1; i < (int)configs_copies.size(); ++i)
            configs_copies[0].splice(configs_copies[0].end(), configs_copies[i]);

        configs_ = configs_copies[0];
    }

    bool contains(const key_type &key) {
        return configs_contains(key) || ((known_.find(key) != known_.end()));
    }

    bool configs_contains(const key_type &key) {
        if(configs_.empty()) return false;
        return configs_.begin()->find(key) != configs_.begin()->end();
    }

    void separate_knowns(const std::vector<key_type> &keys, std::vector<key_type> &knowns, std::vector<key_type> &unknowns) {
        for(auto &key : keys) {
            add(key);
            if(configs_contains(key)) unknowns.push_back(key);
            else knowns.push_back(key);
        }
    }

    void constrain_at_least_one_of(const std::vector<key_type> &keys, state_type value) {
        std::vector<key_type> knowns, unknowns;
        separate_knowns(keys, knowns, unknowns);
        for(auto &key : knowns)
            if(known_.find(key)->second == value) return;

        if(!configs_.empty()) {
            auto config = configs_.begin();
            while(config != configs_.end()) {
                bool found = false;
                for(auto &key: unknowns) {
                    if(config->find(key)->second == value) {
                        found = true;
                        break;
                    }
                }
                if(found) ++config;
                else config = configs_.erase(config);
            }
            assert(!configs_.empty());
        }
    }

    void constrain_none_of(const std::vector<key_type> &keys, state_type value) {
        std::vector<key_type> knowns, unknowns;
        separate_knowns(keys, knowns, unknowns);
        for(auto &key : knowns)
            assert(known_.find(key)->second != value);

        if(!configs_.empty()) {
            auto config = configs_.begin();
            while(config != configs_.end()) {
                bool found = false;
                for(auto &key: unknowns) {
                    if(config->find(key)->second == value) {
                        found = true;
                        break;
                    }
                }
                if(found) config = configs_.erase(config);
                else ++config;
            }
            assert(!configs_.empty());
        }
    }

    void constrain_all_of(const std::vector<key_type> &keys, state_type value) {
        std::vector<key_type> knowns, unknowns;
        separate_knowns(keys, knowns, unknowns);
        for(auto &key : knowns)
            assert(known_.find(key)->second == value);

        if(!configs_.empty()) {
            auto config = configs_.begin();
            while(config != configs_.end()) {
                bool all = true;
                for(auto &key: unknowns) {
                    if(config->find(key)->second != value) {
                        all = false;
                        break;
                    }
                }
                if(all) ++config;
                else config = configs_.erase(config);
            }
            assert(!configs_.empty());
        }
    }

    void constrain_one_of(state_type value) {
        if(!configs_.empty()) {
            auto config = configs_.begin();
            while(config != configs_.end()) {
                int found = 0;
                for(auto &i : *config) {
                    if(i.second == value)
                        found++;
                }
                if(found > 1) config = configs_.erase(config);
                else ++config;
            }
            assert(!configs_.empty());
        }
    }

    void constrain_one_of(const std::vector<key_type> &keys, state_type value) {
        std::vector<key_type> knowns, unknowns;
        separate_knowns(keys, knowns, unknowns);
        int found = 0;
        for(auto &key : knowns)
            if(known_.find(key)->second == value) ++found;

        assert((found == 0) || (found == 1));
        if(found == 1) {
            constrain_none_of(unknowns, value);
        } else {
            if(!configs_.empty()) {
                auto config = configs_.begin();
                while(config != configs_.end()) {
                    found = 0;
                    for(auto &key: keys) {
                        if(config->find(key)->second == value)
                            ++found;
                    }
                    if(found == 1) ++config;
                    else config = configs_.erase(config);
                }
                assert(!configs_.empty());
            }
        }
    }

    int num_configurations() {
        if(configs_.empty()) return 1;
        else return (int)configs_.size();
    }

    std::pair<int, int> fraction(const key_type &key, state_type state) {
        int sum = 0;
        if(configs_contains(key)) {
            for(auto &config: configs_)
                if(config.find(key)->second == state) ++sum;
            return {sum, num_configurations()};
        } else if(contains(key)) {
            if(known_.find(key)->second == state) return {1, 1};
            else return {0, 1};
        } else {
            return {1, (int)possible_states_.size()};
        }
    }

    std::pair<std::vector<state_type>, double> most_likely(const key_type &key) {
        if(!contains(key)) return {possible_states_, 1.0 / possible_states_.size()};

        std::map<double, std::vector<state_type>> likelihoods;
        for(state_type state : possible_states_)
            likelihoods[count(key, state)].push_back(state);
        std::vector<state_type> answer;
        auto itr = likelihoods.rbegin();
        assert(itr != likelihoods.rend());
        return {itr->second, ((double)itr->first)/num_configurations()};
    }

    int count(const key_type &key, state_type state) {
        if(configs_contains(key)) {
            int sum = 0;
            for(auto &config: configs_)
                if(config.find(key)->second == state) ++sum;
            return sum;
        } else if(contains(key)) {
            if(known_.find(key)->second == state) return num_configurations();
            else return 0;
        }
        return 0;
    }

    bool is_true(const key_type &key, state_type state) {
        auto fract = fraction(key, state);
        return fract.first == fract.second;
    }

    void deduce() {
        if(configs_.empty()) return;
        for(int i = 0; i < (int)configs_.begin()->size();) { // loops over each location
            auto first_pair = configs_.begin()->begin();
            for(int j = 0; j < i; ++j) ++first_pair;

            bool all_same = true;
            for(auto &config : configs_) { // loops over each possible configuration
                auto each_other_pair = config.begin();
                for(int j = 0; j < i; ++j) ++each_other_pair; // iter now points to correct location in one configuration
                if(each_other_pair->second != first_pair->second) {
                    all_same = false;
                    break;
                }
            }

            if(all_same) {
                known_.insert(*first_pair);
                for(auto &config : configs_) // loops over each possible configuration
                    config.erase(first_pair->first);
                if(configs_.begin()->empty()) {
                    configs_ = std::list<std::map<key_type, state_type>>();
                    return;
                }
            } else ++i;
        }
    }
};

#endif //_LOGIC_ENGINE_H
