#ifndef _LOGIC_ENGINE_H
#define _LOGIC_ENGINE_H

#include <utility>
#include <vector>
#include <set>
#include <map>
#include <cassert>

template<class key_type, class state_type>
class Node {
public:
    Node() : children({}), parent(nullptr), next(nullptr), last(nullptr), key(), value(), num_leaves(1) {}
    Node(const key_type &k, const state_type &v) : children({}), parent(nullptr), next(nullptr),
        last(nullptr), key(k), value(v), num_leaves(1) {}

    std::set<Node*> children;
    Node *parent, *next, *last;
    key_type key;
    state_type value;
    int num_leaves;
};

template<class key_type, class state_type>
class LogicEngine {
public:
    typedef Node<key_type, state_type> node;

    LogicEngine() = default;

    LogicEngine(const std::set<state_type> &possible_states_) : possible_states(possible_states_) {
        root = new node();
        last_level = new node();
        last_level->next = root;
        root->last = last_level;
    }

    bool find_by_state(const state_type &state, key_type &key) {
        for(const auto &pair : known) {
            if(pair.second == state) {
                key = pair.first;
                return true;
            }
        }
        return false;
    }

    bool is_true(const key_type &key, const state_type &state) {
        auto itr = known.find(key);
        if(itr == known.end()) return false;
        return itr->second == state;
    }

    bool highest_prob(const std::vector<state_type> &states, key_type &key) {
        if(configs.empty()) return false;
        int highestCount = 0;
        for(const auto &level : configs) { // loop over keys
            node* n = level.second->next;
            int count = 0;
            while(n) {
                for(const state_type &state : states) {
                    if(state == n->value) {
                        count += n->num_leaves;
                        break;
                    }
                }
                n = n->next;
            }
            if(count > highestCount) {
                highestCount = count;
                key = level.first;
            }
        }
        return highestCount > 0;
    }

    void set_known(const key_type &key, const state_type &state) {
        if(possible_states.find(state) == possible_states.end()) illegal_state();
        remove_list(key);
        known[key] = state;
    }

    std::pair<std::set<state_type>, double> most_likely(const key_type &key) {
        auto known_itr = known.find(key);
        if(known_itr != known.end()) return {{known_itr->second}, 1};

        auto configs_itr = configs.find(key);
        if(configs_itr == configs.end()) return {possible_states, 1.0 / possible_states.size()};

        std::map<double, std::set<state_type>> likelihoods;
        for(const state_type &state : possible_states)
            likelihoods[count_in_configs(key, state)].insert(state);
        auto itr = likelihoods.rbegin();
        assert(itr != likelihoods.rend());
        return {itr->second, ((double)itr->first)/root->num_leaves};
    }

    void constrain_one_of(const state_type &state) {
        constrain_all(state, 1, false);
    }

    void constrain_none_of(const std::set<key_type> &keys, const state_type &state) {
        constrain_each(keys, state, false);
    }

    void constrain_all_of(const std::set<key_type> &keys, const state_type &state) {
        constrain_each(keys, state, true);
    }

    void constrain_one_of(const std::set<key_type> &keys, const state_type &state) {
        constrain_together(keys, state, 1, false);
    }

    void constrain_at_least_one_of(const std::set<key_type> &keys, const state_type &state) {
        constrain_together(keys, state, 1, true);
    }

    void print() const {
        std::cout << "Knowns: ------------------" << std::endl;
        for(auto s : known)
            std::cout << s.first << ": " << s.second << ", " << std::endl;
        std::cout << std::endl << "Configurations: ------------------" << std::endl;
        int count = 0;
        for(node* c : root->children) print_configs(c, "", count);
        std::cout << "-------------------------------------------" << std::endl;
    }

private:

    std::set<state_type> possible_states;
    std::map<key_type, state_type> known;
    std::map<key_type, node*> configs;
    node *root, *last_level;

    // LOGIC FUNCTIONS

    void deduce() {
        auto itr = configs.begin();
        while(itr != configs.end()) {
            node *n = itr->second->next;
            const state_type &state = n->value;
            bool all_same = true;
            while(n) {
                if(n->value != state) {
                    all_same = false;
                    break;
                }
                n = n->next;
            }
            key_type key = itr->first;
            ++itr;
            if(all_same) set_known(key, state);
        }
    }

    void constrain_each(const std::set<key_type> &keys, const state_type &state, bool is_equal) {
        if(possible_states.find(state) == possible_states.end()) illegal_state();
        for(const key_type &key : keys) {
            auto itr = known.find(key);
            if(itr != known.end()) {
                if(is_equal == (itr->second != state))
                    illegal_constraint();
                continue;
            }

            node *n = add_key(key);
            n = n->next;
            while(n) {
                if(is_equal == (n->value != state)) {
                    node *m = n;
                    n = n->next;
                    delete_branch(m);
                } else n = n->next;
            }
        }
        deduce();
    }

    void constrain_together(const std::set<key_type> &keys, const state_type &state, int min, bool greater) {
        if(possible_states.find(state) == possible_states.end()) illegal_state();
        int found = 0;
        std::set<key_type> unknowns;
        for(const key_type &key : keys) { // loops over keys
            auto known_itr = known.find(key);
            if(known_itr != known.end()) { // if known
                if(known_itr->second == state) {
                    ++found;
                    if(!greater && (found > min))
                        illegal_constraint();
                }
            } else { // if not known
                unknowns.insert(key);
                add_key(key);
            }
        }

        auto children = root->children;
        for(node* c : children)
            constrain_together_rec(c, found, unknowns, state, min, greater);
        deduce();
    }

    void constrain_together_rec(node* n, int found, const std::set<key_type> &keys,
                                const state_type &state, int min, bool greater) {
        if((n->value == state) && (keys.find(n->key) != keys.end())) {
            ++found;
            if(!greater && (found > min)) {
                delete_branch(n);
                return;
            }
        }
        if((n->children.size() == 0) && (found < min)) {
            delete_branch(n);
            return;
        }
        auto children = n->children;
        for(node* c : children)
            constrain_together_rec(c, found, keys, state, min, greater);
    }

    void constrain_all(const state_type &state, int min, bool greater) {
        if(possible_states.find(state) == possible_states.end()) illegal_state();
        int found = 0;
        for(const auto &pair : known) {
            if(pair.second == state) {
                ++found;
                if(!greater && (found > min)) illegal_constraint();
            }
        }

        auto children = root->children;
        for(node* c : children)
            constrain_all_rec(c, found, state, min, greater);
        deduce();
    }

    void constrain_all_rec(node* n, int found, const state_type &state, int min, bool greater) {
        if(n->value == state) {
            ++found;
            if(!greater && (found > min)) {
                delete_branch(n);
                return;
            }
        }
        if((n->children.size() == 0) && (found < min)) {
            delete_branch(n);
            return;
        }
        auto children = n->children;
        for(node* c : children)
            constrain_all_rec(c, found, state, min, greater);
    }

    // GRAPH MANIPULATION FUNCTION

    node* add_key(const key_type &key) {
        auto itr = configs.find(key);
        if(itr != configs.end()) return itr->second;
        node *p = last_level->next;
        last_level = new node();
        last_level->key = key;
        configs[key] = last_level;
        node *l = last_level;

        while(p) {
            for(const state_type &state : possible_states) {
                node* m = new node(key, state);
                m->parent = p;
                m->last = l;
                l->next = m;
                l = m;
                p->children.insert(m);
            }
            update_num_leaves(p);
            p = p->next;
        }

        return last_level;
    }

    void delete_branch(node* n) {
        assert(n != root);
        while(n->parent->children.size() == 1) {
            n = n->parent;
            if(n == root) illegal_constraint();
        }
        n->parent->children.erase(n);
        update_num_leaves(n->parent);
        delete_branch_rec(n);
    }

    void delete_branch_rec(node* n) {
        assert(n != root);
        for(node* c : n->children) delete_branch_rec(c);
        remove_node_from_list(n);
        delete n;
    }

    void remove_node_from_list(node* n) {
        assert(n != root);
        n->last->next = n->next;
        if(n->next) n->next->last = n->last;
    }

    void merge_subtree(node* p, node* q) {
        auto children = q->children;
        for(node* qc : children) {
            bool found = false;
            for(node *pc : p->children) {
                if(pc->value == qc->value) {
                    merge_subtree(pc, qc);
                    found = true;
                    break;
                }
            }
            if(!found) {
                p->children.insert(qc);
                qc->parent = p;
            }
        }

        remove_node_from_list(q);
        delete q;
    }

    void remove_list(const key_type &key) {
        auto itr = configs.find(key);
        if(itr == configs.end()) return;

        node *n = itr->second;
        if(n == last_level) last_level = n->next->parent->last;
        n = n->next;
        node *p = n->parent;

        while(p) {
            p->children.clear();
            p = p->next;
        }

        while(n) {
            node *m = n;
            n = n->next;
            merge_subtree(m->parent, m);
        }

        delete itr->second;
        configs.erase(itr);
    }

    void update_num_leaves(node* n) {
        if(!n) return;
        n->num_leaves = 0;
        for(auto c : n->children)
            n->num_leaves += c->num_leaves;
        update_num_leaves(n->parent);
    }

    // OTHER FUNCTIONS

    int count_in_configs(const key_type &key, const state_type &state) {
        auto itr = configs.find(key);
        assert(itr != configs.end());
        int sum = 0;
        node *n = itr->second->next;
        while(n) {
            if(n->value == state) sum += n->num_leaves;
            n = n->next;
        }
        return sum;
    }

    void illegal_constraint() {
        std::cerr << "You have provided conflicting information!" << std::endl;
        exit(1);
    }

    void illegal_state() {
        std::cerr << "You provided a state that wasn't one of the possible states you specified!" << std::endl;
        exit(1);
    }

    void print_configs(node* n, std::string str, int &count) const {
        str += n->key;
        str += ": ";
        str += n->value;
        str += "(";
        str += std::to_string(n->num_leaves);
        str += ")";
        str += ", ";

        if(n->children.size() == 0) {
            std::cout << count << ": " << str << std::endl;
            count++;
            return;
        }

        for(node* c : n->children)
            print_configs(c, str, count);
    }
};

#endif //_LOGIC_ENGINE_H
