#!/usr/bin/python3.6
import numpy as np

from gym_torcs import TorcsEnv

env = TorcsEnv(vision=False, throttle=False)
# ob = env.reset(relaunch=False)

# print( "Action space :", env.action_space)
# print( "Ob shape :", ob)
# input()

# Generate an agent
from sample_agent import Agent
agent = Agent(1)  # steering only
action = agent.act(ob, reward, done, vision=True)

# single step
ob, reward, done, _ = env.step(action)

# shut down torcs
env.end()
