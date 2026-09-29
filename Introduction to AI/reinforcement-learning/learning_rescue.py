"""Student implementation file for HW3 Part C.

Only edit this file unless course staff explicitly instruct otherwise.
"""

from __future__ import annotations
import math
import random


def estimate_transition_reward_model(trajectories, actions, smoothing):
    """Estimate P_hat and R_hat from offline trajectories."""
    transitions_count = {}
    rewards_sum = {}
    rewards_count = {}
    
    for row in trajectories:
        s = row["state"]
        a = row["action"]
        r = row["reward"]
        next_s = row["next_state"]
        
        pair = (s, a)
        if pair not in transitions_count:
            transitions_count[pair] = {}
        if next_s not in transitions_count[pair]:
            transitions_count[pair][next_s] = 0
        transitions_count[pair][next_s] += 1
        
        triple = (s, a, next_s)
        if triple not in rewards_sum:
            rewards_sum[triple] = 0.0
            rewards_count[triple] = 0
        rewards_sum[triple] += r
        rewards_count[triple] += 1
        
    est_transitions = {}
    acts_by_state = {}
    
    for pair in transitions_count:
        curr_state, curr_action = pair
        if curr_state not in acts_by_state:
            acts_by_state[curr_state] = []
        acts_by_state[curr_state].append(curr_action)
        
        next_map = transitions_count[pair]
        denom = sum(next_map.values()) + smoothing * len(next_map)
        
        outcomes = []
        for ns in sorted(next_map.keys()):
            p = (next_map[ns] + smoothing) / denom
            avg_r = rewards_sum[(curr_state, curr_action, ns)] / rewards_count[(curr_state, curr_action, ns)]
            outcomes.append((p, ns, avg_r))
        est_transitions[pair] = outcomes

    for s in acts_by_state:
        ordered = []
        for a in actions:
            if a in acts_by_state[s]:
                ordered.append(a)
        acts_by_state[s] = tuple(ordered)
        
    state_list = []
    for pair, outcomes in est_transitions.items():
        if pair[0] not in state_list: 
            state_list.append(pair[0])
        for _, ns, _ in outcomes:
            if ns not in state_list: 
                state_list.append(ns)
            
    return {
        "transitions": est_transitions,
        "actions_by_state": acts_by_state,
        "states": tuple(sorted(state_list)),
        "actions": actions
    }


def plan_with_estimated_model(model, start_state, horizon):
    """Run finite-horizon planning on the learned model."""
    transitions = model["transitions"]
    actions_by_state = model["actions_by_state"]
    action_order = model["actions"]
    
    reachable = [start_state]
    queue = [start_state]
    while len(queue) > 0:
        curr = queue.pop(0)
        
        curr_acts = []
        for a in action_order:
            if a in actions_by_state.get(curr, ()):
                curr_acts.append(a)
                
        for a in curr_acts:
            for _, next_s, _ in transitions.get((curr, a), []):
                if next_s not in reachable:
                    reachable.append(next_s)
                    queue.append(next_s)
                    
    states_by_time = {}
    for i in range(horizon + 1):
        states_by_time[i] = []
    for s in reachable:
        if 0 <= s[4] <= horizon:
            states_by_time[s[4]].append(s)
            
    V = {t: {} for t in range(horizon + 1)}
    for s in states_by_time[0]:
        V[0][s] = 0.0
        
    for t in range(1, horizon + 1):
        for s in states_by_time[t]:
            acts = []
            for a in action_order:
                if a in actions_by_state.get(s, ()):
                    acts.append(a)
            
            if not acts:
                V[t][s] = 0.0
                continue
            
            best_found_q = -float("inf")
            has_q = False
            for a in acts:
                if (s, a) in transitions:
                    q = 0.0
                    for p, ns, r in transitions[(s, a)]:
                        q += p * (r + V[t-1].get(ns, 0.0))
                    if q > best_found_q:
                        best_found_q = q
                        has_q = True
            V[t][s] = best_found_q if has_q else 0.0
            
    policy = {t: {} for t in range(1, horizon + 1)}
    for t in range(1, horizon + 1):
        for s in states_by_time[t]:
            acts = []
            for a in action_order:
                if a in actions_by_state.get(s, ()):
                    acts.append(a)
                    
            best_a = None
            max_q = -float("inf")
            
            for a in acts:
                if (s, a) in transitions:
                    curr_q = 0.0
                    for p, ns, r in transitions[(s, a)]:
                        curr_q += p * (r + V[t-1].get(ns, 0.0))
                    # i added this because its float -more accurate hadi dont delete itt -----------------------------------
                    if curr_q > max_q + 1e-12:
                        max_q = curr_q
                        best_a = a
            if best_a is not None:
                policy[t][s] = best_a
                
    return policy


def first_visit_mc(env, policy, n_episodes, gamma, seed):
    """Estimate V^pi from sampled episodes using first-visit Monte Carlo."""
    rng = random.Random(seed)
    
    is_time_dep = False
    if policy and type(list(policy.keys())[0]) is int:
        is_time_dep = True
        
    h = max(policy.keys()) if is_time_dep else env.default_horizon
    all_returns = {}
    
    for _ in range(n_episodes):

        sim = env.simulate(policy, horizon=h, seed=rng.randrange(2**32))
        path = sim["trajectory"]
        
        g = 0.0
        returns = []
        for step in reversed(path):
            g = step[2] + gamma * g
            returns.append(g)
        returns.reverse()
        
        visited = []
        for idx in range(len(path)):
            state = path[idx][0]
            if state not in visited:
                visited.append(state)
                if state not in all_returns:
                    all_returns[state] = []
                all_returns[state].append(returns[idx])
                
    v_final = {}
    for s in all_returns:
        v_final[s] = sum(all_returns[s]) / len(all_returns[s])
    return v_final


def td_prediction(env, policy, n_episodes, alpha_schedule, gamma, seed):
    """Estimate V^pi from online interaction using TD(0)."""
    rng = random.Random(seed)
    
    try:
        first_val = policy[list(policy.keys())[0]]
        is_time_dep = isinstance(first_val, dict)
    except:
        is_time_dep = False
        
    h = max(policy.keys()) if is_time_dep else env.default_horizon
    V = {}
    t_updates = 0
    
    for _ in range(n_episodes):
        s = env.reset(seed=rng.randrange(2**32), horizon=h)
        
        while not env.is_terminal(s):
            if s not in V: 
                V[s] = 0.0
            
            if is_time_dep:
                a = policy.get(s[4], {}).get(s, None)
            else:
                a = policy.get(s, None)
                
            if a is None:
                acts = env.actions(s)
                if not acts:
                    break
                a = acts[0]
                
            next_s, r, done, _ = env.step(a)
            if next_s not in V: 
                V[next_s] = 0.0
            
            alpha = alpha_schedule(t_updates)
            target = r if done else r + gamma * V[next_s]
            V[s] += alpha * (target - V[s])
            
            t_updates += 1
            s = next_s
            
    return V


def q_learning_rescue(env, n_episodes, alpha_schedule, epsilon_schedule, gamma, seed):
    """Learn a tabular Q policy with epsilon-greedy exploration."""
    rng = random.Random(seed)
    Q = {}
    seen = []
    updates = 0
    
    for _ in range(n_episodes):
        s = env.reset(seed=rng.randrange(2**32))
        
        while not env.is_terminal(s):
            acts = env.actions(s)
            if not acts: 
                break
            if s not in seen: 
                seen.append(s)
            
            eps = epsilon_schedule(updates)

            if rng.random() < eps:
                a = rng.choice(acts)
            else:
                best_a = acts[0]
                for action in acts:
                    if Q.get((s, action), 0.0) > Q.get((s, best_a), 0.0):
                        best_a = action
                a = best_a
                
            next_s, r, done, _ = env.step(a)
            if (s, a) not in Q: 
                Q[(s, a)] = 0.0
            
            if done:
                target = r
            else:
                n_acts = env.actions(next_s)
                max_next = 0.0
                if n_acts:
                    max_next = Q.get((next_s, n_acts[0]), 0.0)
                    for na in n_acts:
                        if Q.get((next_s, na), 0.0) > max_next:
                            max_next = Q.get((next_s, na), 0.0)
                target = r + gamma * max_next
                
            Q[(s, a)] += alpha_schedule(updates) * (target - Q[(s, a)])
            updates += 1
            s = next_s
            
    p_final = {}
    for s in seen:
        acts = env.actions(s)
        if acts:
            best_a = acts[0]
            for action in acts:
                if Q.get((s, action), 0.0) > Q.get((s, best_a), 0.0):
                    best_a = action
            p_final[s] = best_a
            
    return Q, p_final


def boltzmann_action(Q, state, actions, temperature):
    """Sample an action according to softmax(Q(state, action) / temperature)."""
    if not actions:
        return None
        
    if temperature <= 0.0:
        greedy_a = actions[0]
        for a in actions:
            if Q.get((state, a), 0.0) > Q.get((state, greedy_a), 0.0):
                greedy_a = a
        return greedy_a
        
    vals = []
    for a in actions:
        vals.append(Q.get((state, a), 0.0))
    max_val = max(vals)
    
    exps = []
    for v in vals:
        exps.append(math.exp((v - max_val) / temperature))
        
    sum_exps = sum(exps)
    probs = [e / sum_exps for e in exps]
    
    r_val = random.random()
    accumulator = 0.0
    for i in range(len(actions)):
        accumulator += probs[i]

        if r_val < accumulator:
            return actions[i]
    return actions[-1]


def evaluate_rescue_agent(env, policy, n_episodes, seed):
    """Return success rate, mean return, mean length, and failure breakdown."""
    rng = random.Random(seed)
    
    if n_episodes == 0:
        return {"mean_return": 0.0, "success_rate": 0.0, "mean_length": 0.0, "failure_breakdown": {}}
        
    is_time = False
    if policy and type(list(policy.keys())[0]) is int:
        is_time = True
        
    h = max(policy.keys()) if is_time else env.default_horizon
    
    tot_r = 0.0
    tot_l = 0
    success = 0
    fails = {}
    
    for _ in range(n_episodes):
        res = env.simulate(policy, horizon=h, seed=rng.randrange(2**32))
        tot_r += res["return"]
        tot_l += res["length"]
        
        reason = res["terminal_reason"]
        if reason == "success":
            success += 1
        else:
            fails[reason] = fails.get(reason, 0) + 1
            
    return {
        "mean_return": tot_r / n_episodes,
        "success_rate": success / n_episodes,
        "mean_length": tot_l / n_episodes,
        "failure_breakdown": fails,
    }


def default_alpha_schedule(t):
    """Alpha schedule required by the assignment."""
    return 0.5 / (1 + (t // 1000))


def constant_epsilon(t):
    return 0.1


def decaying_epsilon(t):
    return max(0.02, 1 / ((t + 1) ** 0.5))