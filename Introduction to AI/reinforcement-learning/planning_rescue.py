"""Student implementation file for HW3 Part B.

Only edit this file unless course staff explicitly instruct otherwise.
"""

from __future__ import annotations
import random

def action_value(env, V_next, state, action):
    """Return the one-step expected backup for a specific action."""
    q_val = 0.0
    
    transitions_list = env.transitions(state, action)
    for transition in transitions_list:
        prob = transition[0]
        next_s = transition[1]
        reward = transition[2]
        
        if next_s in V_next:
            val = V_next[next_s]
        else:
            val = 0.0
            
        q_val += prob * (reward + val)
        
    return q_val


def finite_horizon_dp(env, horizon):
    """Return V[t][s] and a time-dependent policy pi[t][s]."""
    
    reachable = env.reachable_states(horizon)
    states_by_time = {}
    
    for t in range(horizon + 1):
        states_by_time[t] = []
        
    for s in reachable:
        rem_time = s[4]
        if rem_time >= 0 and rem_time <= horizon:
            states_by_time[rem_time].append(s)
            
    V = {}
    V[0] = {}
    
    for s in states_by_time[0]:
        V[0][s] = 0.0
        
    for t in range(1, horizon + 1):
        V[t] = {}
        for s in states_by_time[t]:
            if env.is_terminal(s):
                V[t][s] = 0.0
                continue
                
            avail_actions = env.actions(s)
            if len(avail_actions) == 0:
                V[t][s] = 0.0
                continue
                
            best_q =-float("inf") 
            for act in avail_actions:
                current_q = action_value(env, V[t-1], s, act)
                if current_q > best_q:
                    best_q = current_q
            V[t][s] = best_q

    policy = {}
    for t in range(1, horizon + 1):
        policy[t] = {}
        
    for t in range(1, horizon + 1):
        for s in states_by_time[t]:
            if env.is_terminal(s):
                continue
                
            avail_actions = env.actions(s)
            if len(avail_actions) == 0:
                continue
                
           
            chosen_action = avail_actions[0]
            best_q = action_value(env, V[t-1], s, chosen_action)
            
          
            for act in avail_actions[1:]:
                current_q = action_value(env, V[t-1], s, act)
                if current_q > (best_q + 1e-12):
                    best_q = current_q
                    chosen_action = act
                    
            policy[t][s] = chosen_action
            
    return V, policy


def rollout_time_dependent_policy(env, policy, n_episodes, seed):
    """Simulate a time-dependent policy and return mission statistics."""
    rng = random.Random(seed)
    
    ret_list = []
    len_list = []
    fail_reasons = {}
    success_num = 0
    
    if len(policy) == 0:
        max_h = env.default_horizon
    else:
        max_h = max(policy.keys())
        
    for i in range(n_episodes):
        ep_seed = rng.randrange(2**32)
        res = env.simulate(policy, horizon=max_h, seed=ep_seed)
        
        ret_list.append(res["return"])
        len_list.append(res["length"])
        
        reason = res["terminal_reason"]
        if reason == "success":
            success_num += 1
        else:
            if reason not in fail_reasons:
                fail_reasons[reason] = 0
            fail_reasons[reason] += 1
            
    if n_episodes == 0:
        m_ret = 0.0
        m_len = 0.0
        s_rate = 0.0
    else:
        m_ret = sum(ret_list) / n_episodes
        m_len = sum(len_list) / n_episodes
        s_rate = success_num / n_episodes
        
    stats = {
        "mean_return": m_ret,
        "success_rate": s_rate,
        "mean_length": m_len,
        "failure_breakdown": fail_reasons,
        "returns": ret_list,
    }
    return stats


def compare_policy_slices(env, policy, states_to_show):
    """Return a table showing how actions change with time or battery."""
    ans = []
    seen = []
    
    for s in states_to_show:
        r, c, carry, bat, t_val = s
        physical_s = (r, c, carry, bat)
        
        if physical_s in seen:
            continue
        seen.append(physical_s)
        
        time_dict = {}
        for time_step in sorted(policy.keys()):
            full_s = (r, c, carry, bat, time_step)
            if full_s in policy[time_step]:
                time_dict[time_step] = policy[time_step][full_s]
                
        ans.append({
            "state": s,
            "actions_by_time": time_dict
        })
        
    return ans
