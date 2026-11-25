
# Artificial Intelligence – Planning & Reinforcement Learning

Academic project developed as part of the Introduction to Artificial Intelligence course at the Technion – Israel Institute of Technology.

The project focuses on implementing planning and reinforcement learning algorithms for an autonomous rescue agent operating in a stochastic environment under time and battery constraints.

## Project Implementations

### 1. Planning and Dynamic Programming

[View Source Code](reinforcement-learning/planning_rescue.py)

- Finite-horizon Dynamic Programming.
- Markov Decision Processes (MDPs).
- Bellman optimality updates.
- State-value function computation.
- Time-dependent policy generation.
- Expected reward computation.
- Policy simulation and performance evaluation.

### 2. Reinforcement Learning

[View Source Code](reinforcement-learning/learning_rescue.py)

- Transition probability and reward model estimation from offline trajectories.
- Model-based planning.
- First-Visit Monte Carlo prediction.
- Temporal Difference Learning (TD(0)).
- Tabular Q-Learning.
- Epsilon-Greedy exploration.
- Boltzmann (Softmax) action selection.
- Learning rate and exploration schedules.

## Agent Evaluation

Implementation of evaluation methods based on:

- Mission success rate.
- Average cumulative reward.
- Average episode length.
- Failure analysis.
- Policy comparison under different constraints.

## Technologies and Skills

- Python
- Artificial Intelligence
- Reinforcement Learning
- Dynamic Programming
- Markov Decision Processes
- Probabilistic Modeling
- Algorithm Design
- Stochastic Decision-Making
- Performance Evaluation

## Academic Context

Developed as part of the B.Sc. in Computer Science at the Technion, using the course-provided simulation environment.
